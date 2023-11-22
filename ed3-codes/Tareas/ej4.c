/*
 * Considerando pulsadores normalmente abiertos conectados en un extremo a
 * masa y en el otro directamente a las entradas P0.0 y p0.1.
 * Realizar un programa que identifique en una variable cual o cuales
 * pulsadores han sido presionados. Las identificaciones posibles a
 * implementar en esta variable van a ser "ninguno", "pulsador 1",
 * "pulsador 2", "pulsador 1 y 2".
 */

#include <LPC17xx.h>

int main() {
	LPC_PINCON-> PINMODE0 &= ~(1 | 1 << 1 | 1 << 2 | 1 << 3);  // 00 en las posiciones 0 y 1 de PIN0 -> PULL-UP
    LPC_GPIO0 -> FIODIRL &= ~(1 | 1 << 1); // Defino el NIBBLE inferior como ENTRADA
    while (1) {
        if (((LPC_GPIO0 -> FIOPIN) & (1 << 1)) && ((LPC_GPIO0 -> FIOPIN) & (1))) {
            char cadena[16] = "pulsador 1 y 2";
        } else if (((LPC_GPIO0 -> FIOPIN) & (1 << 1)) && !((LPC_GPIO0 -> FIOPIN) & (1))) {
            char cadena[16] = "pulsador 1 ";
        }
        if (!((LPC_GPIO0 -> FIOPIN) & (1 << 1)) && ((LPC_GPIO0 -> FIOPIN) & (1))) {
            char cadena[16] = "pulsador 2";
        } else {
            char cadena[16] = "ninguno";
        }
    }
    return (0);
}
