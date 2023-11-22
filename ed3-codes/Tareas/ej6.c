/* Realizar un programa que configure el puerto P2.0 y P2.1 para que provoquen una interrupción por 
 * flanco de subida para el primer pin y por flanco de bajada para el segundo. Cuando la interrupción 
 * sea por P2.0 se enviará por el pin P0.0 la secuencia de bits 010011010. Si la interrupción es por 
 * P2.1 se enviará por el pin P0.1 la secuencia 011100110. Las secuencias se envían únicamente cuando 
 * se produce una interrupción, en caso contrario la salida de los pines tienen valores 1 lógicos. 
 * ¿que prioridad tienen configuradas por defecto estas interrupciones?
*/

#include <LPC17xx.h>

void EINT3_IRQHandler(void) {
    // Interrupcion por flanco de subida en P2.0
    if (LPC_GPIOINT->IO2IntStatR & (1 << 0)) {
        LPC_GPIO0->FIOCLR = (1 << 0); // Limpio el bit 0 del puerto P0
        LPC_GPIO0->FIODIR = (LPC_GPIO0->FIODIR & 0xFFFFFFE0) | (0b010011010); // 010011010
        LPC_GPIOINT->IO2IntClr = (1 << 0); // Limpiar la bandera
    }
    // Interrupción por flanco de bajada en P2.1
    if (LPC_GPIOINT->IO2IntStatF & (1 << 1)) {
        LPC_GPIO0->FIOCLR = (1 << 1); // Limpiar el bit 1 del puerto P0
        LPC_GPIO0->FIODIR = (LPC_GPIO0->FIODIR & 0xFFFFFFE0) | (0b011100110); // 011100110
        LPC_GPIOINT->IO2IntClr = (1 << 1); // Limpiar la bandera
    }
}

int main() {
    // Configuro P0.0 y P0.1 como salidas iniciales con valor alto
    LPC_GPIO0->FIODIR |= (3 << 0); // 3 = 0b11
    LPC_GPIO0->FIOPIN |= (3 << 0);

    LPC_GPIO2->FIODIR &= ~(3 << 0); // P2.0 y P2.1 como entradas

    LPC_GPIOINT->IO2IntEnR |= (1 << 0); // Flanco de subida en P2.0
    LPC_GPIOINT->IO2IntEnF |= (1 << 1); // Flanco de bajada en P2.1

    NVIC_EnableIRQ(EINT3_IRQn); // Habilito interrupciones

    while (1) {
        // Espero la interrupcion.
    }

    return 0;
}
