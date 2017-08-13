#include "dht.h"
#include <stdint.h>
uint8_t DHT_Bit(uint8_t tic) //Функция перевода массива в биты. Проверку некорректных данных проводить не будем
{
	if (tic>DHT_0) return 1; else return 0;
}


uint8_t DHT_Read(HAL_DHT* DHTx, uint16_t* Humidity, int16_t* Temperature)
{
	uint16_t pData[LenDHT];																											//Резервируе область памяти для данных, захватываемых таймером
	uint8_t dht_data[5];																												//"Распознанные" данные. Датчики выдают 5 байт: 2 на влажность, 2 на температуру, 1 сумма предыдущих байт
	uint8_t pos = 0;
/*	
	HAL_TIM_Base_DeInit(DHTx->htim);																						//Деинициализируем таймер, т.к. будем работать с портом как с портом GPIO
	HAL_GPIO_Init(DHTx->GPIOx, &DHTx->GPIO_InitStruct);													//Инициализируем ножку микрокантроллера как GPIO
	HAL_GPIO_WritePin(DHTx->GPIOx,DHTx->GPIO_Pin,GPIO_PIN_RESET);								//Поддтягиваем к земле
	DELAY(18);																																	//Ждем 18 мс
	HAL_GPIO_WritePin(DHTx->GPIOx,DHTx->GPIO_Pin,GPIO_PIN_SET);									//Отпускаем ножку
	HAL_GPIO_DeInit(DHTx->GPIOx, DHTx->GPIO_Pin);																//Деиниализируем GPIO
	HAL_TIM_Base_Init(DHTx->htim);																							//Включаем на "нашей" ножке Alternate Function (таймер)
	HAL_TIM_IC_Start_DMA(DHTx->htim,DHTx->Channel,(uint32_t*)pData,LenDHT);			//Читаем с помощью DMA данные с датчика и помещаем их в pData
	DELAY(7);																																		//За 7 мс данные должны гарантированно считаться
	HAL_TIM_IC_Stop_DMA(DHTx->htim,DHTx->Channel);															//Останавливаем чтение данных
*/	
	for (int i=0; i<LenDHT-1; i++)
	{
		pData[i]=pData[i+1]-pData[i];																							//Преобразуем абсолютные значения, полученные через таймер, в задержки между приходами бит
	}
	#if defined(STM32F411xE) || defined(STM32F407xx) //Костыль на медленных контроллерах бит проверки наличия датчика читается нестабильно
	if (((pData[0]+Delta)<DHT_Detect)||((pData[0]-Delta)>DHT_Detect)) return 1;	//Если первый бит (отвечает за проверку наличия датчика) сильно отклоняется от 160 мс, то выходим с ошибкой 1
#endif
	//Из "сырых" данных получаем байты, передаваемые датчиком
	if (pData[0]>150) {pos=1;}//Костыль на медленных контроллерах бит проверки наличия датчика читается нестабильно, если все же прочелся - игнорируем его

	for (int i=0;i<5;i++)
	{
		for (int j=0;j<8;j++)
		{
			dht_data[i]<<=1;
			dht_data[i]|=DHT_Bit(pData[pos++]);
		}
	}

	if ((uint8_t)((dht_data[0]+dht_data[1]+dht_data[2]+dht_data[3]))!=dht_data[4]) return 2; //Если данные не проходят проверку, выходим с ошибкой 2
	if (DHTx->Type==DHT_AUTO)																											//Пытаемся определить тип датчика
	{
		if (dht_data[0]>2) DHTx->TypeAUTO=DHT11;
		if (dht_data[0]<=2) 
		{
			if (dht_data[2]&0x80) 
			{
				DHTx->TypeAUTO=DHT22;
			}else 
			{
				if ((dht_data[1]==0)&&(dht_data[3]==0)) 
				{
					if (dht_data[2]>2)
					{
						DHTx->TypeAUTO=DHT11;
					}else 
					{
						if (dht_data[0]==2) 
						{
							return 3;																																//Не удалось определить датчик
						}else DHTx->TypeAUTO=DHT22;
					}
				}else DHTx->TypeAUTO=DHT22;
			}
		}else DHTx->TypeAUTO=DHT11;
			
	};
	if (DHTx->TypeAUTO==DHT22)																												//Кодирование данных у датчиков DHT11 и DHT22 осуществляется по разному
	{
		*Humidity=(dht_data[0]<<8)+dht_data[1];
		*Temperature=(dht_data[2]<<8)|dht_data[3];
		if (dht_data[2]&0x80) *Temperature^=0x7FFF; 															//Корректируем отрицательную температуру
	}else
	{
		*Humidity=dht_data[0]*10+dht_data[1];
		*Temperature=dht_data[2]*10+dht_data[3];
	}
	return 0;	
}

void DHT_Init(HAL_DHT* DHTx, TypeDHT Type, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,/*TIM_HandleTypeDef *htim,*/ uint32_t Channel)
{
	DHTx->Type=Type;
	DHTx->TypeAUTO=Type;
	//Заполняем структуру DHTx, данными необходимыми для работы с ножкой, к которой подключен датчик, как с GPIO-портом
	DHTx->GPIOx=GPIOx;
	DHTx->GPIO_Pin=GPIO_Pin;
//	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
//  DHTx->GPIO_InitStruct.Pin = GPIO_Pin;
//  DHTx->GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//  DHTx->GPIO_InitStruct.Pull = GPIO_PULLUP;
//  DHTx->GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	//Запоминаем указатель на структуру настроек таймера и номер канала, на котором "висит" датчик. Сам таймер настраивается в теле основной программы
//	DHTx->htim=htim;
	DHTx->Channel=Channel;
}


