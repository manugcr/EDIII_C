#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc_types.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"

#define ADC_RATE	750
#define LISTSIZE	6000
#define TIMEOUT		33250

void configADC(void);
void configDAC(void);
void configGPIO(void);
void configEINT0(void);
void configEINT1(void);
void configDMA(void);

void cleanListADC(void);
void moveListDAC(void);

/* Esta lista guarda el sonido grabado por el microfono. */
__IO uint32_t listADC[LISTSIZE] = {0};

/* Variables globales para poder debugear, podrian ser statics dentro del handler. */
uint32_t	listADC_index	= 0;

int main()
{
	configGPIO();
	configEINT0();
	configEINT1();
	configADC();
	configDAC();

	while(1)
	{

	}


	return 0;
}


/* ------------------------------------------------
 * 					FUNCTIONS
 * ------------------------------------------------
 */

void cleanListADC(void)
{
    for (int i = 0; i < LISTSIZE; i++)
    {
    	listADC[i] = 0;
    }

}

void moveListDAC(void)
{
	for(uint32_t i = 0; i<LISTSIZE; i++)
	{
		listADC[i] = listADC[i]<<6;
	}
	return;
}


/* ------------------------------------------------
 * 					CONFIG
 * ------------------------------------------------
 */
void configGPIO(void)
{
	/* Set P0.23 como entrada AD0.0 */
	PINSEL_CFG_Type pinCFG;
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_23;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P2.10 como EINT0*/
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_PULLDOWN;
	pinCFG.Pinnum		= PINSEL_PIN_10;
	pinCFG.Portnum		= PINSEL_PORT_2;
	PINSEL_ConfigPin(&pinCFG);

	/* Set P2.11 como EINT1*/
	pinCFG.Funcnum		= PINSEL_FUNC_1;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_PULLDOWN;
	pinCFG.Pinnum		= PINSEL_PIN_11;
	pinCFG.Portnum		= PINSEL_PORT_2;
	PINSEL_ConfigPin(&pinCFG);

	//Se configura P0.26 como AOUT
	pinCFG.Funcnum		= PINSEL_FUNC_2;
	pinCFG.OpenDrain	= PINSEL_PINMODE_NORMAL;
	pinCFG.Pinmode		= PINSEL_PINMODE_TRISTATE;
	pinCFG.Pinnum		= PINSEL_PIN_26;
	pinCFG.Portnum		= PINSEL_PORT_0;
	PINSEL_ConfigPin(&pinCFG);
}


void configADC(void)
{
	ADC_Init(LPC_ADC, ADC_RATE);
	ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_BurstCmd(LPC_ADC, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
	NVIC_DisableIRQ(ADC_IRQn);
}


void configDAC(void)
{
	DAC_CONVERTER_CFG_Type dacCFG;
	dacCFG.CNT_ENA = SET;
	dacCFG.DMA_ENA = SET;
	DAC_Init(LPC_DAC);

	/* VER COMO CALCULAR ESTO. */
	DAC_SetDMATimeOut(LPC_DAC, TIMEOUT);
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCFG);
}


void configDMA()
{
	GPDMA_LLI_Type LLI1;
	LLI1.SrcAddr = (uint32_t) listADC;
	LLI1.DstAddr = (uint32_t) &LPC_DAC->DACR;
	LLI1.NextLLI = (uint32_t) &LLI1;
	LLI1.Control = LISTSIZE
				   | (2<<18) //source width 32 bits
				   | (2<<21) //dest width 32 bits
				   | (1<<26); //source increment

	GPDMA_Init();

	GPDMA_Channel_CFG_Type GPDMACfg;
	GPDMACfg.ChannelNum = 0;
	GPDMACfg.SrcMemAddr = (uint32_t)listADC;
	GPDMACfg.DstMemAddr = 0;
	GPDMACfg.TransferSize = LISTSIZE;
	GPDMACfg.TransferWidth = 0;
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg.SrcConn = 0;
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	GPDMACfg.DMALLI = (uint32_t)&LLI1;
	GPDMA_Setup(&GPDMACfg);
	GPDMA_ChannelCmd(0, ENABLE);
}


void configEINT0(void)
{
	/* Interrupcion para inicializar la grabacion y el ADC.
	 *
	 * Limpiar la flag DESPUES de el EXTI_Config();
	 *
	 */

	EXTI_InitTypeDef exti;
	exti.EXTI_Mode		= EXTI_MODE_EDGE_SENSITIVE;
	exti.EXTI_polarity	= EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	exti.EXTI_Line		= EXTI_EINT0;

	EXTI_Config(&exti);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);

}


void configEINT1(void)
{
	/* Interrupcion para poner play/pausa el sonido.
	 *
	 * Limpiar la flag DESPUES de el EXTI_Config();
	 *
	 */

	EXTI_InitTypeDef exti;
	exti.EXTI_Mode		= EXTI_MODE_EDGE_SENSITIVE;
	exti.EXTI_polarity	= EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	exti.EXTI_Line		= EXTI_EINT1;

	EXTI_Config(&exti);
	EXTI_ClearEXTIFlag(EXTI_EINT1);
	NVIC_EnableIRQ(EINT1_IRQn);
}


/* ------------------------------------------------
 * 					HANDLERS
 * ------------------------------------------------
 */


void ADC_IRQHandler(void)
{
	/* Tomamos una muestra del ADC y la guardamos en el array sin superar el limite de muestras. */
	static uint32_t count = 0;

	if (count <= LISTSIZE)
	{
		listADC[count] = ((LPC_ADC->ADDR0)>>6) & 0x3FF;
		count++;
	}
	else
	{
		count = 0;
		NVIC_DisableIRQ(ADC_IRQn);
		moveListDAC();
	}

	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
}


void EINT0_IRQHandler(void)
{
	static uint32_t flag = 0;

	if ( flag == 0)
	{
		flag++;
		return;
	}

	/* Comenzamos a grabar un sonido por el ADC. */

	listADC_index		= 0;
	GPDMA_ChannelCmd(0, DISABLE);
	cleanListADC();

	NVIC_EnableIRQ(ADC_IRQn);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
}

void EINT1_IRQHandler(void)
{
	static uint32_t flag = 1;

	if (flag > 0)
	{
		configDMA();
		flag = 0;
	}
	else
	{
		GPDMA_ChannelCmd(0, DISABLE);
		DAC_UpdateValue(LPC_DAC, 0);
		flag = 1;
	}

	EXTI_ClearEXTIFlag(EXTI_EINT1);
}
