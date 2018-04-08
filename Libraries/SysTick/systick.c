/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               systick.c
** Descriptions:            ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-10-30
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "systick.h"
#include "stm32f10x_systick.h"

/* Private variables ---------------------------------------------------------*/	 
static uint8_t  delay_fac_us = 0;  /* us��ʱ������ */
static uint16_t delay_fac_ms = 0;  /* ms��ʱ������ */
static FlagStatus  Status;


/*******************************************************************************
* Function Name  : Delay_Init
* Description    : ��ʼ���ӳٺ��� 
* Input          : None
* Output         : None
* Return         : None
* Attention		 : SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
*******************************************************************************/
void Delay_Init(void)
{
    RCC_ClocksTypeDef RCC_ClocksStatus;

    RCC_GetClocksFreq(&RCC_ClocksStatus);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);  /*ѡ���ⲿʱ��  HCLK/8 */
	
	//SysTick_ITConfig(ENABLE);// DIS 
    delay_fac_us = RCC_ClocksStatus.HCLK_Frequency / 8000000;
    delay_fac_ms = RCC_ClocksStatus.HCLK_Frequency / 8000;      
}
					
/*******************************************************************************
* Function Name  : delay_us
* Description    : ��ʼ���ӳٺ��� 
* Input          : - Nus: ��ʱus
* Output         : None
* Return         : None
* Attention		 : �������ֵΪ 0xffffff / (HCLK / 8000000)
*******************************************************************************/         
#define del 1
void delay_us(u32 Nus)
{ 
	//long i = 0;
	//for(i = 0; i<Nus; i++){};
#ifdef del
    SysTick_SetReload(delay_fac_us * Nus);          /* ʱ����� */
    SysTick_CounterCmd(SysTick_Counter_Clear);		/* ��ռ����� */
    SysTick_CounterCmd(SysTick_Counter_Enable);		/* ��ʼ���� */ 	
    do
    {
        Status = SysTick_GetFlagStatus(SysTick_FLAG_COUNT);
    }while (Status != SET);							/* �ȴ�ʱ�䵽�� */
    SysTick_CounterCmd(SysTick_Counter_Disable);    /* �رռ����� */
	SysTick_CounterCmd(SysTick_Counter_Clear);	    /* ��ռ����� */   
#endif
}


/*******************************************************************************
* Function Name  : delay_ms
* Description    : ��ʼ���ӳٺ��� 
* Input          : - nms: ��ʱms
* Output         : None
* Return         : None
* Attention		 : �������ֵ nms<=0xffffff*8*1000/SYSCLK ��72M������,nms<=1864 
*******************************************************************************/  
void delay_ms(uint16_t nms)
{    
 //long i = 0;
	//for(i = 0; i<nms; i++){};
		
#ifdef del
    uint32_t temp = delay_fac_ms * nms;

    if (temp > 0x00ffffff)
    {
        temp = 0x00ffffff;
    }
    SysTick_SetReload(temp);		             /* ʱ����� */
    SysTick_CounterCmd(SysTick_Counter_Clear);	 /* ��ռ����� */
    SysTick_CounterCmd(SysTick_Counter_Enable);	 /* ��ʼ���� */ 
    do
    {
        Status = SysTick_GetFlagStatus(SysTick_FLAG_COUNT);
    }while (Status != SET);				         /* �ȴ�ʱ�䵽�� */
    SysTick_CounterCmd(SysTick_Counter_Disable); /* �رռ����� */
	SysTick_CounterCmd(SysTick_Counter_Clear);	 /* ��ռ����� */
#endif
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/















