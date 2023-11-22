/* Escribir un programa en C que permita realizar un promedio movil con los últimos
 * 8 datos ingresados por el puerto 1. Considerar que cada dato es un entero signado
 * y está formado por los 16 bits menos significativos de dicho puerto. El resultado,
 * también de 16 bits, debe ser sacado por los pines P0.0 al P0.11 y P0.15 al P0.18.
 * Recordar que en un promedio movil primero se descarta el dato mas viejo de los 8
 * datos guardados, se ingresa un nuevo dato proveniente del puerto y se realiza la
 * nueva operación de promedio con esos 8 datos disponibles, así sucesivamente.
 * Considerar el uso de un retardo antes de tomar una nueva muestra por el puerto.
 */

#include <LPC17xx.h>

void retardo(uint32_t tiempo);
int main() {
  uint32_t tiempo;
  uint32_t suma;
  uint32_t promedio;
  uint16_t M[8];
  uint32_t a = 0;
  uint32_t pasante = 0;
  // configuro los I/O
  //LPC_GPIO0 -> FIOMASK = 0x00000000;
  LPC_GPIO1 -> FIODIR &= ~(0XFFFF); // Defino el NIBBLE inferior como ENTRADA
  LPC_GPIO0 -> FIODIR |= (0XF << 15); // Defino el pin 15 al 18 como SALIDA del PIN0
  LPC_GPIO0 -> FIODIR |= (0XFFF); // Defino el NIBBLE inferior como SALIDA del PIN0
  //MODO
  //LPC_PINCON -> PINMODE0 &= ~(0XFFFFFFFF); //00 EN TODAS LAS POSICIONES-> PULL-UP
  for (int i = 0; i < 8; i++) {
    M[i] = LPC_GPIO1 -> FIOPINL;
  }
  while (1) {
    if (!((LPC_GPIO1 -> FIOPIN) & (0x0000))) {
      M[pasante] = LPC_GPIO1 -> FIOPINL;
      pasante++;
      if (pasante == 7) {
        pasante = 0;
      }
      for (int i = 0; i < 8; i++) {
        suma += M[i];
      }
      promedio = suma / 8;
      uint32_t aux1;
      uint32_t aux2;
      aux1 = promedio & (0XFFF);
      aux2 = ((promedio) & (0XF << 12));
      LPC_GPIO0 -> FIOSET |= (aux1 | (aux2 << 15));
      tiempo = 1000000;
      retardo(tiempo);
    }
  }
  return (0);
}
void retardo(uint32_t tiempo) {
  uint32_t contador = 0;
  for (contador; contador < tiempo; contador++) {}
}
