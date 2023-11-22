/*
 * Configurar el pin P0.4 como entrada digital con resistencia de pull down y utilizarlo para
 * decidir si el valor representado por los pines P0.0 al P0.3 van a ser sumados o restados al
 * valor guardado en la variable "acumulador".El valor inicial de "acumulador" es 0.
 */

#include <LPC17xx.h>

int main() {
    uint32_t acumulador = 0;
    LPC_GPIO0 -> FIODIRL &= ~(0x4); // Defino el NIBBLE inferior como ENTRADA
    LPC_PINCON -> PINMODE0 = (1<<6 | 1<<7); // en la posicion P0.4 pongo dos 00

    // criterio : 0 es resta y 1 es suma
    while (1) {
        if (LPC_GPIO0 -> FIOPIN & (0x4)) {
            acumulador += LPC_GPIO0 -> FIOPIN & (0xF);
        } else {
            acumulador -= LPC_GPIO0 -> FIOPIN & (0xF);
        }
        return (0);
}