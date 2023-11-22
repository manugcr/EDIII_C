/*
 * Programar el microcontrolador LPC1769 para que mediante su ADC digitalice dos senales
 * analogicas cuyos anchos de banda son de 10KHz cada una. Los canales utilizados deben ser
 * el 2 y 4 y los datos deben ser guardados en dos regiones de memoria distintas que permitan
 * contar con los ultimos 20 datos de cada canal. Suponer una frecuencia de core cclk 100MHz
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"

#define TABLE_LEN 20

void confDMA(void);
void confGPIO(void);
void confADC(void);
void confDAC(void);

volatile uint32_t* values_2 = (uint32_t*) 0x2007C000;;
volatile uint32_t values_4[TABLE_LEN];

int main(void)
{
	confGPIO();
	confADC();
	confDAC();
	confDMA();

	while (1)
	{

	}

	return 0;
}


void confGPIO()
{
	// Seteamos P0.25 como AD0.2
	PINSEL_CFG_Type pinCFG;
	pinCFG.Portnum	=	0;
	pinCFG.Pinnum	=	25;
	pinCFG.Funcnum	=	1;							// Modo AD0.2
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;	// Neither pull-up/down
	PINSEL_ConfigPin(&pinCFG);

	// Seteamos P1.30 como AD0.4
	pinCFG.Portnum	=	1;
	pinCFG.Pinnum	=	30;
	pinCFG.Funcnum	=	4;							// Modo AD0.4
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;	// Neither pull-up/down
	PINSEL_ConfigPin(&pinCFG);

	// Seteamos P0.26 como AOUT
	pinCFG.Funcnum = 2;
	pinCFG.Pinmode = 0;
	pinCFG.Pinnum = 26;
	pinCFG.Portnum = 0;
	PINSEL_ConfigPin(&pinCFG);

	return;
}

void confDAC(void)
{
	uint32_t tmp;
	DAC_CONVERTER_CFG_Type DAC_ConverterConfigStruct;
	DAC_ConverterConfigStruct.CNT_ENA = SET;
	DAC_ConverterConfigStruct.DMA_ENA = SET;
	DAC_Init(LPC_DAC);
	/* set time out for DAC*/
	tmp = (100);
	DAC_SetDMATimeOut(LPC_DAC,tmp);
	DAC_ConfigDAConverterControl(LPC_DAC, &DAC_ConverterConfigStruct);

	return;
}


void confADC(void)
{
	// ADC en modo burst. 2 Canales de 20KHz cada uno
	ADC_Init(LPC_ADC, 40000);
	ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
	ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
	ADC_BurstCmd(LPC_ADC, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);

//	ADC_ChannelCmd(LPC_ADC, 4, ENABLE);
//	ADC_IntConfig(LPC_ADC, 4, ENABLE);
	LPC_ADC->ADINTEN	&=~	(1<<8);					// Para modo burst el bit 8 de AD0INTEN tiene que ser 0.

//	NVIC_DisableIRQ(ADC_IRQn);						// Si se usa DMA hay que deshabilitar las interrupciones.
	return;
}


void confDMA(void)
{
	GPDMA_LLI_Type DMA_LLI_Struct1;
	GPDMA_LLI_Type DMA_LLI_Struct2;

	//Prepare DMA link list item structure
	DMA_LLI_Struct1.SrcAddr	= (uint32_t) &(LPC_ADC->ADDR2);
	DMA_LLI_Struct1.DstAddr	= (uint32_t) values_2;
	DMA_LLI_Struct1.NextLLI	= (uint32_t) &DMA_LLI_Struct1;
	DMA_LLI_Struct1.Control	= 20
							| (2<<18) // source width 32 bit
							| (2<<21) // dest. width 32 bit
							| (1<<27) // dest increment
							;

	DMA_LLI_Struct2.SrcAddr= (uint32_t)values_2;
	DMA_LLI_Struct2.DstAddr= (uint32_t)&(LPC_DAC->DACR);
	DMA_LLI_Struct2.NextLLI= (uint32_t)&DMA_LLI_Struct1;
	DMA_LLI_Struct2.Control= 20
			| (2<<18) //source width 32 bit
			| (2<<21) //dest. width 32 bit
			| (1<<26) //source increment
			;

//	DMA_LLI_Struct2.SrcAddr	= (uint32_t) &(LPC_ADC->ADDR4);
//	DMA_LLI_Struct2.DstAddr	= (uint32_t) values_4;
//	DMA_LLI_Struct2.NextLLI	= (uint32_t) &DMA_LLI_Struct2;
//	DMA_LLI_Struct2.Control	= TABLE_LEN
//							| (2<<18) // source width 32 bit
//							| (2<<21) // dest. width 32 bit
//							| (1<<27) // source increment
//							;

	/* GPDMA block section -------------------------------------------- */
	GPDMA_Init();
	// Setup GPDMA channel --------------------------------
	GPDMA_Channel_CFG_Type GPDMACfg1;
	GPDMACfg1.ChannelNum		= 2;
	GPDMACfg1.DMALLI			= (uint32_t) &DMA_LLI_Struct1;
	GPDMACfg1.DstMemAddr		= (uint32_t) DMA_LLI_Struct1.DstAddr;
	GPDMACfg1.SrcMemAddr		= (uint32_t) 0;
	GPDMACfg1.DstConn			= (uint32_t) 0;
	GPDMACfg1.SrcConn			= GPDMA_CONN_ADC;
	GPDMACfg1.TransferSize		= (uint32_t) 20;
	GPDMACfg1.TransferWidth		= 0;
	GPDMACfg1.TransferType		= GPDMA_TRANSFERTYPE_P2M;

//	GPDMA_Chawnnel_CFG_Type GPDMACfg2;
//	GPDMACfg2.ChannelNum		= 4;
//	GPDMACfg2.DMALLI			= (uint32_t) &DMA_LLI_Struct2;
//	GPDMACfg2.DstMemAddr		= (uint32_t) DMA_LLI_Struct2.DstAddr;
//	GPDMACfg2.SrcMemAddr		= (uint32_t) 0;
//	GPDMACfg2.DstConn			= (uint32_t) 0;
//	GPDMACfg2.SrcConn			= GPDMA_CONN_ADC;
//	GPDMACfg2.TransferSize		= TABLE_LEN;
//	GPDMACfg2.TransferWidth		= 0;
//	GPDMACfg2.TransferType		= GPDMA_TRANSFERTYPE_P2M;

	GPDMA_Channel_CFG_Type GPDMACfg2;
	GPDMACfg2.ChannelNum = 0;
	GPDMACfg2.SrcMemAddr = (uint32_t)(values_2);
	GPDMACfg2.DstMemAddr = 0;
	GPDMACfg2.TransferSize = 20;
	GPDMACfg2.TransferWidth = 0;
	GPDMACfg2.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg2.SrcConn = 0;
	GPDMACfg2.DstConn = GPDMA_CONN_DAC;
	GPDMACfg2.DMALLI = (uint32_t)&DMA_LLI_Struct2;
	GPDMA_Setup(&GPDMACfg2);


	GPDMA_Setup(&GPDMACfg1);
	GPDMA_Setup(&GPDMACfg2);
	GPDMA_ChannelCmd(2, ENABLE);
	GPDMA_ChannelCmd(0, ENABLE);
	return;
}
