// sensors.h
#include "macros.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_i2c.h"
#include <stdio.h>
#include "DS1822.h"
#include "serial.h"
#include "systick.h"
#include "GLCD.h"
#include "strings.h"
#include "dht11.h"
#include "mesh.h"
#include "pinmap.h"

#include "EERTOS.h"

//Termopare https://hubstub.ru/stm32/141-izmerenie-temperatury-s-pomoschyu-termopary-na-primere-max6675-dlya-stm32.html
//Gas http://catethysis.ru/stm32-mq135/
//Preasure http://ziblog.ru/2013/03/15/bmp085-datchik-davleniya.html

//#define One_Wire_Pin 		GPIOA, GPIO_Pin_15
#define One_Wire_Pin 		GPIOB, GPIO_Pin_6


//////////
//#define	CS_LOW 	GPIOB->BSRR = GPIO_BSRR_BR1;
//#define  CS_HIGH 	GPIOB->BSRR = GPIO_BSRR_BS1;

//Mapple mini
#define DUST_PIN_LED_GND  B, 5, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ	
#define DUST_PIN_LED_PWM  B, 4, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ	
#define DUST_PIN_ANALOG   A, 0, HIGH, MODE_INPUT_FLOATING, SPEED_2MHZ

extern SensorFile_t sFile;

extern float DS18b20_temp;
extern float TCoupleData;	
extern int dh_T;
extern int dh_H;
extern uint16_t co2;
extern float internalTemp;
extern float battVolt;
extern uint16_t dustLvl;

extern uint8_t ROM_SN[One_Wire_Device_Number_MAX][DS1822_SERIAL_NUM_SIZE];
extern uint8_t devices_cnt;
extern float DS_Arr[128];

uint16_t getCO2Level();
unsigned char One_Wire_Error_Handle (unsigned char err);

DECLARE_TASK(VibroSensor_Hndl);
DECLARE_TASK(GasSensor_Hndl);
DECLARE_TASK(GetInternalsParams);
DECLARE_TASK(Humidity_Hndl);
DECLARE_TASK(TermoCoupe_Hndl);
DECLARE_TASK(Ds18b20_Hndl);
DECLARE_TASK(Ds18b20_ReguestTemp);
DECLARE_TASK(Ds18b20_Search);

DECLARE_TASK(DustSensor_Hndl);
DECLARE_TASK(InfoOut_T);