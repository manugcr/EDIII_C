/* Utilizando los 4 registros match del Timer 0 y las salidas P0.0, P0.1, P0.2 y P0.3,
 * realizar un programa en C que permita obtener las formas de ondas adjuntas, donde los pulsos en alto
 * tienen una duración de 5 mseg. Un pulsador conectado a la entrada EINT3, permitirá elegir
 * entre las dos secuencias mediante una rutina de servicio a la interrupción. La prioridad de la
 * interrupción del Timer tiene que ser mayor que la del pulsador. Estas formas de ondas son muy
 * útiles para controlar un motor paso a paso.
 *
 * Secuencia 1:
 * 	A --> 1 0 0 0 1 0 0 0 1 0
 * 	B --> 0 1 0 0 0 1 0 0 0 1
 * 	C --> 0 0 1 0 0 0 1 0 0 0
 * 	D --> 0 0 0 1 0 0 0 1 0 0
 *
 * Secuencia 2:
 * 	A --> 1 1 0 0 1 1 0 0 1 1
 * 	B --> 0 1 1 0 0 1 1 0 0 1
 * 	C --> 0 0 1 1 0 0 1 1 0 0
 * 	D --> 0 0 0 1 1 0 0 1 1 0
 *
 *
 *	NO TERMINADO
 */

#include "LPC17xx.h"

uint8_t sequence_1[4] = {1000100010, 0100010001, 0010001000, 0001000100};
uint8_t sequence_2[4] = {1100110011, 0110011001, 0011001100, 0001100110};
uint8_t *current_sequence = 0;

void confGPIO(void); 					// Config de puertos
void confGPIOInt(void); 				// Interrupciones por GPIO
void confTimerMatch(void);				// Config del timer
void setPins(uint8_t state);			// Para cambiar el estado de los pines segun la secuencia

int main(void)
{
	confGPIO();
	confGPIOInt();
	confTimerMatch();

	while(1)
	{

	}

	return 0;
}

void confGPIO(void)
{
	LPC_GPIO0->FIODIR &=~ (1<<4); 		// Seteo P0.4 como entrada.
	LPC_GPIO0->FIODIR |= (0xF);			// Seteo P0[3:0] como salida.
	return;
}

void confGPIOInt(void)
{
	LPC_GPIOINT->IO0IntEnF |= (1<<4); 	// Int flanco de bajada P0.4
	LPC_GPIOINT->IO0IntClr |= (1<<4); 	// Limpiamos la flag P0.4
	NVIC_EnableIRQ(EINT3_IRQn);        	// Habilita de interrupciones por GPIO0.
	return;
}

void confTimerMatch(void)
{
	LPC_SC->PCONP       |= (1<<1);		// Tab. 46 Prendemos el TMR0
	LPC_SC->PCLKSEL0    |= (1<<2);  	// pag. 59 Seteamos PCLK TMR0 (01) PCLK = CCLK
	LPC_PINCON->PINSEL3 |= (3<<24);		// pag. 120 PIN 1.28 como MAT0.0 (11)
	LPC_TIM0->EMR       |= (3<<4);		// pag. 509 MAT0.0 toggle mode.
	LPC_TIM0->MR0        = 70000000;	// Le damos un valor al match register.
	LPC_TIM0->MCR       |= (1<<0);		// pag. 507 habilitamos interrupciones.
	LPC_TIM0->MCR       |= (1<<1);		// pag. 507 Reset on MR0.
	LPC_TIM0->TCR        = 3;      		// pag. 505 Reseteamos y habilitamos el contador.
	LPC_TIM0->TCR       &= ~(1<<1);    	// Se saca el tmr del reset
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

void setPins(uint8_t state)
{
	// Configurar los pines P0.0 a P0.3 según el estado proporcionado
	LPC_GPIO0->FIOPIN = (LPC_GPIO0->FIOPIN & 0xFFFFFFF0) | (state & 0x0F);
}

void TIMER0_IRQHandler(void)
{
	static uint8_t sequence_index = 0;	// Seguimiento del indice actual

	// Configurar los pines P0.0 a P0.3 segun el estado actual en la secuencia
	setPins(sequence_1[sequence_index]);

	// Incrementar el indice y ajustarlo al tamano del array
	sequence_index = (sequence_index + 1) % 4;

	LPC_TIM0->IR |= 1; 					// Limpiamos la flag de interrupcion.
	return;
}

void EINT3_IRQHandler(void)
{
	if (current_sequence % 2 == 0)
		current_sequence = sequence_1;
	else
		current_sequence = sequence_2;

	LPC_GPIOINT->IO0IntClr |= (1<<4); 	// Limpiamos la flag de interrupcion.
	return;
}
