/*
 * File: bmp085.h
 * Date: 12.03.2013
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */

#ifndef BMP085_H_
#define BMP085_H_

void bmp085_init(void);
int16_t bmp085_read_temperature(void);
int32_t bmp085_read_pressure(void);

#define PIN_BMP085_XCLR		B, 14,  LOW, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ
#define PIN_BMP085_EOC		B, 15,  LOW, MODE_INPUT_FLOATING, SPEED_2MHZ
#endif /* BMP085_H_ */
