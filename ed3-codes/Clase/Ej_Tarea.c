/* Ejercicio: Generar una señal de periodo T2. Con T1=T2/n , y n es un número que ingresa por 4 pines
 * del GPIO. Cuando n=0 , la señal es cero y cuando n=15 T1 = T2. (T2=50ms).
 * Cuando se ingresa una señal externa por algún pin de interrupción sacamos la secuencia
 * 0xA0A3 con la característica que los unos son 10ms y los ceros 5 ms.
 *
 * 1 = 10ms
 * 0 = 5ms
 * 0xA0A3 = 1010 0000 1010 0011
 */


#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"

#define INPUT	    (uint8_t) 0
#define	OUTPUT	    (uint8_t) 1
#define	RISING	    (uint8_t) 0
#define FALLING	    (uint8_t) 1
#define PIN(x)		(1<<x)
#define PORT(x)		x

void confGPIO(void);
void confTIMER0(uint32_t num, uint8_t toggle);
void confEXTInt(void);
void EINT0_IRQHandler(void);
void EINT3_IRQHandler(void);

uint32_t numero;

int main(void)
{

	confGPIO();
	confEXTInt();
	confTIMER0(1, 3);

	while(1)
	{
		// Espero interrupciones.
	}


    return 0;
}


void confTIMER0(uint32_t num, uint8_t toggle)
{
	// Del pin P1.28 MAT0.0 va a salir nuestra senal
	PINSEL_CFG_Type pinCFG;

	pinCFG.Portnum		=	PINSEL_PORT_1;
	pinCFG.Pinnum		=	PINSEL_PIN_28;
	pinCFG.Funcnum		=	PINSEL_FUNC_3;					// MAT0.0
	GPIO_SetDir(PINSEL_PORT_1, PIN(28), OUTPUT);
	PINSEL_ConfigPin(&pinCFG);

	// Genero senal T2 = 50ms --> 25ms up - 25ms down. Para debuguear trabajo en [seg].
	TIM_TIMERCFG_Type	timerCFG;
	TIM_MATCHCFG_Type	matchCFG;

	timerCFG.PrescaleOption		=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		=	1;						// TC++ Cada 1[us]

	matchCFG.MatchChannel		=	0;
	matchCFG.IntOnMatch			= 	ENABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	toggle;
	matchCFG.MatchValue			=	2500000/num;			// 2500000 / 1 = 2.5 seg
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM0, &matchCFG);
	TIM_Cmd(LPC_TIM0, ENABLE);

	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_EnableIRQ(TIMER0_IRQn);

	return;
}


void confGPIO(void)
{
	// Seteo como GPIO Input P2[3:0] donde va a estar el numero de 4bits.
	PINSEL_CFG_Type pinCFG;

	pinCFG.Portnum		=	PINSEL_PORT_2;
	pinCFG.Pinnum		=	PINSEL_PIN_0;
	pinCFG.Funcnum		=	PINSEL_FUNC_0;
	pinCFG.Pinmode		=	PINSEL_PINMODE_PULLDOWN;
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Pinnum		=	PINSEL_PIN_1;
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Pinnum		=	PINSEL_PIN_2;
	PINSEL_ConfigPin(&pinCFG);

	pinCFG.Pinnum		=	PINSEL_PIN_3;
	PINSEL_ConfigPin(&pinCFG);

	// Interrupciones para ambos flancos en P2[3:0], osea cualquier cambio de nivel me interrumpe.
	LPC_GPIOINT->IO2IntEnF	|=	0xF;
	LPC_GPIOINT->IO2IntEnR	|=	0xF;
	LPC_GPIOINT->IO2IntClr	|=	0xF;
	NVIC_EnableIRQ(EINT3_IRQn);

	return;
}

void confEXTInt(void)
{
	// Seteamos P2.10 input
	PINSEL_CFG_Type	pinCFG;
	pinCFG.Portnum	=	PINSEL_PORT_2;
	pinCFG.Pinnum	=	PINSEL_PIN_10;
	pinCFG.Pinmode	=	PINSEL_PINMODE_PULLUP;
	pinCFG.Funcnum	=	PINSEL_FUNC_1;			// EINT0
	PINSEL_ConfigPin(&pinCFG);

	// Interrupciones por EINT0 P2.10
	EXTI_InitTypeDef extiCFG;
	extiCFG.EXTI_Line		=	EXTI_EINT0;
	extiCFG.EXTI_Mode		=	EXTI_MODE_LEVEL_SENSITIVE;
	extiCFG.EXTI_polarity	=	EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&extiCFG);

	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_SetPriority(EINT0_IRQn, 0);
	NVIC_EnableIRQ(EINT0_IRQn);

	return;
}

void EINT0_IRQHandler(void)
{
	// TO DO Generar senal 0xA0A3 con 10ms UP - 5ms DOWN.

	EXTI_ClearEXTIFlag(EXTI_EINT0);
	return;
}

void EINT3_IRQHandler(void)
{
	numero	=	(LPC_GPIO2->FIOPIN) & 0xF;		// Guardo el numero introducid

	if (numero == 0)
		confTIMER0(1, 1);
	else if (numero>0 && numero<15)
		confTIMER0(numero, 3);
	else if (numero == 15)
		confTIMER0(1, 3);

	LPC_GPIOINT->IO2IntClr |= 	0xF;			// Limpio la flag de interrupcion.
	return;
}
