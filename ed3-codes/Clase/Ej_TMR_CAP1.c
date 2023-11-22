/* Utilizando el modo Capture, escribir un código en C  para que guarde en una variable ,llamada "shooter",
 * el tiempo (en milisegundos) que le lleva a una persona presionar dos pulsadores con un único dedo.
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#define INPUT	    (uint8_t) 0
#define	OUTPUT	    (uint8_t) 1
#define	RISING	    (uint8_t) 0
#define FALLING	    (uint8_t) 1
#define PIN(x)		(1 << x)
#define PORT(x)		x

void confGPIO(void);
void confTIMER0(void);
void calculate_time(void);

uint32_t	shooter 	=	0;
uint32_t	captures[2]	=	{0, 0};

int main(void)
{
	confGPIO();
	confTIMER0();

	while(1)
	{
		calculate_time();
	}

	return 0;
}

void confGPIO(void)
{
	// P1.26 CAP0.0 y P1.27 CAP0.1
	PINSEL_CFG_Type pinCFG;

	pinCFG.Portnum		=	PINSEL_PORT_1;
	pinCFG.Pinnum		=	PINSEL_PIN_26;
	pinCFG.Funcnum		=	PINSEL_FUNC_3;					// CAP0.0
	GPIO_SetDir(PINSEL_PORT_1, PIN(26), OUTPUT);
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Portnum		=	PINSEL_PORT_1;
	pinCFG.Pinnum		=	PINSEL_PIN_27;
	pinCFG.Funcnum		=	PINSEL_FUNC_3;					// CAP0.0
	GPIO_SetDir(PINSEL_PORT_1, PIN(27), OUTPUT);
	PINSEL_ConfigPin(&pinCFG);

}

void confTIMER0(void)
{
	// Seteo TIMER0 modo capture en dos canales CAP0.0 y CAP0.1

	TIM_TIMERCFG_Type	timerCFG;
	TIM_CAPTURECFG_Type	captureCFG;

	timerCFG.PrescaleOption		=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		=	1000;					// TC++ cada 1000[us]

	captureCFG.FallingEdge		=	ENABLE;
	captureCFG.RisingEdge		=	DISABLE;
	captureCFG.IntOnCaption		=	ENABLE;
	captureCFG.CaptureChannel	=	0;
	TIM_ConfigCapture(LPC_TIM0, &captureCFG);

	captureCFG.FallingEdge		=	ENABLE;
	captureCFG.RisingEdge		=	DISABLE;
	captureCFG.IntOnCaption		=	ENABLE;
	captureCFG.CaptureChannel	=	1;
	TIM_ConfigCapture(LPC_TIM0, &captureCFG);

	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);

	return;
}


void TIMER0_IRQHandler(void)
{
	if (TIM_GetIntStatus(LPC_TIM0, TIM_CR0_INT))
	{
		captures[0] = TIM_GetCaptureValue(LPC_TIM0, 0);
		TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
	}
	else if (TIM_GetIntStatus(LPC_TIM0, TIM_CR1_INT))
	{
		captures[1] = TIM_GetCaptureValue(LPC_TIM0, 1);
		TIM_ClearIntPending(LPC_TIM0, TIM_CR1_INT);
	}
	return;
}


void calculate_time(void)
{
	if (captures[0] < captures[1])
		shooter = captures[1] - captures[0];
	else
		shooter = captures[0] - captures[1];

	return;
}





