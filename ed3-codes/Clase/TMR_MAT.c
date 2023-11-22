//#include "LPC17xx.h"
//
//void confGPIO(void); // Prototipo de la funcion de conf. de puertos
//void confTimer(void);
//
//int main(void) {
//
//	confGPIO();
//	confTimer();
//	while(1){}
//	return 0;
//}
//
//void confGPIO(void){
//	LPC_GPIO0->FIODIR |= (1<<22);
//	return;
//}
//
//void confTimer(void){
//	LPC_SC->PCONP       |= (1<<1);  // pag. 65
//	LPC_SC->PCLKSEL0    |= (1<<2);  // pag. 59
//	LPC_PINCON->PINSEL3 |= (3<<24); // pag. 120
//	//LPC_PINCON->PINSEL7 |= (2<<18);
//	LPC_TIM0->EMR       |=(3<<4);   // pag. 509
//	LPC_TIM0->MR0        = 70000000;//
//	LPC_TIM0->MCR       |= (1<<1);       // pag. 507
//	LPC_TIM0->MCR       &= ~(1<<0);       // pag. 507
//	LPC_TIM0->TCR        = 3;       // pag. 505
//	LPC_TIM0->TCR       &= ~(1<<1);
//	NVIC_EnableIRQ(TIMER0_IRQn);
//	return;
//}
//
//void TIMER0_IRQHandler(void) //ISR del timer0
//{
//	static uint8_t i = 0;
//	if (i==0){
//		LPC_GPIO0->FIOSET = (1<<22);
//		i = 1;
//	}
//	else if (i==1){
//		LPC_GPIO0->FIOCLR = (1<<22);
//		i = 0;
//	}
//	LPC_TIM0->IR|=1; //Limpia bandera de interrupci�n
//	return;
//}
//
//



/*
 * Estudiante:
 * Carrera:
 * Descripci�n: Este programa pende y apaga un led
 *              conectado al pin P0.22 de forma intermitente
 *
 */

#include"LPC17xx.h"

void retardo(void);

int main(void){

	uint32_t relojCpu = SystemCoreClock;

	//LPC_PINCON->PINSEL1 &= ~(3<<12);   // 0B00111111111111 & 0B10010101010101 = 0B00_010101010101
	LPC_GPIO0->FIODIR     |= (1<<22);     // 0B1000...000 | 0B010101..101 = 1_10101..101
	//LPC_GPIO0->FIOMASK =0xFFFFFFFF;  //~FIOMASK & REGISTRO =0000

	while(1){
		LPC_GPIO0->FIOCLR |= (1<<22);  // apaga el led
		retardo();
		LPC_GPIO0->FIOSET |= (1<<22);  // apaga el led
		retardo();
	}

	return 0;
}

void retardo(void){

	uint32_t contador;
	for(contador =0 ; contador<6000000; contador++){};

}
