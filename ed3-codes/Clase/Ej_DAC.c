/*
 * Utilizando CMSIS escriba y comente un código que genere una onda del tipo trapezoidal
 * a la salida del DAC como se muestra en la figura. Para ello el DAC comenzará, a partir
 * de cero, a incrementar su valor de a un bits hasta llegar a un valor máximo que se
 * mantendrá un tiempo dado y luego decrementará de a un bits hasta volver a cero
 * nuevamente. Los controles del valor máximo y los tiempos de duración de los
 * escalones de subida y bajada están organizados en la posición de memoria 0x10004000 de
 * la siguiente forma:
 * bits 0 a 7: valor máximo a alcanzar por el DAC.
 * bits 8 a 15: valor a utilizar en una función de demora que define el tiempo que se mantiene el valor máximo.
 * bits 16 a 23: valor a utilizar en una función de demora para definir el tiempo que se mantiene cada incremento de 1 bits en la subida.
 * bits 24 a 31: valor a utilizar en una función de demora para definir el tiempo que se mantiene cada decremento de 1 bits en bajada.
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

void confGPIO();
void confDAC();
void confTIMER();

#define ADDRESS 0x10004000
uint8_t MAX_VAL;
uint8_t MAX_TIME;
uint8_t MAX_INC;
uint8_t MAX_DEC;
uint32_t signal[];
uint32_t valores;

int main()
{
    volatile uint32_t *address = (uint32_t *)ADDRESS;
    valores 	= *address;
    MAX_VAL		= valores & 0xFF;
    MAX_TIME	= (valores >> 8) & 0xFF;
    MAX_INC		= (valores >> 16) & 0xFF;
    MAX_DEC		= (valores >> 24) & 0xFF;

	confGPIO();
	confDAC();
	confTIMER();

	// Relleno el array de la manera {0, 1, 2, ..., MAX_VAL, ..., 2, 1, 0}
	for (int i=0 ; i<MAX_VAL ; ++i)
		signal[i] = i;
    for (int i=MAX_VAL ; i<MAX_VAL*2 ; ++i)
        signal[i] = MAX_VAL - (i - MAX_VAL);

	while(1);

	return 0;
}

void confGPIO()
{
	// Pin de salida de senal P0.26 AOUT
	PINSEL_CFG_Type pinCFG;
	pinCFG.Portnum	= 0;
	pinCFG.Pinnum	= 26;
	pinCFG.Funcnum	= 2;
	pinCFG.Pinmode	= 0;
	PINSEL_ConfigPin(&pinCFG);
}

void confDAC()
{
	DAC_CONVERTER_CFG_Type dacCFG;
	dacCFG.CNT_ENA = DISABLE;
	dacCFG.DMA_ENA = DISABLE;
	DAC_Init(LPC_DAC);
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCFG);
}

void confTIMER()
{
	TIM_TIMERCFG_Type	timerCFG;
	TIM_MATCHCFG_Type	matchCFG;

	timerCFG.PrescaleOption		=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		=	1;						// TC++ Cada 1 [us]

	matchCFG.MatchChannel		=	0;
	matchCFG.IntOnMatch			= 	ENABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	matchCFG.MatchValue			=	MAX_INC;			// El valor de match que doy esta en [us].
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM0, &matchCFG);
	TIM_Cmd(LPC_TIM0, ENABLE);

	NVIC_EnableIRQ(TIMER0_IRQn);

}

void TIMER0_IRQHandler()
{
	static uint32_t value = 0;

	if (value < MAX_VAL)
		TIM_UpdateMatchValue(LPC_TIM0, 0, MAX_INC);
	else if (value == MAX_VAL)
		TIM_UpdateMatchValue(LPC_TIM0, 0, MAX_TIME);
	else if (value > MAX_VAL)
		TIM_UpdateMatchValue(LPC_TIM0, 0, MAX_DEC);

	DAC_UpdateValue(LPC_DAC, signal[value]);
	TIM_ResetCounter(LPC_TIM0);

	LPC_TIM0->IR	|=	1;
	return;
}
