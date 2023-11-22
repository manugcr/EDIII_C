/* Configurar 4 canales del ADC para que funcionando en modo burst se obtenga una frecuencia
 * de muestreo en cada uno de 50K muestras/seg. Suponer un cclk = 100 Mhz
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"

#define INPUT	    (uint8_t) 0
#define	OUTPUT	    (uint8_t) 1
#define	RISING	    (uint8_t) 0
#define FALLING	    (uint8_t) 1
#define PIN(x)		(1 << x)
#define PORT(x)		x

void confGPIO();
void confADC();

uint32_t channel_0;
uint32_t channel_1;
uint32_t channel_2;
uint32_t channel_3;

int main(void)
{
	confGPIO();
	confADC();

	while(1){}

	return 0;
}


void confGPIO()
{
	// P0.23, P0.24, P0.25, P0.26 como entradas AD0
	PINSEL_CFG_Type	pinCFG;
	pinCFG.Portnum	=	0;
	pinCFG.Pinnum	=	23;
	pinCFG.Funcnum	=	1;							// Modo AD0
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;	// Ni pull-up ni pull-down
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Portnum	=	0;
	pinCFG.Pinnum	=	24;
	pinCFG.Funcnum	=	1;							// Modo AD0
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;	// Ni pull-up ni pull-down
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Portnum	=	0;
	pinCFG.Pinnum	=	25;
	pinCFG.Funcnum	=	1;							// Modo AD0
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;	// Ni pull-up ni pull-down
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Portnum	=	0;
	pinCFG.Pinnum	=	26;
	pinCFG.Funcnum	=	1;							// Modo AD0
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;	// Ni pull-up ni pull-down
	PINSEL_ConfigPin(&pinCFG);

	return;
}


void confADC()
{
	// Habilitamos el ADC para 4 canales, con interrupciones.
	ADC_Init(LPC_ADC, 200000);						// 200KHz rate --> 50KHz para cada canal
	ADC_BurstCmd(LPC_ADC, ENABLE);					// Modo burst activado.
	ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);

	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);

	ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);

	ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);

	ADC_ChannelCmd(LPC_ADC, 3, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN3, ENABLE);

	LPC_ADC->ADINTEN	&=~	(1<<8);					// Para modo burst el bit 8 de AD0INTEN tiene que ser 0.

	NVIC_EnableIRQ(ADC_IRQn);

	return;
}


void ADC_IRQHandler()
{

	if (ADC_ChannelGetStatus(LPC_ADC, 0, 1))
		channel_0 =  ADC_ChannelGetData(LPC_ADC, 0);
	else if (ADC_ChannelGetStatus(LPC_ADC, 1, 1))
		channel_1 =  ADC_ChannelGetData(LPC_ADC, 1);
	else if (ADC_ChannelGetStatus(LPC_ADC, 2, 1))
		channel_2 =  ADC_ChannelGetData(LPC_ADC, 2);
	else if (ADC_ChannelGetStatus(LPC_ADC, 3, 1))
		channel_3 =  ADC_ChannelGetData(LPC_ADC, 3);

	LPC_ADC->ADGDR 		&= 	LPC_ADC->ADGDR;		// Limpio la flag
	return;
}




