/* Escribir un programa para que por cada presión de un pulsador, la frecuencia de parpadeo
 * disminuya a la mitad debido a la modificación del registro del Match 0.
 * El pulsador debe producir una interrupción por EINT2 con flanco descendente.
 *
 * Funciono pero medio bugueado.
 */
#include "LPC17xx.h"
#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_pinsel.h"

#define INPUT	    (uint8_t) 0
#define	OUTPUT	    (uint8_t) 1
#define	RISING	    (uint8_t) 0
#define FALLING	    (uint8_t) 1

#define PORT(x)		x
#define PIN(x)		(1 << x)

void confGPIO(void);
void confExtInt(void);
void confTimerMatch(void);

int main(void)
{
	confGPIO();
	confExtInt();
	confTimerMatch();

	while(1){}
	return 0;
}

void confGPIO(void)
{
	PINSEL_CFG_Type	pinCFG;

	// Seteamos P0.22 como output LED
	pinCFG.Portnum	=	PINSEL_PORT_0;
	pinCFG.Pinnum	=	PINSEL_PIN_22;
	pinCFG.Funcnum	=	PINSEL_FUNC_0;				// GPIO
	GPIO_SetDir(PINSEL_PORT_0, PIN(22), OUTPUT);
	PINSEL_ConfigPin(&pinCFG);

	// Seteamos P2.12 input
	pinCFG.Portnum	=	PINSEL_PORT_2;
	pinCFG.Pinnum	=	PINSEL_PIN_12;
	pinCFG.Pinmode	=	PINSEL_PINMODE_PULLUP;
	pinCFG.Funcnum	=	PINSEL_FUNC_1;				// EINT2
	GPIO_SetDir(PINSEL_PORT_2, PIN(11), INPUT);
	PINSEL_ConfigPin(&pinCFG);

	return;
}

void confExtInt(void)
{
	// Interrupciones por EINT2 P2.11
	EXTI_Init();
	EXTI_InitTypeDef extiCFG;
	extiCFG.EXTI_Line		=	EXTI_EINT2;
	extiCFG.EXTI_Mode		=	EXTI_MODE_LEVEL_SENSITIVE;
	extiCFG.EXTI_polarity	=	EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&extiCFG);
	NVIC_EnableIRQ(EINT2_IRQn);

	return;
}

void confTimerMatch(void)
{
	// Habilitamos TMR0 Modo match con interrupciones.
	TIM_TIMERCFG_Type timerCFG;
	TIM_MATCHCFG_Type matchCFG;

	timerCFG.PrescaleOption	=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue	=	1000;					// TC++ cada 1000us

	matchCFG.MatchChannel		=	0;
	matchCFG.IntOnMatch			=	ENABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	matchCFG.MatchValue			=	1000;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM0, &matchCFG);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);

	return;
}

void TIMER0_IRQHandler(void)
{
	if (GPIO_ReadValue(PORT(0)) & PIN(22))
        GPIO_ClearValue(PORT(0), PIN(22));
    else
        GPIO_SetValue(PORT(0), PIN(22));

	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}


void EINT2_IRQHandler(void)
{
	static uint32_t match_val[2] = {100, 1000};
	static uint8_t count = 0;

	LPC_TIM0->MR0 = match_val[count];

	count++;
	if (count == 1)
		count = 0;

	EXTI_ClearEXTIFlag(EXTI_EINT2);
	return;
}

