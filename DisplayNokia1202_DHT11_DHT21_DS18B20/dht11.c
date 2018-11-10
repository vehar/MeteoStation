#include "dht11.h"
#include "stm32f10x.h"
#include "main.h"
#include "delay.h"

unsigned char dht_buf[2][5];

int dht_status[2] = {DHT_NO_CONN, DHT_NO_CONN};

void dht_process(void) { //Раз в 10 сек
	dht_status[DHT11] = read_DHT(&dht_buf[DHT11][0], DHT11);
	dht_status[DHT22] = read_DHT(&dht_buf[DHT22][0], DHT22);
}

int dhtGet(int num, int sensor) {
	
	if(dht_status[sensor] != DHT_OK) return 0xFFFFFC00;
	
	switch(num) {
		case DHT_HUM:
			return dht_buf[sensor][0];
		
		case DHT_TEMP: {
			if(dht_buf[sensor][2] & 0x80) return (int)dht_buf[sensor][2] + 0xFFFFFF00;
			return dht_buf[sensor][2];
		}
	}
	
	return -1;
}

uint16_t read_cycle(uint16_t cur_tics, uint8_t neg_tic, int sensor){
	uint16_t cnt_tics;
	
 	if (cur_tics < MAX_TICS) cnt_tics = 0;	
	if (neg_tic){
		while ((sensor == DHT11 ? !DHT11_IN : !DHT22_IN) && (cnt_tics < MAX_TICS)){
			cnt_tics++;
		}
	}
	else {
		while ((sensor == DHT11 ? DHT11_IN : DHT22_IN) && (cnt_tics < MAX_TICS)){
			cnt_tics++;
		}
	}
 	return cnt_tics;
}

extern uint16_t Arr[50];

#define BUFF_DT_LEN (42)

	uint16_t dt[BUFF_DT_LEN];

uint8_t read_DHT(uint8_t *buf, int sensor){

	uint16_t cnt;
	uint8_t i, check_sum; 
	
	if(sensor == DHT11) {
			DHT11_SET_OUT;
			DHT11_LOW;
			delay_ms(20);
			DHT11_HI;
			DHT11_SET_IN;
	} else  {
			DHT22_SET_OUT;
			DHT22_LOW;
			delay_ms(20);
			DHT22_HI;
			DHT22_SET_IN;		
	}
	
  //start reading	
 	cnt = 0; 
	for(i = 0; i < 83 && cnt < MAX_TICS; i++){
		if (i & 1){
			cnt = read_cycle(cnt, 1, sensor);
		} else {
			cnt = read_cycle(cnt, 0, sensor);
			dt[i/2] = cnt;
		}
	}
	
	if (cnt >= MAX_TICS) return DHT_NO_CONN;
	
	memcpy(Arr, dt, 43);
	
	//convert data
 	for(i = 2; i < 42; i++){
		(*buf) <<= 1;
  	if (dt[i] > 50) {
			(*buf)++;
 		}
		if (!((i-1) % 8) && (i > 2)) {
			buf++;
		}
 	}
	
	//calculate checksum
	buf -= 5;
	check_sum = 0;
 	for(i = 0; i < 4; i++){
		check_sum += *buf;
		buf++;
	}
	
	if (*buf != check_sum) return DHT_CS_ERROR;
	buf -= 4;			
	if(sensor == DHT22) {
		   float f;
			 int v = buf[0];
       f = v * 256 + buf[1];
			 f /= 10;
		   v = f; 
			 buf[0] = v;
		   v = buf[2];
       f = (v & 0x7F) * 256 + buf[3];
			 f /= 10;
       if (buf[2] & 0x80)  f *= -1;
		   v = f;
			 buf[2] = v & 0xFF;
	}
	
	return DHT_OK;	
}

