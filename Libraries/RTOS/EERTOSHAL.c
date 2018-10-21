#include "EERTOSHAL.h"

//+++++++++++++PRIVATE RTOS INT VARS+++++++++++++++++++++++++++++++
 uint32_t CPSR_;
 bit	       global_nointerrupted_flag;
 U_ALU_INT      global_interrupt_mask;
 U_ALU_INT      global_interrupt_cond;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifdef AVR_CORE
	//RTOS Запуск системного таймера
	void RTOS_timer_init (void)
	{
		TCNT2 = 0;                                // Установить начальное значение счётчиков
		OCR2  = LO(RtosTimerDivider);         		// Установить значение в регистр сравнения
		TIMSK = (0<<TOIE2)|(1<<OCIE2);
	}

	void RunRTOS (void)
	{
		ATOMIC_BLOCK_FORCEON
		{
				TCCR2 = (1<<WGM21)|(1<<CS22)|(0<<CS20)|(0<<CS21);    // Freq = CK/256 - Установить режим и предделитель
					TIMSK = (0<<TOIE2)|(1<<OCIE2);
		}// Разрешаем прерывание RTOS - запуск ОС // Автосброс после достижения регистра сравнения
	}

		#warning поэкспериментировать с переменным RtosTimerDivider в SuspendRTOS и в зависимости от задач(возможно их приоритета)
	//RTOS увеличение предделителя системного таймера
	void SuspendRTOS (void)//Фактически снижение частоты системного таймера
	{
		ATOMIC_BLOCK_FORCEON
		{
			TCCR2 = (0<<CS21)|(1<<CS22)|(1<<CS20); // Freq = CK/1024
		}
	}

	//RTOS Остановка системного таймера
	void FullStopRTOS (void)
	{
		ATOMIC_BLOCK_FORCEON
		{
		TCCR2 = 0;                        // Сбросить режим и предделитель
		TIMSK = (0<<TOIE2)|(0<<OCIE2);	 // запрещаем прерывание RTOS - остановка ОС
		}
	}

	void DeadTimerInit (void)
	{
		TCCR0 = (1<<WGM01)|(1<<CS02)|(0<<CS00)|(0<<CS01);
		TCNT0=0x00;
		OCR0=LO(DeadTimerDivider);
		TIMSK = (0<<TOIE0)|(1<<OCIE0);
	}
#endif


#ifdef ARM_CORE

	#define TM3_INIT_VALUE 0x000000FF
	//RTOS Запуск системного таймера
void RTOS_timer_init (void)
{
			NVIC_InitTypeDef         NVIC_InitStructure;
			TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
			//////----------FOR TIM 3 ------------------------------------
			/*
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
			NVIC_Init(&NVIC_InitStructure);

			TIM_TimeBaseStructure.TIM_Period        = 1000;//TM3_INIT_VALUE;
			TIM_TimeBaseStructure.TIM_Prescaler     = 0;
			TIM_TimeBaseStructure.TIM_ClockDivision = 0;
			TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Down;
			TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

			TIM_PrescalerConfig  (TIM3, 0, TIM_PSCReloadMode_Immediate);
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);   // Без этого сразу входит в обработчик, даже без наступления нужного события

			TIM3->CNT = TM3_INIT_VALUE;
			//TIM_Cmd(TIM3, ENABLE);*/
			
			//////----------FOR TIM 2 STM32F103------------------------------------
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
			  
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
			NVIC_InitStructure.NVIC_IRQChannel 									 	= TIM2_IRQn;	  
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 0;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority 				= 0;	
			NVIC_InitStructure.NVIC_IRQChannelCmd 								= ENABLE;
			NVIC_Init(&NVIC_InitStructure);
	
			TIM_DeInit(TIM2);
			
			SystemCoreClockUpdate();
			RCC_ClocksTypeDef RCC_Clocks;		
			RCC_GetClocksFreq(&RCC_Clocks);		
			
			//TODO: need time corrections!
			TIM_TimeBaseStructure.TIM_Period 					= (RCC_Clocks.HCLK_Frequency / 10000); //1ms			 																						
			TIM_TimeBaseStructure.TIM_Prescaler 			= 10;				   
			TIM_TimeBaseStructure.TIM_ClockDivision		=	TIM_CKD_DIV1; 		
			TIM_TimeBaseStructure.TIM_CounterMode			=	TIM_CounterMode_Up; 	
			TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
			TIM_ClearFlag(TIM2, TIM_FLAG_Update);							   
			TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
			//TIM_Cmd(TIM2, ENABLE);	
}

void RunRTOS (void)
{
		ATOMIC_BLOCK_FORCEON
		{
			//TIM_Cmd(TIM3, ENABLE);
			TIM_Cmd(TIM2, ENABLE);	
		}
}


		#warning поэкспериментировать с переменным RtosTimerDivider в StopRTOS и в зависимости от задач(возможно их приоритета)
	//RTOS увеличение предделителя системного таймера
	void SuspendRTOS (void)//Фактически снижение частоты системного таймера
	{
		ATOMIC_BLOCK_FORCEON
		{
			#warning temporary
			FullStopRTOS();
		}
	}


	//RTOS Остановка системного таймера
	void FullStopRTOS (void)
	{
		ATOMIC_BLOCK_FORCEON
		{
			//TIM_Cmd(TIM3, DISABLE);
			TIM_Cmd(TIM2, DISABLE);	
		}
	}


	void DeadTimerInit (void)
	{
		#warning FOR wdt
	}


	void ltoa(long int n,char *str)
	{
	unsigned long i;
	unsigned char j,p;
	i=1000000000L;
	p=0;
	if (n<0)
	{
	n=-n;
	*str++='-';
	};
	do
	{
	j=(unsigned char) (n/i);
	if (j || p || (i==1))
	{
	*str++=j+'0';
	p=1;
	}
	n%=i;
	i/=10L;
	}
	while (i!=0);
	*str=0;
	}
	//************************************************** *****

	void reverse(char s[])
	{
	int i, j;
	char c;
	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
	c = s[i];
	s[i] = s[j];
	s[j] = c;
	}
	}

	void itoa(int n, char s[])
	{
	int i, sign;
	if ((sign = n) < 0) //записываем знак
	n = -n; // делаем n положительным числом
	i = 0;
	do { //генерируем цифры в обратном порядке
	s[i++] = n % 10 + '0'; //берем следующую цифру
	} while ((n /= 10) > 0); // удаляем
	if (sign < 0)
	s[i++] = '-';
	s[i] = '\0';
	reverse(s);
	}

	#warning протестить!!
	void my_ltoa(signed long long int data, unsigned char *pRes);
	void my_ltoa(signed long long int data, unsigned char *pRes)
	{
		unsigned char buff[22];
		unsigned long long int int_path, fract_path, val;
		unsigned int sign_flag=0;
		if(data<0)
		{
			int_path = (unsigned long long int) (-data);
			sign_flag = 1;
		}
		else
		{
			int_path = (unsigned long long int) data;
		}

		//max длина long long int 19 символов - 9223372036854775807
		for(unsigned int i=0; i<19; i++)
		{
			val=int_path;
			int_path = val/10;
			fract_path = val%10;

			buff[i] = fract_path+'0';
			buff[i+1] = '\0';

			if(int_path == 0) break;
		}

		//копируем результат наоборот
		val = strlen((const char*) buff);

		//проверяем знак флага
		if(sign_flag)
		{
			buff[val] = '-';
			buff[val+1] = '\0';
			val++;
		}

		for(unsigned int i=0, j=val-1; i<val; i++, j--)
		{
			pRes[j] = buff[i];
		}

		pRes[val] = '\0';
	}
	//---------------------------------------------------

#endif
