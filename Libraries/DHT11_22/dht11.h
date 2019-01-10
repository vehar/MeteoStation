/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DHT11_H
#define __DHT11_H

/* Includes ------------------------------------------------------------------*/
//#include "utils.h"
#include "stdint.h"
/* Exported constants --------------------------------------------------------*/
#define MAX_TICS 30000
#define DHT_OK 0
#define DHT_NO_CONN 1
#define DHT_CS_ERROR 2

enum {
	DHT_TEMP,
	DHT_HUM
};

enum {
DHT11,
DHT22 
};

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

uint8_t read_DHT(uint8_t *buf, int sensor);
int dhtGet(int num, int sensor);
void dht_process(int sensor);

#endif /* __DHT11_H */
