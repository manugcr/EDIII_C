#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"


#define DMA_SIZE 60
#define NUM_SINE_SAMPLE 60
#define SINE_FREQ_IN_HZ 60
#define PCLK_DAC_IN_MHZ 25

void configPin();
void configDAC();
void configDMA();
void configADC();
void configTIMER();
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);


uint32_t sinSamples[NUM_SINE_SAMPLE] = {511, 564, 617, 669, 719, 767, 812, 853, 891, 925, 954, 978, 997, 1011, 1020, 1023,
						   1020, 1011, 997, 978, 954, 925, 891, 853, 812, 767, 719, 669, 617, 564, 511, 458,
						   405, 353, 303, 255, 210, 169, 131, 97, 68, 44, 25, 11, 2, 0, 2, 11, 25, 44, 68,
						   97, 131, 169, 210, 255, 303, 353, 405, 458};

__IO uint16_t ADC0Value = 0;

int main()
{

 	configPin();
	configTIMER();
	configADC();
	configDAC();

	for(uint8_t index = 0; index<NUM_SINE_SAMPLE; index++)
		sinSamples[index] = sinSamples[index]<<6;

	configDMA();

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
	dacCfg.CNT_ENA = SET;
	dacCfg.DMA_ENA = SET;
	DAC_Init(LPC_DAC);
	/*Set timeout*/
	uint32_t tmp;
	tmp = (PCLK_DAC_IN_MHZ * 1000000)/(SINE_FREQ_IN_HZ * NUM_SINE_SAMPLE);
	DAC_SetDMATimeOut(LPC_DAC, tmp);
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCfg);

}


void configDMA()
{
	GPDMA_LLI_Type LLI1;
	LLI1.SrcAddr = (uint32_t) sinSamples;
	LLI1.DstAddr = (uint32_t) &LPC_DAC->DACR;
	LLI1.NextLLI = (uint32_t) &LLI1;
	LLI1.Control = DMA_SIZE
				   | (2<<18) //source width 32 bits
				   | (2<<21) //dest width 32 bits
				   | (1<<26); //source increment

	GPDMA_Init();

	GPDMA_Channel_CFG_Type GPDMACfg;
	GPDMACfg.ChannelNum = 0;
	GPDMACfg.SrcMemAddr = (uint32_t)sinSamples;
	GPDMACfg.DstMemAddr = 0;
	GPDMACfg.TransferSize = DMA_SIZE;
	GPDMACfg.TransferWidth = 0;
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg.SrcConn = 0;
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	GPDMACfg.DMALLI = (uint32_t)&LLI1;
	GPDMA_Setup(&GPDMACfg);
	GPDMA_ChannelCmd(0, ENABLE);
}


void configADC()
{
	ADC_Init(LPC_ADC, 200000);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
}

void configTIMER(void){
	TIM_TIMERCFG_Type configTimer;
	configTimer.PrescaleOption = TIM_PRESCALE_USVAL;
	configTimer.PrescaleValue = 1; //TC se incrementa cada (CCLK/4)^-1 * 2500 = 100us
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &configTimer);

	TIM_MATCHCFG_Type configMatch;
	configMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	configMatch.IntOnMatch = DISABLE;
	configMatch.MatchChannel = 1;
	configMatch.MatchValue = 1000000;
	configMatch.ResetOnMatch = ENABLE;
	configMatch.StopOnMatch = DISABLE;
	TIM_ConfigMatch(LPC_TIM0, &configMatch);

	TIM_Cmd(LPC_TIM0, ENABLE);
}


void ADC_IRQHandler()
{
	static uint32_t tmp = 0;
	tmp = ((LPC_ADC->ADDR0)>>6) & 0x3FF;
	ADC0Value = map(tmp, 0, 1024, 10, 300);

	DAC_UpdateValue(LPC_DAC, (PCLK_DAC_IN_MHZ * 1000000)/(ADC0Value * NUM_SINE_SAMPLE));

	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
	return;
}


uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

