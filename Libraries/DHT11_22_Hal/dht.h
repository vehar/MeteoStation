#ifndef __DHT_H
#define __DHT_H

#ifdef __cplusplus
 extern "C" {
#endif 

	 /*	* Модуль для работы с датчиками DHT11 и DHT22
		*	Ver. 1.4
		* Таймер необходимо настраивать в основной программе в режиме Input Capture direct mode с включенным DMA. 
		*	Выход соответствующего канала как Alternate Function Open Drain, можно с подтяжкой вверх
		* Делитель таймера ставим таким образом, чтобы отчет шел за 1 микросекунду,
		* т.е. вот так: htimX.Init.Prescaler = (HAL_RCC_GetPCLK2Freq()/1000000)-1; (Для TIM3 на STM32F4 он будет равен 42-1, т.е. 41)
		*	Polarity Selection назначаем Falling Edge (по спадающему фронту)
		* DMA настраиваем Data Width Half Word
		*	
		* Copyright (c) 2016, Bogomazyuk Vasiliy (Богомазюк Василий)
	*/

#include <stdint.h>	 
#include "stm32f10x_gpio.h"	 
#include "stm32f10x_tim.h"		 
	 
#ifdef STM32F103xB
	//#include "stm32f1xx_hal.h"
	 #include "stm32f10x.h"
#endif
#ifdef STM32F030x6
	//#include "stm32f0xx_hal.h"
#endif
#if defined(STM32F411xE) || defined(STM32F407xx)
	//#include "stm32f4xx_hal.h"
#endif
 

#define DELAY(__MS__)	HAL_Delay((__MS__)) //Задержка на время чтения датчика. Если будет RTOS - можно заменить на его задержку
	 
#define Delta				10 		//Погрешность снятия данных
#define DHT_Detect	168		//Период для сигнала, показывающего, что датчик готов
#define DHT_0				100		//Порог "нуля". У (по крайней мере у меня) DHT11 и DHT22 разные временные характеристики
#define LenDHT			42		//1 инициализация + 1 начальный + 40 битов данных

typedef enum	//Тип датчика
{
  DHT11			=	11,
  DHT22			=	22,
	DHT_AUTO	=	0
}TypeDHT;
	 
typedef struct HAL_DHT //Структура хранит параметры, необходимые для работы с датчиком
{
	TypeDHT	Type;											//Тип датчика
	TypeDHT	TypeAUTO;									//Тип датчика, полученный автоопределением, для этого Type указываем равным DHT_AUTO
	GPIO_TypeDef *GPIOx;							//Порт GPIO, на котором "висит" датчик
	uint16_t GPIO_Pin;								//Ножка микроконтроллера, на котором "висит" датчик
	GPIO_InitTypeDef GPIO_InitStruct;	//Заполненная структура GPIO порта, на котором находится датчик. Ее запоминаем, чтобы каждый раз не заполнять
	//TIM_HandleTypeDef *htim;					//Заполненая структура таймера, настроенного в режиме Input Capture direct mode. Таймер (с DMA) нужен для снятия данных с датчика
	uint32_t Channel;									//Канал таймера на котором находится датчик. Канал соответствует порту GPIO, описанному выше. И ножка канала и GPIO-порт настроены в режиме Open Drain с подтяжкой вверх
}HAL_DHT;

	void DHT_Init(HAL_DHT* DHTx, TypeDHT Type, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,/*TIM_HandleTypeDef *htim,*/ uint32_t Channel); //Настройка структуры, описывающей датчик

	//Снимаем данные с датчика DHTx и помещаем в соответствующие переменные.
	//Функция возвращает коды ошибок:
	//0	-	Ошибок нет
	//1	-	Датчик не обнаружен
	//2	-	Ошибка чтения датчика
	//3	- При DHT_AUTO не удалось определить тип датчика
	uint8_t DHT_Read(HAL_DHT* DHTx, uint16_t* Humidity, int16_t* Temperature);
	 
	 
	 
#ifdef __cplusplus
}
#endif

#endif /* __DHT_H */
