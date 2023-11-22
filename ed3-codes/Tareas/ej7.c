/* Realizar un programa que configure el puerto P0.0 y P2.0 para que provoquen una interrupción 
 * por flanco de subida. Si la interrupción es por P0.0 guardar el valor binario 100111 en la variable 
 * "auxiliar", si es por P2.0 guardar el valor binario 111001011010110.
*/

#include <LPC17xx.h>

uint32_t auxiliar = 0; // Almacena el valor binario.

void EINT3_IRQHandler(void) {
    // Interrupcion por flanco de subida en P0.0
	if (LPC_GPIOINT->IO0IntStatR & (1 << 0)){
		auxiliar = 0b100111;
    	LPC_GPIOINT->IO0IntClr = (1 << 0); // Limpio la bandera
	}
    // Interrupcion por flanco de subida en P2.0
    if (LPC_GPIOINT->IO2IntStatF & (1 << 1)) {
    	auxiliar = 0b111001011010110;
    	LPC_GPIOINT->IO2IntClr = (1 << 0); // Limpio la bandera
    }
}

int main() {
    // P0.0 y P2.0 como entradas
    LPC_GPIO0->FIODIR &= ~(1 << 0);
    LPC_GPIO2->FIODIR &= ~(1 << 0);

    // Interrupcion por flanco de subida en P0.0 (EINT0)
    LPC_SC->EXTMODE |= (1 << 0);
    // Interrupcion por flanco de subida en P2.0 (EINT2)
    LPC_SC->EXTMODE |= (1 << 2);

	// Habilito interrupciones
    NVIC_EnableIRQ(EINT3_IRQn);

    while (1) {
        // Espero la interrupcion.
    }

    return 0;
}
