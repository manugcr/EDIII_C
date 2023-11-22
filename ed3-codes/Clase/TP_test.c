#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

#define LISTSIZE 1000

void configTimer(void);
void configADC(void);
void configGPIO(void);
void configDMA(void);

uint16_t ADC0Value = 0;
uint32_t listADC[LISTSIZE] = {0};

int main()
{
	configTimer();
	configGPIO();
	configADC();

	/* zero-fill adc values list. */
	for(int i=0; i<LISTSIZE; i++)
		*(listADC+i) = (uint32_t) 0;

	while(1){}
}


void configTimer(void)
{
	/* TIM0.1 en modo toggle para conversion del ADC. */
	TIM_TIMERCFG_Type	timerCFG;
	TIM_MATCHCFG_Type	matchCFG;

	timerCFG.PrescaleOption	=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		=	1;

	matchCFG.MatchChannel		=	1;
	matchCFG.IntOnMatch			=	DISABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
	matchCFG.MatchValue			=	500000;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM0, &matchCFG);
	TIM_Cmd(LPC_TIM0, ENABLE);
}


void configADC(void)
{
	/* ADC en 200KHz con conversion por flanco desc de TMR0.1 */
	ADC_Init(LPC_ADC, 200000);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
}


void configGPIO(void)
{
	/* Set P0.23 como entrada AD0.0 */
	PINSEL_CFG_Type pinCFG;
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_23;
	pinCFG.Portnum		= PINSEL_PORT_0;

	PINSEL_ConfigPin(&pinCFG);
}


void ADC_IRQHandler(void)
{
	/* Tomamos una muestra del ADC y la guardamos en el array sin superar el limite de muestras. */
	static uint32_t count = 0;

	if (count < LISTSIZE)
		listADC[count] = ((LPC_ADC->ADDR0)>>4) & 0xFFF;
	else
		count = 0;

	count++;

	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
}
