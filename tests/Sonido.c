#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "pacman.h"

#define TIMEOUT 6000

void configPin();
void configDAC();
void configDMA2(const uint16_t rawData[]);
void configADC();
void configTIMER();
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);

GPDMA_LLI_Type LLI_Array[24];
GPDMA_Channel_CFG_Type dmaCFG;

uint32_t ADCVAL 	= 0;
uint32_t ADCVALMAP	= 0;

int main()
{
 	configPin();
 	configTIMER();
	configDAC();
	configADC();
	configDMA2(rawData);

	while(1)
	{
	}

	return 0;
}


void configPin()
{
	/* Set P0.26 como AOUT */
	PINSEL_CFG_Type pinCFG;
	pinCFG.Funcnum		= PINSEL_FUNC_2;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_26;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P0.23 como AD0.0 */
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_23;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);

	return;
}


void configDAC()
{
	DAC_CONVERTER_CFG_Type dacCfg;
	dacCfg.DBLBUF_ENA	= 0;  // esta yo la pondría por las dudas
	dacCfg.CNT_ENA		= SET;
	dacCfg.DMA_ENA		= SET;

	DAC_ConfigDAConverterControl(LPC_DAC, &dacCfg);
	DAC_SetDMATimeOut(LPC_DAC, TIMEOUT);
	DAC_Init(LPC_DAC);

	return;
}


void configDMA2(const uint16_t rawData[])
{
	for (int i = 0; i < 24; i++)
	{
		LLI_Array[i].DstAddr 	= (uint32_t) & (LPC_DAC->DACR);
		LLI_Array[i].SrcAddr 	= (uint32_t)(rawData + i * 4095);

		if (i == (24 - 1)) {
		    LLI_Array[i].NextLLI = (uint32_t)&LLI_Array[0];
		} else {
		    LLI_Array[i].NextLLI = (uint32_t)&LLI_Array[i + 1];
		}

		LLI_Array[i].Control 	= 4095
								| (1<<18)	// source width 16 bit
								| (1<<22)	// dest width = word 32 bits
								| (1<<26);	// source increment
	}

	dmaCFG.ChannelNum			= 0;
	dmaCFG.TransferSize			= 4095;
	dmaCFG.TransferWidth		= 0;
	dmaCFG.TransferType			= GPDMA_TRANSFERTYPE_M2P;
	dmaCFG.SrcConn				= 0;
	dmaCFG.DstConn				= GPDMA_CONN_DAC;
	dmaCFG.SrcMemAddr			= (uint32_t) rawData;
	dmaCFG.DstMemAddr			= 0;
	dmaCFG.DMALLI				= (uint32_t) &LLI_Array[0];

	GPDMA_Init();
	GPDMA_Setup(&dmaCFG);
	GPDMA_ChannelCmd(0, ENABLE);
	return;
}


void configADC()
{
	ADC_Init(LPC_ADC, 200000);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);

	return;
}


void configTIMER(void){
	TIM_TIMERCFG_Type timerCFG;
	timerCFG.PrescaleOption		= TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		= 1;

	TIM_MATCHCFG_Type matchCFG;
	matchCFG.MatchChannel		= 1;
	matchCFG.MatchValue			= 1000000;
	matchCFG.ResetOnMatch		= ENABLE;
	matchCFG.StopOnMatch		= DISABLE;
	matchCFG.IntOnMatch			= DISABLE;
	matchCFG.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM0, &matchCFG);
	TIM_Cmd(LPC_TIM0, ENABLE);

	return;
}


void ADC_IRQHandler()
{
	ADCVAL 		= ((LPC_ADC->ADDR0)>>6) & 0x3FF;
	ADCVALMAP	= map(ADCVAL, 0, 1024, 4000, 8000);

//	GPDMA_ChannelCmd(0, DISABLE);
//	DAC_UpdateValue(LPC_DAC, 0);
	DAC_SetDMATimeOut(LPC_DAC, ADCVALMAP);
//	configDMA2(rawData);

	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;

	return;
}


uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
