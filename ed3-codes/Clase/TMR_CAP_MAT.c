//Autor: J. Martin Ayarde
//Descripción: Este código genera una señal cuadrada mediante el timer2 en modo Match
//             y es utilizada para generar los eventos que captura el Timer 0 en modo Capture.

#include "LPC17xx.h"

void confTimerMatch(void);
void confTimerCapt(void);
uint32_t aux = 0;

int main(void)
{
	confTimerCapt();
	confTimerMatch();
	while(1){}			// Espero interrupcion.
	return 0;
}


void confTimerCapt(void)
{
	LPC_SC->PCONP       |= (1<<1);			// pag. 65 Prendemos el tmr0
	LPC_SC->PCLKSEL0    |= (1<<2);			// pag. 58 Seleccionamos frecuencia para timer0 (01) osea PCLK = CCLK
	LPC_PINCON->PINSEL3 |= (3<<20);			// pag. 120 PIN 1.26 como CAP0.0 (11)
	LPC_TIM0->CCR       |= (1<<1)|(1<<2); 	// pag. 508 Falling edge para CAP0.0 e Interrupciones por CAP0.0
	LPC_TIM0->TCR        = 3;             	// pag. 505 Reseteamos y habilitamos el contador.
	LPC_TIM0->TCR       &= ~(1<<1);			// Deshabilitamos el contador.
	NVIC_EnableIRQ(TIMER0_IRQn);			// Habilitamos interrupciones por TMR0
	return;
}

void confTimerMatch(void)
{
	LPC_SC->PCONP       |= (1<<22);		// pag. 65 Prendemos el tmr2
	LPC_PINCON->PINSEL0 |= (3<<18);		// pag. 118 PIN 0.9 como MAT2.3 (11)
	LPC_TIM2->EMR       |=(3<<10);		// pag. 509 MAT2.3 toggle mode.
	LPC_TIM2->MR3        = 70000000;	// Le damos un valor al match register.
	LPC_TIM2->MCR       |= (1<<10);		// pag. 507 Reset on MR3
	LPC_TIM2->MCR       &= ~(1<<9);		// pag. 507 Deshabilitamos interrupciones.
	LPC_TIM2->TCR        = 3;      		// pag. 505 Reseteamos y habilitamos el contador.
	LPC_TIM2->TCR       &= ~(1<<1);    	// Se deshabilita el contador.
	return;
}

void TIMER0_IRQHandler(void) //ISR del timer0
{
	static uint8_t i = 0;
	aux = LPC_TIM0->CR0;	// Variable auxiliar para observar el valor del registro de captura
	LPC_TIM0->IR |= 1;		// Limpia bandera de interrupción
	return;
}


//
///*
// *  Times calculation
// * PR=4999 --> TC increments on every PR+1/CCLK = 50[us]
// *
// * 		f=1[Hz]     --> T=1000[ms] --> Ttogle=500[ms]  --> MATCH = (500[ms]/50[us])-1 = 9999
// * 		f=10[Hz]    --> T=100[ms]  --> Ttogle=50[ms]   --> MATCH = (50[ms]/50[us])-1 = 999
// * 		f=100[Hz]   --> T=10[ms]   --> Ttogle=5[ms]    --> MATCH = (5[ms]/50[us])-1  = 99
// */
//
//#include "LPC17xx.h"
//
//#define SET_PR 4999
//
//void configEint(void);
//void configTimer0(void);
//void configTimer1(void);
//
//uint8_t state = 0;
//uint32_t count = 0;
//float period = 0;
//
//int periods[3] = {9999, 999, 99};
//
//int main(void)
//{
//    configEint();
//    configTimer0();
//    configTimer1();
//
//    while (1)
//    {
//    }
//    return 0;
//}
//
//void configEint(void)
//{
//    LPC_PINCON->PINSEL4 |= (1 << 20);	// Seteamos interrupcion externa P2.10 (01)
//    LPC_SC->EXTINT |= (1);				// Limpiamos la bandera.
//    LPC_SC->EXTMODE |= (1);				//
//    LPC_SC->EXTPOLAR |= (1);
//    NVIC_SetPriority(EINT0_IRQn, 1);
//    NVIC_EnableIRQ(EINT0_IRQn);
//}
//
//void configTimer0(void)
//{
//    LPC_PINCON->PINSEL3 |= (3 << 24); // P1.28 as MAT0.0
//
//    LPC_SC->PCONP |= (1 << 1);
//    LPC_SC->PCLKSEL0 |= (1 << 2); // PCLK = cclk
//
//    LPC_TIM0->PR = SET_PR;
//    LPC_TIM0->MR0 = periods[0];
//    LPC_TIM0->MCR = 2;         // Timer0 reset on Match0
//    LPC_TIM0->EMR |= (3 << 4); // MAT0.0 toggle mode
//    LPC_TIM0->IR |= 0x3F;      // Clear all interrupt flag
//    LPC_TIM0->TCR = 3;         // Enable and Reset
//    LPC_TIM0->TCR &= ~2;
//}
//
//void configTimer1(void)
//{
//    LPC_PINCON->PINSEL3 |= (3 << 4);	// P1.18 as CAP0.1
//
//    LPC_SC->PCONP |= (1 << 2);
//    LPC_SC->PCLKSEL0 |= (1 << 4); 		// PCLK = cclk
//    LPC_TIM1->PR = SET_PR;
//
//    LPC_TIM1->CCR |= (5); 				// CAP1.0 rising edge interrupt
//
//    LPC_TIM1->IR |= 0x3F; 				// Clear all interrupt flags
//    LPC_TIM1->TCR = 3;    				// Enable and Reset
//    LPC_TIM1->TCR &= ~2;
//    NVIC_EnableIRQ(TIMER1_IRQn);
//}
//
//void EINT0_IRQHandler(void)
//{
//    state++;
//    if (state > 2)
//        state = 0;
//
//    LPC_TIM0->TCR |= 2;
//    LPC_TIM0->MR0 = periods[state];
//    LPC_TIM0->TCR &= ~(2);
//
//    LPC_SC->EXTINT |= 1;
//}
//
//void TIMER1_IRQHandler(void)
//{
//
//    uint32_t oldCount = count;
//    count = LPC_TIM1->CR0;
//
//    period = (count - oldCount) * 0.05;
//
//    LPC_TIM1->IR |= 1 << 4;
//}
//
