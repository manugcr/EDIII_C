/* Utilizando el Timer 1, escribir un código en C para que por cada presión de un pulsador,
 * la frecuencia de parpadeo de un led disminuya a la mitad debido a la modificación del
 * reloj que llega al periférico. El pulsador debe producir una interrupción por GPIO0
 * con flanco descendente. Adjuntar el código en C.
 *
 * FUNCIONO EN LA PLACA
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"

void confGPIO(void);
void confGPIOInt(void);
void confTimerMatch(void);

#define INPUT	    (uint8_t) 0
#define	OUTPUT	    (uint8_t) 1
#define	RISING	    (uint8_t) 0
#define FALLING	    (uint8_t) 1

#define PIN(x)		(1 << x)
#define PORT(x)		x

int main(void)
{
	confGPIO();
	confGPIOInt();
	confTimerMatch();

	while(1){}
	return 0;
}

void confGPIO()
{
	// Con drivers P0.4 como entrada GPIO
	PINSEL_CFG_Type pinCFG;
	pinCFG.Portnum	=	PINSEL_PORT_0;
	pinCFG.Pinnum	=	PINSEL_PIN_4;
	pinCFG.Pinmode	=	PINSEL_PINMODE_PULLUP;	// Tristate (?
	pinCFG.Funcnum	= 	PINSEL_FUNC_0; 			// GPIO

	PINSEL_ConfigPin(&pinCFG);
	GPIO_SetDir(PINSEL_PORT_0, PIN(4), INPUT);
	GPIO_SetDir(PINSEL_PORT_0, PIN(22), OUTPUT); // Led P0.22 output

	return;
}

void confGPIOInt(void)
{
	// Con drivers interrupciones por flanco de bajada GPIO P0.4
	GPIO_IntCmd(PORT(0), PIN(4), FALLING);
	GPIO_ClearInt(PORT(0), PIN(4));
	NVIC_EnableIRQ(EINT3_IRQn);

	return;
}

void confTimerMatch(void)
{
	// Con drivers TMR1.0 modo match cada 1seg base, sin interrupciones.
	// PR * MR = 1000*1000 = 1000000us = 1000ms = 1s
	TIM_TIMERCFG_Type	timerCFG;
	TIM_MATCHCFG_Type	matchCFG;

	timerCFG.PrescaleOption		=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		=	1000;

	matchCFG.MatchChannel		=	0;
	matchCFG.IntOnMatch			=	ENABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	matchCFG.MatchValue			=	1000;

	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM1, &matchCFG);
	TIM_Cmd(LPC_TIM1, ENABLE);

	NVIC_EnableIRQ(TIMER1_IRQn);

	return;
}

void TIMER1_IRQHandler(void)
{
	if (GPIO_ReadValue(PORT(0)) & PIN(22))
        GPIO_ClearValue(PORT(0), PIN(22));
    else
        GPIO_SetValue(PORT(0), PIN(22));

	TIM_ClearIntPending(LPC_TIM1, TIM_MR1_INT);
}

void EINT3_IRQHandler(void)				// Cambiamos frecuencia del led
{
	static uint8_t freq[4] = {1, 2, 0, 4};
	static uint8_t count = 0;


	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER1, freq[count]);

	if (count == 3)
		count = 0;
	count++;

	TIM_ResetCounter(LPC_TIM1);
	GPIO_ClearInt(PORT(0), PIN(4));
	return;
}
