/* Utilizando el timer0, un dac, interrupciones y el driver del LPC1769 , escribir un código que
 * permita generar una señal triangular periódica simétrica, que tenga el mínimo periodo posible,
 * la máxima excursión de voltaje pico a pico posible y el mínimo incremento de señal posible por el dac.
 * Suponer una frecuencia de core cclk de 100 Mhz. El código debe estar debidamente comentado.
 *
 * Busco una onda triangular de 0 a 1023 y de 1023 a 0.
 * Por default N = 4
 * t minimo por escalon 1 [us].
 * T = 1023+1022 = 2045
 * [tick] = 1/(fclk/N) = 1/(100e6/4) = 4e-8
 * timeout = t/1/(fclk/N) = 1[us]/4e-8 = 25 [ticks]
 *
 *
 * <------T------>
 * <t>
 *        _
 *      _| |_
 *	  _|     |_
 * 	_|         |_
 */


#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"

#define INPUT	    (uint8_t) 0
#define	OUTPUT	    (uint8_t) 1
#define	RISING	    (uint8_t) 0
#define FALLING	    (uint8_t) 1
#define PIN(x)		(1 << x)
#define PORT(x)		x
#define SIZE 		2045

void confGPIO();
void confDAC();
void confDMA();

uint32_t values[SIZE];			// {0, 1, 2, ..., 1023, ..., 2, 1, 0} = 1023+1022 = 2045

int main()
{
	// Rellenar el array con valores 0:1023:0
	for (int i = 0; i < 1023; i++)
		values[i] = i;

	for (int j = 1023; j <= 2045; j++)
		values[j] = 1023-(j-1023);

	for (int i = 0; i < SIZE; i++)
		values[i] = values[i]<<6;

	confGPIO();
	confDAC();
	confDMA();
	GPDMA_ChannelCmd(0, ENABLE);

	while(1){}

	return 0;
}

void confGPIO()
{
	// P0.26 como AOUT
	PINSEL_CFG_Type pinCFG;
	pinCFG.Funcnum = 2;
	pinCFG.OpenDrain = 0;
	pinCFG.Pinmode = 0;
	pinCFG.Portnum = 0;
	pinCFG.Pinnum = 26;
	PINSEL_ConfigPin(&pinCFG);

	return;
}

void confDAC()
{
	DAC_CONVERTER_CFG_Type dacCFG;
	dacCFG.CNT_ENA = SET;
	dacCFG.DMA_ENA = SET;
	DAC_Init(LPC_DAC);
	DAC_SetDMATimeOut(LPC_DAC, 25);						// 25 ticks
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCFG);
	return;
}

void confDMA()
{
	// DMA M2P
	GPDMA_LLI_Type LLI;
	LLI.SrcAddr		=	(uint32_t) values;
	LLI.DstAddr		=	(uint32_t) &LPC_DAC->DACR;
	LLI.NextLLI		=	(uint32_t) &LLI;				// Lista ciclica.
	LLI.Control		=	SIZE
					|	(2<<18)							// 32bits source
					|	(2<<21)							// 32bits destino
					|	(1<<26)							// Source increment
					;

	GPDMA_Channel_CFG_Type gpdmaCFG;
	GPDMA_Init();
	gpdmaCFG.ChannelNum		=	0;						// Canal de maxima prioridad
	gpdmaCFG.TransferSize	=	SIZE;
	gpdmaCFG.TransferWidth	=	0;						// Solo para M2M
	gpdmaCFG.SrcMemAddr		=	(uint32_t) values;
	gpdmaCFG.DstMemAddr		=	0;						// Solo para M2M o P2M
	gpdmaCFG.TransferType	=	GPDMA_TRANSFERTYPE_M2P;
	gpdmaCFG.SrcConn		=	0;						// Solo para P2M o P2P
	gpdmaCFG.DstConn		=	GPDMA_CONN_DAC;
	gpdmaCFG.DMALLI			=	(uint32_t) &LLI;
	GPDMA_Setup(&gpdmaCFG);

	return;
}
