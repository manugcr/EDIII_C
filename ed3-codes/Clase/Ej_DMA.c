/*
 * Considerando que se tiene un bloque de datos comprendidos entre las posiciones de memorias,
 * dirección inicial = 0x10000800
 * dirección final = 0x10001000
 * ambas inclusive y se desea trasladar este bloque de datos una nueva zona de memoria comprendida entre la
 * dirección inicial= 0x10002800
 * dirección final = 0x10003000 (en el mismo orden).
 * Teniendo en cuenta además que los datos contenidos dentro de la zona de memoria son de 16 bits
 * (AHB Master endianness configuration - por defecto) y que estos deben moverse de a uno (1)
 * en cada evento de DMA, se sincronizará la transmisión con evento de match0 del timer1.
 * Se pide que Enumere y explique los puntos a tener en cuenta para configurar correctamente el controlador DMA.
 *
 */

/*
	1- Se setea el pin PCGDMA en el registro PCONP. Esto habilita el periferico DMA.
	2- Se setea el clock del periferico, en este caso el DMA utiliza el mismo clock que el core. Modificando el registro CCLKCFG indicamos con 8 bits el valor por el cual queremos dividir la frecuencia.
	3- Pasamos a configurar el DMA, si usamos CMSIS se crea una variable de tipo GPDAM_Channel_CFG_Type channel o sino se modifican los registros del canal que se este utilizando:
		3.1- Definimos el canal, el DMA cuenta con 8 canales. En este caso elijo el canal 0 -> channel.ChannelNum=0;
		3.2- Se define el tamaño de la transferencia, en este caso es 0x10001000-0x10000800 = 0x800 -> channel.TransferSize = 0x800;
		3.3- Como es de memoria a memoria hay quee definir el ancho de la transferencia, en este caso es de 16 bits -> 	     channelTransferWidht = GPDMA_WIDTH_HALFWORD;
		3.4- Hay que definir el tipo de tranferencia, en ese caso de memoria a memoria.
		3.5- Definir el periferico de la fuente y de destino, en este caso como es memoria a memoria ambos son 0;
		3.6- Definir la lista.
	4- Definimos la lista fuente:
		4.1- Direccion de la fuente = 10000800
		4.2- direccion de destino = 0x10002800
		4.3- se define la siguiente lista, en este caso es 0 ya que no hay que linkear listas.
		4.4- Se define el registro de control:
			4.4.1- en los primeros 12 bits definimos el tamaño de la transferencia = 0x800
			4.4.2- colocamos un 1 en la posicion 19 y en la posicion 22 indicando que nuestros datos tienen ancho de 16 bits
			4.4.3- Colocamos un 1 en los bits 26 y 27 que indican que las posiciones fuente y de destino se incrementes a medida 		       que se transfieren datos
	5- Configuramos para que se sincronice la transmision con evento de match0 del timer1:
		5.1- Ponemos en 0 el bit 10 del registro DMACSync que habilita la sincronizacion del DMA con el UART1 Tx o MAT1.0
		5.2- Ponemos en 1 el bit 2 del registro DMAReqSel que indica que se sincronizara con el MAT1.0
	6- Llamar a la funcion GPDMA_Setup(&channel) y enviarle como parametro un puntero de la estructura GPDAM_Channel_CFG_Type
 */

void confDMA()
{
	GPDMA_Channel_CFG_Type dmaCFG;
	dmaCFG.SrcAddr	= (uint32_t *) srcAddress;
	dmaCFG.DstAddr	= (uint32_t *) dstAddress;
	dmaCFG.NextLLI	= 0;
	dmaCFG.Control	= DMA_SWAP_SIZE
					| (1<<18) // source width 16 bit
					| (1<<21) // dest. width 16 bit
					| (1<<26) // source increment
					;

	/* GPDMA block section -------------------------------------------- */
	NVIC_DisableIRQ(DMA_IRQn);
	GPDMA_Init();
	GPDMACfg.ChannelNum		= 7;
	GPDMACfg.SrcMemAddr		= (uint32_t *) srcAddress;
	GPDMACfg.DstMemAddr		= (uint32_t *) dstAddress;
	GPDMACfg.TransferSize	= DMA_SIZE;
	GPDMACfg.TransferWidth	= GPDMA_WIDTH_HALFWORD;
	GPDMACfg.TransferType	= GPDMA_TRANSFERTYPE_M2M;
	GPDMACfg.SrcConn		= 0;
	GPDMACfg.DstConn		= 0;
	GPDMACfg.DMALLI			= 0;
	GPDMA_Setup(&GPDMACfg);


	return;
}
