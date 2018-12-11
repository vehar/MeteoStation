#include <stm32f10x.h>
#include <stm32f10x_dma.h>
#include "init.h"

#include "debug.h"
extern uint8_t verboseOutput;

//http://www.avislab.com/blog/stm32-rtc/
unsigned char RTC_Init(void)
{
    // Дозволити тактування модулів управління живленням і управлінням резервної областю
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Дозволити доступ до області резервних даних
    PWR_BackupAccessCmd(ENABLE);
    // Якщо годинник вимкнений - ініціалізувати
    if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
    {
        // Виконати скидання області резервних даних
        RCC_BackupResetCmd(ENABLE);
        RCC_BackupResetCmd(DISABLE);
 
        // Вибрати джерелом тактових імпульсів зовнішній кварц 32768 і подати тактування
        RCC_LSEConfig(RCC_LSE_ON);
        while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY) {}
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
 
        RTC_SetPrescaler(0x7FFF); // Встановити поділювач, щоб годинник рахував секунди
 
        // Вмикаємо годинник
        RCC_RTCCLKCmd(ENABLE);
 
        // Чекаємо на синхронізацію
        RTC_WaitForSynchro();
 
				//NVIC_EnableIRQ (RTC_IRQn);           //разрешить прерывания от RTC
        return 1;
    }
    return 0;
}


/////////////////////////////////


void Spi_Init(void)
{
//включаем тактирование порта B и альтернативных функций 
	RCC->APB2ENR  |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;   
	
//13(SCK) и 15(MOSI) вывод - альтернативная функция  push pull, 14(MISO) вывод - Input floating, 1(CS) вывод - выход, push-pull
	GPIOB->CRH &= ~(GPIO_CRH_CNF13_0 | GPIO_CRH_CNF15_0 | GPIO_CRL_CNF1_0);  
	GPIOB->CRH |= GPIO_CRH_CNF13_1 | GPIO_CRH_CNF15_1;  	
	GPIOB->CRH |= GPIO_CRH_MODE10_0 | GPIO_CRH_MODE13_1 |	GPIO_CRH_MODE15_1;

//включаем тактирование SPI2
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;   
	
    SPI2->CR1 |= SPI_CR1_BR;                //Baud rate = Fpclk/256
    SPI2->CR1 &= ~SPI_CR1_CPOL;             //Polarity cls signal CPOL = 0;
	  SPI2->CR1 |= SPI_CR1_CPHA;             	//Sampled on the falling edge
    SPI2->CR1 |= SPI_CR1_DFF;               //16 bit data
    SPI2->CR1 &= ~SPI_CR1_LSBFIRST;         //MSB will be first
    SPI2->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;  //Software slave management & Internal slave select
	
    SPI2->CR1 |= SPI_CR1_MSTR;              //Mode Master
    SPI2->CR1 |= SPI_CR1_SPE;               //Enable SPI2
	
	//PIN_CONFIGURATION(PB_1);
}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
	    RCC->APB2ENR |=
        RCC_APB2ENR_IOPAEN |
        RCC_APB2ENR_IOPBEN |
        RCC_APB2ENR_IOPCEN |
        RCC_APB2ENR_IOPDEN |
	    //RCC_APB2ENR_USART1EN |
		RCC_APB2ENR_AFIOEN;
	
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE); 
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE); 	
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE); 
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD , ENABLE); 						 
/**
 *	LED1 -> PB0   LED2 -> PB1
 */					 
  /*  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;					    //USER KEY A
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;					   		//USER KEY B
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);*/

PIN_CONFIGURATION(POW_1v5_EN);
//PIN_ON(POW_1v5_EN);

PIN_CONFIGURATION(POW_5v0_EN);
PIN_ON(POW_5v0_EN);

PIN_CONFIGURATION(GSM_ON_OFF);
PIN_ON(GSM_ON_OFF);

PIN_CONFIGURATION(RADIO_SET);
PIN_ON(RADIO_SET);

PIN_CONFIGURATION(USER_KEY_A);
	if(PIN_SIGNAL(USER_KEY_A) == 0)//Pressed on start
	{
		verboseOutput = 1;
	}
}

void SysTikConfig(void)
{
RCC_ClocksTypeDef RCC_Clocks;
RCC_GetClocksFreq(&RCC_Clocks);
SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000); //1ms	
}

void NVIC_Configuration(void)
{
  /*
	NVIC_InitTypeDef NVIC_InitStructure; 
	 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	*/
}

void TIM_Configuration(void)
{
	/*
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
  TIM_DeInit(TIM2);
	
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	
	
  TIM_TimeBaseStructure.TIM_Period = (RCC_Clocks.HCLK_Frequency / 1000); //1ms			 					
																
  TIM_TimeBaseStructure.TIM_Prescaler = 10;				   
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 	
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);							   
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM2, ENABLE);				
*/	
}

void USART_Configuration(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructureTx;
  GPIO_InitTypeDef GPIO_InitStructureRx;
	
  USART_InitTypeDef USART_InitStructure; 
	
  //Debug uart
	
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);
  
  /*
  *  USART1_TX -> PA9 , USART1_RX ->	PA10
  */
	//PIN_CONFIGURATION(USART1_TX); //MODE_AF_PUSH_PULL check it!!!
	//PIN_CONFIGURATION(USART1_RX);
//*	
  GPIO_InitStructureTx.GPIO_Pin = GPIO_Pin_9;	         
  GPIO_InitStructureTx.GPIO_Mode = GPIO_Mode_AF_PP; 
  GPIO_InitStructureTx.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOA, &GPIO_InitStructureTx);		   

  GPIO_InitStructureRx.GPIO_Pin = GPIO_Pin_10;	        
  GPIO_InitStructureRx.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
  GPIO_InitStructureRx.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOA, &GPIO_InitStructureRx);
//*/
	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  USART_InitStructure.USART_BaudRate = 9600; //115200 //9600
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure); 
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
  USART_Cmd(USART1, ENABLE);
  
  //Radiolink uart
  //USART2_TX -> PA2 , USART2_RX ->	PA3
  //USART3_TX -> PB10 , USART2_RX ->	PB11
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
	
  GPIO_InitStructureTx.GPIO_Pin = GPIO_Pin_10;	         
  GPIO_Init(GPIOB, &GPIO_InitStructureTx);		   
  GPIO_InitStructureRx.GPIO_Pin = GPIO_Pin_11;	   
  GPIO_Init(GPIOB, &GPIO_InitStructureRx);
  
  	/* NVIC Configuration */
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
  USART_Init(USART3, &USART_InitStructure); 
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  //USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
  USART_Cmd(USART3, ENABLE);
}
///////////////////////////////////////////

void Adc_Init()  
{
 ADC_InitTypeDef ADC_InitStructure;
	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

 ADC_StructInit(&ADC_InitStructure);
 ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
 ADC_InitStructure.ADC_ScanConvMode = ENABLE;
 ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
 ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
 ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
 ADC_InitStructure.ADC_NbrOfChannel = 1;//1
 ADC_Init(ADC1, &ADC_InitStructure);
	
 ADC_TempSensorVrefintCmd(ENABLE); //Opening the internal temperature sensor
	
 ADC_Cmd(ADC1, ENABLE);
 
 //Channel settings
 //ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_55Cycles5); //A7
 //ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5); //Internal temperature
	
 ADC_ResetCalibration(ADC1);
 while (ADC_GetResetCalibrationStatus(ADC1));
 ADC_StartCalibration(ADC1);
 while (ADC_GetCalibrationStatus(ADC1)); 
}

volatile uint16_t ADCBuffer[] = {0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA};

void ADC_DMA_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    /* Enable ADC1 and GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE );

	DMA_InitStructure.DMA_BufferSize = 4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCBuffer;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1 , ENABLE ) ;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel = 4;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_7Cycles5);
	ADC_Cmd(ADC1 , ENABLE ) ;
	ADC_DMACmd(ADC1 , ENABLE ) ;
	ADC_ResetCalibration(ADC1);

	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd ( ADC1 , ENABLE ) ;
}