/* Escribir un programa para que por cada presión de un pulsador, la frecuencia de parpadeo de un led
 * disminuya a la mitad debido a la modificación del pre-escaler del Timer 2. El pulsador debe
 * producir una interrupción por EINT1 con flanco descendente.
 *
 *
 * FUNCIONO EN LA PLACA
 *
 */

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
	PINSEL_CFG_Type pinCFG;

	// P0.22 Como led output
	pinCFG.Portnum	=	PINSEL_PORT_0;
	pinCFG.Pinnum	=	PINSEL_PIN_22;
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;
	pinCFG.Funcnum	=	PINSEL_FUNC_0;				// GPIO
	GPIO_SetDir(PINSEL_PORT_0, PIN(22), OUTPUT);
	PINSEL_ConfigPin(&pinCFG);

	// P2.11 como entrada EINT1
	pinCFG.Portnum	=	PINSEL_PORT_2;
	pinCFG.Pinnum	=	PINSEL_PIN_11;
	pinCFG.Pinmode	=	PINSEL_PINMODE_TRISTATE;
	pinCFG.Funcnum	= 	PINSEL_FUNC_1; 			 	// EINT1
	GPIO_SetDir(PINSEL_PORT_2, PIN(4), INPUT);
	PINSEL_ConfigPin(&pinCFG);

	return;
}

void confExtInt(void)
{
	// Interrupciones por EINT1
	EXTI_Init();
	EXTI_InitTypeDef extiCFG;
	extiCFG.EXTI_Line		=	EXTI_EINT1;
	extiCFG.EXTI_Mode		=	EXTI_MODE_EDGE_SENSITIVE;
	extiCFG.EXTI_polarity	=	EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&extiCFG);
	NVIC_EnableIRQ(EINT1_IRQn);

	return;
}

void confTimerMatch(void)
{
	// Habilitamos TMR2 Modo match, interrupciones activadas, TC++ cada 1000us
	TIM_TIMERCFG_Type timerCFG;
	TIM_MATCHCFG_Type matchCFG;

	timerCFG.PrescaleOption = TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue = 1000;

	matchCFG.MatchChannel = 0;
	matchCFG.IntOnMatch = ENABLE;
	matchCFG.StopOnMatch = DISABLE;
	matchCFG.ResetOnMatch = ENABLE;
	matchCFG.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	matchCFG.MatchValue = 1000;

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM2, &matchCFG);
	TIM_Cmd(LPC_TIM2, ENABLE);
	NVIC_EnableIRQ(TIMER2_IRQn);

	return;
}

void TIMER2_IRQHandler(void)
{
	if (GPIO_ReadValue(PORT(0)) & PIN(22))
        GPIO_ClearValue(PORT(0), PIN(22));
    else
        GPIO_SetValue(PORT(0), PIN(22));

	TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
}

void EINT1_IRQHandler(void)
{
	static uint8_t prescale_val[4] = {1000, 2000, 4000, 5000};
	static uint8_t count = 0;

	TIM_TIMERCFG_Type timerCFG;
	timerCFG.PrescaleOption	=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue	=	prescale_val[count];

	count++;
	if (count == 3);
		count == 0;

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timerCFG);
	TIM_ResetCounter(LPC_TIM2);
	EXTI_ClearEXTIFlag(EXTI_EINT1);

	return;
}

