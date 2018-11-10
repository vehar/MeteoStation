/* STM32 includes */
#include "stm32f10x.h"
#ifdef USE_STDPERIPH_DRIVER
#include <stm32f10x_conf.h>
#endif

#include "lcd1202.h"

#include "main.h"

#include "init.c"
#include "dht11.h"
#include "ds18b20.h"
#include <stdio.h>

int timer_sec = 0;
int display_update = 0;

void TIM6_DAC_IRQHandler (void) //100mSec
{
	static int delay_sec = 10, delay_5sec = 5;
	static int step = 0;
	
	TIM6->SR &= ~TIM_SR_UIF; //Clear status register update
		
	/*if(--delay_sec == 0){
		delay_sec = 10; //1 Sec
		timer_sec = 1;
		
		if(--delay_5sec == 0) { //5 Sec delay
			delay_5sec = 5;
			dht_process();
			ds18b20Process();
			display_update = 1;
		}
		
					
 		if(step) {
 			step = 0;
 			LED_BLUE_ON;
 		} else {
 			step = 1;
 			LED_BLUE_OFF;
 		}
	}*/

}

/**
 * Main function
 */
int v;
int temp = 0;
int hum = 0;
//Humidity sensor ВРЕ11 http://ffix.ru/index.php/stm32-stm8/21-http-ffixru-component-k2-itemlist-user-63-dmitrijhtml 
//PA-6 Data pin

uint16_t Arr[50];

int main(void)
{
		int cnt = 0; 
	  
	  char buf[17];
    sysInit();
    //lcd_init();

     //lcd_str(0, 26, "FFIX.RU");
	//	 lcd_str_center(2, "wait");
	
	  //ds18b20Init();

    while(1)
    {
		delay_ms(2000);
				/*if(BUTTON) {
					cnt++; lcd_big_char(5, 30, cnt+'0');
					while(BUTTON);
				}*/

				//if(display_update) 
				//	{

					dht_process();
					temp = dhtGet(DHT_TEMP, DHT11); //if(v == 0xFFFFFC00) lcd_str(1, 0, "DHT11 T: ---"); else { sprintf(buf, "DHT11 T: %iC", v);  lcd_str(1, 0, buf); }
					hum = dhtGet(DHT_HUM,  DHT11); //if(v == 0xFFFFFC00) lcd_str(2, 0, "DHT11 H: ---"); else { sprintf(buf, "DHT11 H: %i%%", v); lcd_str(2, 0, buf); }
						
					//v = dhtGet(DHT_TEMP, DHT22); //if(v == 0xFFFFFC00) lcd_str(4, 0, "DHT22 T: ---"); else { sprintf(buf, "DHT22 T: %iC", v);  lcd_str(4, 0, buf); }
					//v = dhtGet(DHT_HUM,  DHT22); //if(v == 0xFFFFFC00) lcd_str(5, 0, "DHT22 H: ---"); else { sprintf(buf, "DHT22 H: %i%%", v); lcd_str(5, 0, buf); }	
						
					//sprintf(buf, "DS18B20 T: %iC", ds18b20GetTemperature()); lcd_str(7, 0, buf);							
				//}
		
    }
}

