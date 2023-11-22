#include "LPC17xx.h"

void confGPIO(void); // Prototipo de la funcion de conf. de puertos
void confTimer(void);
uint32_t aux = 0;

int main(void)
{
	confGPIO();
	confTimer();
	while(1){}
	return 0;
}

void confGPIO(void)
{
	LPC_GPIO0->FIODIR |= (1<<22);
	return;
}

void confTimer(void)
{
	LPC_SC->PCONP       |= (1<<1); //Por defecto timer 0 y 1 estan siempre prendidos
	LPC_SC->PCLKSEL0    |= (1<<2); // configuraci�n del clock de perif�rico
	LPC_PINCON->PINSEL3 |= (3<<20);        //pag. 120
	LPC_TIM0->CCR       |= (1<<1)|(1<<2);  //pag. 508
	LPC_TIM0->TCR        = 3;              //pag. 505
	LPC_TIM0->TCR       &= ~(1<<1);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

void TIMER0_IRQHandler(void)
{
	static uint8_t i = 0;
	aux = LPC_TIM0->CR0; //Variable auxiliar para observar el valor del registro de captura

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

	LPC_TIM0->IR |= 1; //Limpia bandera de interrupci�n
	return;
}
