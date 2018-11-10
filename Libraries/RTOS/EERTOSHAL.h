#ifndef EERTOSHAL_H
#define EERTOSHAL_H

//#define AVR_CORE
#define ARM_CORE

#ifdef AVR_CORE

	//#include "..\AVR_platform.h" //Here - all platform specific includes
	#include <mega128.h> //TODO: move to platform.h
	#include <mem.h>
	#include <stdint.h>
	#include <stdlib.h>    // itoa
	#include <string.h>


	#define STATUS_REG 			    SREG
	#define Interrupt_Flag		    SREG_I
	#define _disable_interrupts()	#asm("cli")
	#define _enable_interrupts()	#asm("sei")
	#define wdt_reset()             #asm("wdr")

		#ifdef __CODEVISIONAVR__
			#define RTOS_ISR  		TIM2_COMP
			#define DEAD_TIME_ISR   TIM0_COMP
			#define F_CPU  _MCU_CLOCK_FREQUENCY_
		 #else
			#define RTOS_ISR  		TIMER2_COMP_vect
			#define DEAD_TIME_ISR   TIMER0_COMP_vect
			#define F_CPU  16000000L
		#endif

	//RTOS Config
	//System Timer Config
	#define Prescaler	  		    256
	#define	RtosTimerDivider  		(F_CPU/Prescaler/1000)// 1 mS
	//#define	RtosTimerDivider  	(F_CPU/Prescaler/10000)*/// 0.1mS!
	//Прерывание 10 000 раз в сек., но обработчик таймеров стартует каждое 10-е прерывание
	//получается 1мс

	#define	DeadTimerDivider  		(F_CPU/Prescaler/1000)
	#define HI(x) ((x)>>8)
	#define LO(x) ((x)& 0xFF)

	typedef unsigned char U_ALU_INT; //for better different cpu-types compiller compability
	typedef signed char   S_ALU_INT;

	//For debug only!!!!!!!!!!!!!!!!!!!!!!!!!!
	#define LED1 		6
	#define LED2		7
	#define	LED3		5

	#define LED_PORT 	PORTD
	#define LED_DDR		DDRD

	#define _LED_B_ON  LED_PORT|=(1<<LED1)
	#define _LED_B_OFF LED_PORT&=~(1<<LED1)

#endif//AVR_CORE

#ifdef ARM_CORE

#include "..\platform.h" //Here - all platform specific includes

	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>    // itoa
	#include <string.h>
	
	#warning 	inline__inline_for_KEIL

	#define	inline __inline //for KEIL
			
	void itoa(int n, char s[]);
	void reverse(char s[]);
	void ltoa(long int n,char *str) ;

	#define bit uint8_t
	typedef unsigned int U_ALU_INT; //for better different cpu-types compiller compability
	typedef signed int   S_ALU_INT;

	//extern uint8_t tmp_status_reg = 0;
	//extern uint8_t tmp_Interrupt_Flag = 0;

	#define STATUS_REG 	          	CPSR_
	#define Interrupt_Flag		    I
	#define _disable_interrupts()	__disable_irq();
	#define _enable_interrupts()	__enable_irq();
#endif //ARM_CORE

//------------------------------------------------------------------
//------------------------------------------------------------------
//---------------------ATOMIC_BLOCK---------------------------------
/*из такого atomic-блока нельзя принудительно выходить (при помощи goto/break/return), иначе прерывание не будет восстановлено.*/

//+++++++++++++PRIVATE RTOS INT VARS+++++++++++++++++++++++++++++++
extern uint32_t CPSR_;
extern bit	       global_nointerrupted_flag;
extern U_ALU_INT      global_interrupt_mask;
extern U_ALU_INT      global_interrupt_cond;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef AVR_CORE
	inline static int _irqDis(void)
	{
		_disable_interrupts();
		return 1;
	}

	inline static int _irqEn(void)
	{
		_enable_interrupts();
		return 0;
	}
	
	//PORT Defines

/*
#define LED1 		6
#define LED2		7
#define LED3		5
*/
#define LED_PORT 	0//PORTD
#define LED_DDR		0//DDRD

#endif //AVR_CORE

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#ifdef ARM_CORE

	
	inline static int _irqDis(void)
	{
		__disable_irq();
		// __ASM volatile ("cpsid i" : : : "memory");
		return 1;
	}

	inline static int _irqEn(void)
	{
		__enable_irq();
		//__ASM volatile ("cpsie i" : : : "memory");
		return 0;
	}

	/*
	__STATIC_INLINE uint32_t  __get_CPSR(void)
	{
	  register uint32_t __regCpsri        __ASM("cpsri");
	  return(__regCpsri );
	}

	__STATIC_INLINE void __set_CPSR(uint32_t cpsr)
	{
	  register uint32_t __regCpsri         __ASM("cpsri");
	  __regCpsri = (cpsr & 0xff);
	}
	*/

#define GET_TSC *(uint32_t*)0xE0001004
//uint32_t *DWT_CYCCNT  = ((uint32_t*)0xE0001004)

// ПРИМ - DWT, это отрадочный счетчик STM32.
//        В отличие от TIM, этот счетчик работает на полной частоте, а не на половинной.
//        ВАЖНО! - DWT необязательный компонент камня. Т.е., этот счетчик может быть не у всех производителей Кортексов.
//                 При портировании этого кода на Кортекс от другого производителя, обратить на это внимание.

//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/BABJFFGJ.html


//#define _LED_B_ON  STM_EVAL_LEDOn(LED3); //LED_PORT|=(1<<LED1)
//#define _LED_B_OFF STM_EVAL_LEDOff(LED3); //LED_PORT&=~(1<<LED1)

#endif //ARM_CORE

#define ATOMIC_BLOCK_FORCEON \
    for(global_nointerrupted_flag = _irqDis();\
         global_nointerrupted_flag;\
         global_nointerrupted_flag = _irqEn())
//------------------------------------------------------------------
//------------------------------------------------------------------
/* Example
main()
{
    ATOMIC_BLOCK_FORCEON
    {
        do_something();
    }
}*/

inline static U_ALU_INT _iDisGetPrimask(void)       //if (STATUS_REG & (1<<Interrupt_Flag)){_disable_interrupts(); nointerrupted = 1;}
{
    U_ALU_INT result;
    #warning пока без считывания!!!!
    result = CPSR_;
    return result;
} 

inline static U_ALU_INT _iSetPrimask(U_ALU_INT priMask)
{
#warning пока без считывания!!!!
    CPSR_ = priMask;
    return 0;
}

#define ATOMIC_BLOCK_RESTORESTATE \
     for( global_interrupt_mask = _iDisGetPrimask(), global_nointerrupted_flag = 1;\
         global_nointerrupted_flag;\
         global_nointerrupted_flag = _iSetPrimask(global_interrupt_mask))
//------------------------------------------------------------------
//------------------------------------------------------------------


inline static U_ALU_INT _irqCondDis(U_ALU_INT flag)
{
    if (flag){_disable_interrupts();}
    return 1;
}

inline static U_ALU_INT _irqCondEn(U_ALU_INT flag)
{
    if (flag){_enable_interrupts();}
    return 0;
}

#define ATOMIC_BLOCK_FORCEON_COND(condition) \
    for(global_interrupt_cond = condition, global_nointerrupted_flag = _irqCondDis(global_interrupt_cond);\
        global_nointerrupted_flag;\
        global_nointerrupted_flag = _irqCondEn(global_interrupt_cond))
//------------------------------------------------------------------
//------------------------------------------------------------------

inline static U_ALU_INT _irqCondDisGetPrimask(U_ALU_INT flag)
{
    U_ALU_INT result;
    if (flag)
    {
    result = CPSR_;
    _disable_interrupts();
    }
    return result;
}


#define ATOMIC_BLOCK_RESTORESTATE_COND(condition) \
        for(global_interrupt_cond = condition, global_interrupt_mask = _irqCondDisGetPrimask(global_interrupt_cond), global_nointerrupted_flag = 1;\
        global_nointerrupted_flag;\
        global_nointerrupted_flag = global_interrupt_cond ? _iSetPrimask(global_interrupt_mask):0)

//---------------------ATOMIC_BLOCK---------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------



extern void RTOS_timer_init (void);

extern void RunRTOS (void);      	//запуск системмного таймера
extern void SuspendRTOS (void);     //замедление системмного таймера
extern void FullStopRTOS (void); 	//полная остановка системмного таймера

extern void DeadTimerInit (void);
extern void DeadTimerRun (void);
extern void DeadTimerStop (void);

#endif
