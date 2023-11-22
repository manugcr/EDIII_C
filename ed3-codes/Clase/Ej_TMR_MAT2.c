/* Escribir el código que configure el timer0 para cumplir con las especificaciones dadas en
 * la figura adjunta. (Pag 510 Figura 115 del manual de usuario del LPC 1769).
 * Considerar una frecuencia de cclk de 100 Mhz y una división de reloj de periférico de 2.
 *
 *
 *	Deberia andar (? Pq los 100MHz son por defecto, y lo unico que habria que adaptar es que
 *	el divisor de PCLK sea 2 y que el Match Register el TMR0 sea 6.
 */

#include "LPC17xx.h"

void confTimerMatch(void);

int main(void)
{
	confTimerMatch();

	while(1){}
	return 0;
}


void confTimerMatch(void)
{
	TIM_TIMERCFG_Type	timerCFG;
	TIM_MATCHCFG_Type	matchCFG;

	timerCFG.PrescaleOption		=	TIM_PRESCALE_TICKVAL;
	timerCFG.PrescaleValue		=	2;

	matchCFG.MatchChannel		=	0;
	matchCFG.IntOnMatch			=	DISABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
	matchCFG.MatchValue			=	6;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM0, &matchCFG);
	TIM_Cmd(LPC_TIM0, ENABLE);

	return;
}


