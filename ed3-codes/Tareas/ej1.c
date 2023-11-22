/* Una famosa empresa de calzados a incorporado a sus zapatillas 10 luces leds
 * comandadas por un microcontrolador LPC1769 y ha pedido a su grupo de ingenieros
 * que diseñen 2 secuencias de luces que cada cierto tiempo se vayan intercalando
 * (secuencia A - secuencia B- secuencia A- ... ). Como todavía no se ha definido
 * la frecuencia a la cual va a funcionar el CPU del microcontrolador,
 * las funciones de retardos que se incorporen deben tener como parametros de
 * entrada variables que permitan modificar el tiempo de retardo que se vaya a
 * utilizar finalmente. Se pide escribir el código que resuelva este pedido,
 * considerando que los leds se encuentran conectados en los puertos P0,0 al P0.9.
 * Adjuntar el archivo .c bajo el nombre "eje1ApellidoDelEstudiante.c"
*/

#include <LPC17xx.h>

void delay(uint32_t ms){
	uint32_t i;
	for(i = 0 ; i<ms; i++){};
}

int main() {
	uint32_t relojCpu = SystemCoreClock;

    LPC_GPIO0->FIODIR |= 0x03FF;  // Set P0.0 to P0.9 as output

    while (1) {
        // Sequence A
        LPC_GPIO0->FIOPIN = 0x0000;  // Turn off all LEDs
        delay(500);
        LPC_GPIO0->FIOPIN = 0x03FF;  // Turn on all LEDs
        delay(500);

        // Sequence B
        LPC_GPIO0->FIOPIN = 0x02AA;  // Turn on alternate LEDs
        delay(500);
        LPC_GPIO0->FIOPIN = 0x0155;  // Turn on alternate LEDs
        delay(500);
    }

    return 0;
}