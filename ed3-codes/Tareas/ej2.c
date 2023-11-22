/*
 * En los pines P2.0 a P2.7 se encuentra conectado un display de 7 segmentos.
 * Utilizando la variable numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67}
 * que codifica los números del 0 a 9 para ser mostrados en el display, realizar un
 * programa que muestre indefinidamente la cuenta de 0 a 9 en dicho display.
 */

#include"LPC17xx.h"

void retardo(void);

int main(void){

	uint32_t relojCpu = SystemCoreClock;
	uint8_t numDisplay[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
	uint8_t count = 0;

	LPC_GPIO2 -> FIODIR |= (0xFF);

	while(1){
        LPC_GPIO2->FIOPIN = numDisplay[count];  // Display current count
        delay();
        count = (count + 1) % 10;               // Increment count and wrap around

        // Clear the display after cycling through all numbers
        if (count == 0) {
            LPC_GPIO2->FIOPIN = 0x00;  // Turn off all segments
        }
	}

	return 0;
}

void delay(void){
	uint32_t i;
	for(i = 0 ; i<6000000; i++){};
}
