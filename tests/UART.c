#include "LPC17xx.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

#define LISTSIZE 11571

void confPin(void);
void confUart(void);
void UART0_IRQHandler(void);
void UART_IntReceive(void);

uint8_t info[1] = "";
uint16_t info2[LISTSIZE] = {0};

uint32_t count = 0;

int main (void)
{
	confPin();
	confUart();
	while(1){}
	return 0;
}


void confPin(void){
	PINSEL_CFG_Type PinCfg;
	//configuraci�n pin de Tx y Rx
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg); //receptor
	return;
}


void confUart(void){

	UART_CFG_Type      UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	//configuracion por defecto:
	UART_ConfigStructInit(&UARTConfigStruct);
	//inicializa periforico
	UART_Init(LPC_UART2, &UARTConfigStruct);
	//Inicializa FIFO
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);
	// Habilita interrupcion por el RX del UART
	UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);
	// Habilita interrupcion por el estado de la linea UART
	UART_IntConfig(LPC_UART2, UART_INTCFG_RLS, ENABLE);
	//NVIC_SetPriority(UART2_IRQn, 1);
	//Habilita interrupcion por UART2
	NVIC_EnableIRQ(UART2_IRQn);
	return;
}


void UART2_IRQHandler(void){
	uint32_t intsrc, tmp, tmp1;

	//Determina la fuente de interrupcion
	intsrc = UART_GetIntId(LPC_UART2);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Evalua Line Status
	if (tmp == UART_IIR_INTID_RLS)
	{
		tmp1 = UART_GetLineStatus(LPC_UART2);
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// ingresa a un Loop infinito si hay error
		if (tmp1)
		{
			while(1){};
		}
	}

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
	{
		UART_Receive(LPC_UART2, info, sizeof(info), NONE_BLOCKING);
	}

	if (count < LISTSIZE & info[0] != 0)
	{
		info2[count] = (info[0]);
		count++;
	}

	if (count >= LISTSIZE)
		count = 0;

	return;
}

