#include "LPC17xx.h"

void confGPIO(void); // Prototipo de la funcion de conf. de puertos
void confTimer(void);

int main(void)
{
	confGPIO();
	confTimer();
	while(1){}
	return 0;
}

void confGPIO(void)
{
	LPC_GPIO0->FIODIR |= (1<<22); 		// P0.22 Led de la placa.
	return;
}


void confTimer(void){
	LPC_SC->PCONP       |= (1<<1);  	// pag. 65 Prendemos el TMR0
	LPC_SC->PCLKSEL0    |= (1<<2);  	// pag. 59 Seteamos PCLK (01) osea PCLK = CCLK
	LPC_PINCON->PINSEL3 |= (3<<26); 	// pag. 120 Seteamos P1.29 MAT0.1
	LPC_PINCON->PINSEL3 |= (3<<24);		// pag. 120 Seteamos P1.28 MAT0.0
	LPC_TIM0->EMR       |=(3<<4);   	// pag. 509 MAT0.0 Toggle mode
	LPC_TIM0->EMR       |=(3<<6);		// pag. 509 MAT0.1 Toggle mode
	LPC_TIM0->MR0        = 70000000;	// Valor al match register 0 TMR0
	LPC_TIM0->MR1        = 140000000;	// Valor al match register 1 TMR0
	LPC_TIM0->MCR       &= ~(1<<1); 	// pag. 507 No se resetea si llega a MR0.
	LPC_TIM0->MCR       |= (1<<4);		// Se resetea si llega a MR1
	LPC_TIM0->MCR       |= (1<<0);		// Interrupciones para MR0
	LPC_TIM0->MCR       &= ~(1<<3);		// No hay interrupciones para MR1
	LPC_TIM0->TCR        = 3;       	// pag. 505 Counter enable and reset.
	LPC_TIM0->TCR       &= ~(1<<1);		// The counters remain reset until TCR[1] is returned to zero.
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}


void TIMER0_IRQHandler(void)
{
	static uint8_t i = 0;

	if (i==0)
	{
		LPC_GPIO0->FIOSET = (1<<22);
		i = 1;
	}
	else if (i==1)
	{
		LPC_GPIO0->FIOCLR = (1<<22);
		i = 0;
	}
	LPC_TIM0->IR|=1; 					// Limpia bandera de interrupción
	return;
}

