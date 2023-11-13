#include "LPC17xx.h"
#include "lpc_types.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"

#define ADC_RATE	700
#define LISTSIZE	4000
#define TIMEOUT		10000
#define NUM_LISTS	3

void configADC(void);
void configDAC(void);
void configGPIO(void);
void configEINT0(void);
void configEINT1(void);
void configDMA(uint16_t listADC[]);
void configNVIC(void);
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);

void cleanListADC(void);
void moveListDAC(void);
void buttonDebounce(void);

/* DEBUG. */
__IO uint32_t *samples_count = (__IO uint32_t *)0x2007C000;

/* Esta lista guarda el sonido grabado por el microfono. */
__IO uint16_t listADC[LISTSIZE] = {0};

uint32_t ADCVAL 	= 0;
uint32_t ADCVALMAP	= 0;
__IO uint8_t  RECORDING	= 0;

GPDMA_LLI_Type LLI_Array[NUM_LISTS];
GPDMA_Channel_CFG_Type dmaCFG;

/* ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * --------------------------------------------MAIN------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 */
int main()
{
	configGPIO();
	configEINT0();
	configEINT1();
	configADC();
	configDAC();
	configNVIC();

	while(1)
	{
		// idle...
	}

	return 0;
}




/* ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * ------------------------------------------FUNCTIONS---------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 */
void cleanListADC(void)
{
	/* Rellenar con ceros la lista del ADC. */

    for (uint32_t i = 0; i < LISTSIZE; i++)
    {
    	listADC[i] = 0;
    }

}

void moveListDAC(void)
{
	/* Desplazamos los valor de la lista 6 lugares, 4 para el DAC y 2 mas para recortar los LSB. */

	for (uint32_t i = 0; i < LISTSIZE; i++)
	{
		listADC[i] = listADC[i]<<6;
	}
	return;
}

void buttonDebounce(void)
{
	/* Delay para antirrebote del botones. */

    for (uint32_t i = 0; i < 50000; i++){}
}

uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}




/* ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * -------------------------------------------CONFIGS----------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 */
void configGPIO(void)
{
	/* Set P0.23 como AD0.0 */
	PINSEL_CFG_Type pinCFG;
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_23;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P0.24 como AD0.1 */
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_24;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P2.10 como EINT0*/
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_PULLDOWN;
	pinCFG.Pinnum		= PINSEL_PIN_10;
	pinCFG.Portnum		= PINSEL_PORT_2;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P2.11 como EINT1 */
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_PULLDOWN;
	pinCFG.Pinnum		= PINSEL_PIN_11;
	pinCFG.Portnum		= PINSEL_PORT_2;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P0.26 como AD0.0 */
	pinCFG.Funcnum		= PINSEL_FUNC_2;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_26;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);

	/* P.022 Output LED */
	LPC_GPIO0->FIODIR |= (1<<22);	// Led Rojo
	LPC_GPIO3->FIODIR |= (1<<25);	// Led Verde
	LPC_GPIO3->FIODIR |= (1<<26);	// Led Azul
	LPC_GPIO0->FIOSET |= (1<<22);  	// Apaga el led rojo.
	LPC_GPIO3->FIOSET |= (1<<25);  	// Apaga el led verde.
	LPC_GPIO3->FIOSET |= (1<<26);  	// Apaga el led azul.
}


void configADC(void)
{
	/* El ADC se utiliza en modo burst para tener el maximo de resolucion.
	 * La interrupcion se activa en configNVIC() y comienza apagada ya que se prende con el pulsador en EINT0.
	 */

	ADC_Init(LPC_ADC, ADC_RATE);
	ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	//ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
	ADC_BurstCmd(LPC_ADC, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	//ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);
}


void configDAC(void)
{
	DAC_CONVERTER_CFG_Type dacCFG;
	dacCFG.CNT_ENA = SET;
	dacCFG.DMA_ENA = SET;

	DAC_SetDMATimeOut(LPC_DAC, TIMEOUT);				/* REVISAR CALCULO DE TIMEOUT. */
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCFG);
	DAC_Init(LPC_DAC);
}


void configDMA(uint16_t listADC[])
{
	for (int i = 0; i < NUM_LISTS; i++)
	{
		LLI_Array[i].DstAddr = (uint32_t) &(LPC_DAC->DACR);
		LLI_Array[i].SrcAddr = (uint32_t)(listADC + i * 4095);

		if (i == (NUM_LISTS - 1)) {
			LLI_Array[i].NextLLI = (uint32_t)&LLI_Array[0];
		} else {
			LLI_Array[i].NextLLI = (uint32_t)&LLI_Array[i + 1];
		}

		LLI_Array[i].Control = 4095
							 | (1 << 18) // source width 16 bit
							 | (1 << 22) // dest width = word 32 bits
							 | (1 << 26); // source increment
	}


	dmaCFG.ChannelNum			= 0;
	dmaCFG.TransferSize			= 4095;
	dmaCFG.TransferWidth		= 0;
	dmaCFG.TransferType			= GPDMA_TRANSFERTYPE_M2P;
	dmaCFG.SrcConn				= 0;
	dmaCFG.DstConn				= GPDMA_CONN_DAC;
	dmaCFG.SrcMemAddr			= (uint32_t) listADC;
	dmaCFG.DstMemAddr			= 0;
	dmaCFG.DMALLI				= (uint32_t) &LLI_Array[0];

	GPDMA_Init();
	GPDMA_Setup(&dmaCFG);
	GPDMA_ChannelCmd(0, ENABLE);
	return;
}


void configEINT0(void)
{
	/* Interrupcion para inicializar la grabacion y el ADC.
	 * Las interrupciones se activan en configNVIC()
	 */

	EXTI_InitTypeDef exti;
	exti.EXTI_Mode		= EXTI_MODE_EDGE_SENSITIVE;
	exti.EXTI_polarity	= EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	exti.EXTI_Line		= EXTI_EINT0;

	EXTI_Config(&exti);
}


void configEINT1(void)
{
	/* Interrupcion para poner play/pausa el sonido.
	 * Las interrupciones se activan en configNVIC()
	 */

	EXTI_InitTypeDef exti;
	exti.EXTI_Mode		= EXTI_MODE_EDGE_SENSITIVE;
	exti.EXTI_polarity	= EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	exti.EXTI_Line		= EXTI_EINT1;

	EXTI_Config(&exti);
}


void configNVIC(void)
{
	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
	NVIC_DisableIRQ(ADC_IRQn);

	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);

	EXTI_ClearEXTIFlag(EXTI_EINT1);
	NVIC_EnableIRQ(EINT1_IRQn);

	GPDMA_ChannelCmd(0, DISABLE);
}




/* ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * ------------------------------------------HANDLERS----------------------------------------------
 * ------------------------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------------------------
 */
void ADC_IRQHandler(void)
{
	/* Tomamos una muestra del ADC y la guardamos en el array sin superar el limite de muestras. */

	//if (RECORDING > 0)
	//{
		if (*samples_count <= LISTSIZE)
		{
			LPC_GPIO3->FIOCLR |= (1<<26); 	// Prende el led azul.
			listADC[*samples_count] = ((LPC_ADC->ADDR0)>>6) & 0x3FF;
			(*samples_count)++;
		}
		else
		{
			LPC_GPIO0->FIOCLR |= (1<<22);  	// Prende el led rojo.
			LPC_GPIO3->FIOSET |= (1<<25);  	// Apaga el led verde.
			LPC_GPIO3->FIOSET |= (1<<26); 	// Apaga el led azul.
			*samples_count = 0;
			RECORDING = 0;
			moveListDAC();
		}

	//}
//	else
//	{
//		ADCVAL 		= ((LPC_ADC->ADDR1)>>6) & 0x3FF;
//		ADCVALMAP	= map(ADCVAL, 0, 1024, 8000, 12000);
//		DAC_SetDMATimeOut(LPC_DAC, ADCVALMAP);
//	}


	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
}


void EINT0_IRQHandler(void)
{
	/* Comenzamos a grabar un sonido por el ADC.
	 * Al llenarse el array de valores se detiene automaticamente la grabacion a
	 * la espera de poner en play el sonido.
	 */

	RECORDING = 1;

	buttonDebounce();

	LPC_GPIO0->FIOSET |= (1<<22);  	// Apaga el led rojo.
	LPC_GPIO3->FIOSET |= (1<<25);  	// Apaga el led verde.
	LPC_GPIO3->FIOSET |= (1<<26);  	// Apaga el led azul.
	GPDMA_ChannelCmd(0, DISABLE);
	*samples_count = 0;
	cleanListADC();

	NVIC_EnableIRQ(ADC_IRQn);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
}

void EINT1_IRQHandler(void)
{
	/* Si esta reproduciendo sonido, deshabilita el canal y baja a 0 la salida.
	 * Si esta en pausa, pone en play la reproduccion de sonido.
	 */
	static uint8_t PLAY = 0;

	buttonDebounce();

	if (PLAY > 0) //Esta en pausa
	{
		LPC_GPIO3->FIOSET |= (1<<25);  	// Apaga el led verde.
		LPC_GPIO3->FIOSET |= (1<<26);  	// Apaga el led azul.
		LPC_GPIO0->FIOCLR |= (1<<22);	// Prende el led rojo.
		GPDMA_ChannelCmd(0, DISABLE);
		DAC_UpdateValue(LPC_DAC, 0);
		PLAY = 0;
	}
	else //se reproduce
	{
		LPC_GPIO0->FIOSET |= (1<<22);  	// Apaga el led rojo.
		LPC_GPIO3->FIOSET |= (1<<26);  	// Apaga el led azul.
		LPC_GPIO3->FIOCLR |= (1<<25);	// Prende el led verde.
		configDMA(listADC);
		PLAY = 1;
	}

	EXTI_ClearEXTIFlag(EXTI_EINT1);
}
