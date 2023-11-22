/* Realizar un código en C que permita filtrar las muestras provenientes del ADC con un filtro promediador
 * móvil de N muestras, donde  N inicialmente es igual a 2 y puede ser incrementado en dos unidades cada vez
 * que se presiona un pulsador conectado en P0.6 hasta llegar a un valor de 600.  A su vez, utilizando el bit
 * de overrun y un timer, complete el código realizado para que en caso de producirse el evento de pérdida de
 * datos por parte del ADC, se deje de muestrear y se saque por el pin MATCH2.1 una señal cuadrada de 440 Hz.
 * Considerar una frecuencia de cclk de 60 Mhz y configurar el ADC para obtener una frecuencia de muestreo
 * de 5 Kmuestras/seg.
 *
 *
 * f = 440[Hz]
 * T = 1/440 = 0.002272 [seg] = 2272 [us]
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#define OUTPUT		1
#define INPUT		0
#define RISING		0
#define FALLING		1
#define PORT(x)		x
#define PIN(x)		(1<<x)

uint32_t promediador = 2;

void confGPIO();
void confADC();
void confTIMER();

int main(void)
{
	confGPIO();
	confTIMER();

    return 0;
}


void confTIMER()
{
	// P4.26 Modo Match 2.1. Onda cuadrada 440[Hz] para cclk 60[MHz]
	TIM_TIMERCFG_Type timerCFG;
	TIM_MATCHCFG_Type matchCFG;

	timerCFG.PrescaleOption		=	TIM_PRESCALE_USVAL;
	timerCFG.PrescaleValue		=	1;						// TC++ Cada 1 [us]

	matchCFG.MatchChannel		=	1;
	matchCFG.IntOnMatch			= 	DISABLE;
	matchCFG.ResetOnMatch		=	ENABLE;
	matchCFG.StopOnMatch		=	DISABLE;
	matchCFG.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
	matchCFG.MatchValue			=	2272;					// f = 1/2272 = 440 [Hz]
	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timerCFG);
	TIM_ConfigMatch(LPC_TIM2, &matchCFG);
	TIM_Cmd(LPC_TIM2, ENABLE);

	return;
}


void confADC()
{


	return;
}


void confGPIO()
{
	// P0.6 Interrupciones por GPIO
	PINSEL_CFG_Type pinCFG;
	pinCFG.Portnum		=	0;
	pinCFG.Pinnum		=	26;
	pinCFG.Funcnum		=	1;
	pinCFG.Pinmode		=	PINSEL_PINMODE_PULLUP;
	PINSEL_ConfigPin(&pinCFG);
	GPIO_SetDir(PORT(0), PIN(26), INPUT);
	GPIO_IntCmd(PORT(0), PIN(26), RISING);
	NVIC_EnableIRQ(EINT3_IRQn);

	//P4.26 Modo MAT2.1
	PINSEL_CFG_Type pinCFG;
	pinCFG.Portnum		=	4;
	pinCFG.Pinnum		=	26;
	pinCFG.Funcnum		=	3;
	pinCFG.Pinmode		=	PINSEL_PINMODE_TRISTATE;
	GPIO_SetDir(PORT(4), PIN(26), OUPUT);
	PINSEL_ConfigPin(&pinCFG);

	return;
}

void EINT3_IRQHandler()
{
	if (promediador >= 600)
		promediador = 2;
	else
		promediador += 2;

	GPIO_ClearInt(PORT(0), PIN(26));
	return;
}
