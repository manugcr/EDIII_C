///*
//===============================================================================
// Name        : Timer_and_GPIO.c
// Author      :
// Version     :
//===============================================================================
//*/
//
//#include "LPC17xx.h"
//
///*!
// * Header files to project. Include library
// */
//
//#include "lpc17xx_timer.h"
//#include "lpc17xx_gpio.h"
//#include "lpc17xx_pinsel.h"
//
//
//// TODO: insert other definitions and declarations here
//
//#define	OUTPUT	    (uint8_t) 1
//#define INPUT	    (uint8_t) 0
//
//#define PIN_22		((uint32_t)(1<<22))
//#define PORT_ZERO	(uint8_t)	0
//#define PORT_ONE	(uint8_t)	1
//#define PORT_TWO	(uint8_t)	2
//#define PORT_THREE	(uint8_t)	3
//
//void config_GPIO(void);
//void config_timer(void);
//
//void TIMER0_IRQHandler(void){
//	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
//
//	if (GPIO_ReadValue(PORT_ZERO)&PIN_22) {
//		GPIO_ClearValue(PORT_ZERO, PIN_22);
//	} else {
//		GPIO_SetValue(PORT_ZERO,PIN_22);
//	}
//
//	return;
//}
//
//int main(void) {
//
//	config_GPIO();
//	config_timer();
//	GPIO_SetValue(PORT_ZERO,PIN_22);
//
//	while(1) {}
//    return 0 ;
//}
//
//void config_GPIO(){
//	PINSEL_CFG_Type pin_configuration;
//
//	pin_configuration.Portnum 	=	PINSEL_PORT_0;
//	pin_configuration.Pinnum	=	PINSEL_PIN_22;
//	pin_configuration.Pinmode	=	PINSEL_PINMODE_PULLUP;
//	pin_configuration.Funcnum	= 	PINSEL_FUNC_0;
//	pin_configuration.OpenDrain	=	PINSEL_PINMODE_NORMAL;
//
//	PINSEL_ConfigPin(&pin_configuration);
//
//	GPIO_SetDir( PORT_ZERO , PIN_22 , OUTPUT );
//
//	return;
//}
//
//void config_timer(){
//	TIM_TIMERCFG_Type	struct_config;
//	TIM_MATCHCFG_Type	struct_match;
//
//	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;
//	struct_config.PrescaleValue		=	1;
//
//	struct_match.MatchChannel		=	1;
//	struct_match.IntOnMatch			=	ENABLE;
//	struct_match.ResetOnMatch		=	ENABLE;
//	struct_match.StopOnMatch		=	DISABLE;
//	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
//	struct_match.MatchValue			=	1000000;
//
//	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
//	TIM_ConfigMatch(LPC_TIM0, &struct_match);
//
//	TIM_Cmd(LPC_TIM0, ENABLE);
//
//	NVIC_EnableIRQ(TIMER0_IRQn);
//
//}


#include "LPC17xx.h"

uint16_t ADC0Value = 0;

void confGPIO(void); // Prototipo de la funci�n de conf. de puertos
void confADC(void); //Prototipo de la funci�n de conf. de interrupciones externas
void confTimer(void);
void retardo(void);

int main(void) {
	confGPIO();
	confTimer();
	confADC();
	while(1){
		LPC_GPIO0->FIOSET = (1<<22);
		retardo();
		LPC_GPIO0->FIOCLR = (1<<22);
		retardo();
	}
	return 0;
}

void retardo (void){
	uint32_t conta;
	for(conta = 0; conta<1000000;conta++){}
	return;
}

void confGPIO(void){
	LPC_GPIO0->FIODIR |= (1<<22)|(1<<9);
	return;
}

void confADC(void){
	LPC_SC->PCONP |= (1 << 12);
	LPC_ADC->ADCR |= (1 << 21);    //enable ADC
	LPC_SC->PCLKSEL0 |= (3<<24);  //CCLK/8
	LPC_ADC->ADCR &=~(255 << 8);  //[15:8] CLKDIV
	//LPC_ADC->ADCR |= (1 << 0);   // channel
	LPC_ADC->ADCR &= ~(1 << 16);   // no burst
	LPC_ADC->ADCR |= (1 << 26);    // match
	LPC_ADC->ADCR &= ~(3 << 24);   // match
	LPC_ADC->ADCR |= (1 << 27);    //edge
	//LPC_PINCON->PINSEL4 |= (1<<20);
	LPC_PINCON->PINMODE1 |= (1<<15); //neither pull-up nor pull-down.
	LPC_PINCON->PINSEL1 |= (1<<14);
	LPC_ADC->ADINTEN |= (1<<0);
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}

void confTimer(void){
	LPC_SC->PCONP |= (1 << 1); //Por defecto timer 0 y 1 estan siempre prendidos
	LPC_SC->PCLKSEL0 |= (1<<2); // configuraci�n del clock del perif�rico
	//LPC_PINCON->PINSEL3 |= (3<<24);
	LPC_TIM0->EMR|=(3<<6); //toggle
	LPC_TIM0->MR1 = 100000000;
	LPC_TIM0->MCR = (1<<4); //reset on match
	LPC_TIM0->TCR = 3;      // hab y reset
	LPC_TIM0->TCR &= ~(1<<1);
	return;
}


void ADC_IRQHandler(void)
{
	float volt = 0;
	ADC0Value = ((LPC_ADC->ADDR0)>>4) & 0xFFF; //Variable auxiliar para observar el valor del registro de captura
	if(ADC0Value>2055)
		LPC_GPIO0->FIOSET |= (1<<9);
	else
		LPC_GPIO0->FIOCLR |= (1<<9);
	volt=(ADC0Value/4096)*3.3;

	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;

	return;
}


