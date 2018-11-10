/*
 * File: bmp085.c
 * Date: 12.03.2013
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */

#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include "bmp085.h"

enum bmp085_settings
{
	BMP085_SETTINGS_DEVICE_ADDRESS = 0x77, // адрес
	BMP085_SETTINGS_OSS = 0, //
	BMP085_SETTINGS
};

union bmp085_calibration_coefficients_type
{
	uint16_t raw[11];
	struct 
	{
		int16_t ac1;
		int16_t ac2;
		int16_t ac3;
		uint16_t ac4;
		uint16_t ac5;
		uint16_t ac6;
		int16_t b1;
		int16_t b2;
		int16_t mb;
		int16_t mc;
		int16_t md;
		int32_t b5;
	} st;
};

struct bmp085_type
{
	// калибровочные коэффициенты
	union bmp085_calibration_coefficients_type calibration_coefficients;

	// нескомпенсированная температура
	int16_t uncompensated_temperature;

	// нескомпенсированное  давление
	int32_t uncompensated_pressure;
};

struct bmp085_type bmp085;

//------------------------------------------------------------------------------
void bmp085_init(void)
{
	uint8_t index;

	// активируем датчик
//	PIN_OFF(PIN_BMP085_XCLR);

	// пауза перед обменом
	delay_ms(5);

	// устанавливаем указатель на начальный адрес корректировочных ячеек
	mcu_i2c_write_byte(BMP085_SETTINGS_DEVICE_ADDRESS, 0xAA);

	// считываем значение корректировочных ячеек
	for (index = 0; index < ARRAY_LENGHT(bmp085.calibration_coefficients.raw); index++)
	{
		bmp085.calibration_coefficients.raw[index] = ((uint16_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS)) << 8;
		bmp085.calibration_coefficients.raw[index] |= (uint16_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS);
	}
}

//------------------------------------------------------------------------------
int32_t bmp085_read_pressure(void)
{
	volatile int32_t x1;
	volatile int32_t x2;
	volatile int32_t x3;
	volatile int32_t b6;
	volatile int32_t b3;
	volatile uint32_t b4;
	volatile uint32_t b7;
	volatile int32_t p;

	// запуск преобразования
	mcu_i2c_write_two_byte(BMP085_SETTINGS_DEVICE_ADDRESS, 0xF4, 0x34);

	// ждем окончания преобразования
//	while (PIN_SIGNAL(PIN_BMP085_EOC))
	{
	}

	// чтение результата преобразования
	mcu_i2c_write_byte(BMP085_SETTINGS_DEVICE_ADDRESS, 0xF6);

	bmp085.uncompensated_pressure = ((int32_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS)) << 16;
	bmp085.uncompensated_pressure |= ((int32_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS)) << 8;
	bmp085.uncompensated_pressure |= (int32_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS);
	bmp085.uncompensated_pressure >>= (8 - BMP085_SETTINGS_OSS);

	// корректировка
	b6 = bmp085.calibration_coefficients.b5 - 4000;

	x1 = b6 * b6;
	x1 >>= 12;
	x1 *= bmp085.calibration_coefficients.b2;
	x1 >>= 11;

	x2 = b6 * bmp085.calibration_coefficients.ac2;
	x2 >>= 11;

	x3 = x1 + x2;

	b3 = (bmp085.calibration_coefficients.ac1 * 4) + x3;
	b3 <<= BMP085_SETTINGS_OSS;
	b3 += 2;
	b3 /= 4;

	x1 = bmp085.calibration_coefficients.ac3 * b6;
	x1 >>= 13;

	x2 = b6 * b6;
	x2 >>= 12;
	x2 *= bmp085.calibration_coefficients.b1;
	x2 >>= 16;

	x3 = x1 + x2;
	x3 += 2;
	x3 >>= 2;

	b4 = (uint32_t) (x3 + 32768) * bmp085.calibration_coefficients.ac4;
	b4 >>= 15;

	b7 = (uint32_t) (bmp085.uncompensated_pressure - b3) * (50000 >> BMP085_SETTINGS_OSS);

	if (b7 < 0x80000000)
	{
		p = b7 * 2 / b4;
	}
	else
	{
		p = b7 * b4 / 2;
	}

	x1 = (p >> 8);
	x1 *= x1 * 3038;
	x1 >>= 16;

	x2 = -7357 * p;
	x2 >>= 16;

	return ((p + (x1 + x2 + 3791) / 16) * 3) / 4;
}

//------------------------------------------------------------------------------
int16_t bmp085_read_temperature(void)
{
	int32_t x1;
	int32_t x2;

	// запуск преобразования
	mcu_i2c_write_two_byte(BMP085_SETTINGS_DEVICE_ADDRESS, 0xF4, 0x2E);

	// ждем окончания преобразования
//	while (PIN_SIGNAL(PIN_BMP085_EOC))
	{
	}

	// чтение результата преобразования
	mcu_i2c_write_byte(BMP085_SETTINGS_DEVICE_ADDRESS, 0xF6);

	bmp085.uncompensated_temperature = ((int16_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS)) << 8;
	bmp085.uncompensated_temperature |= (int16_t) mcu_i2c_read_byte(BMP085_SETTINGS_DEVICE_ADDRESS);

	// корректировка
	x1 = bmp085.uncompensated_temperature - bmp085.calibration_coefficients.ac6;
	x1 *= bmp085.calibration_coefficients.ac5;
	x1 >>= 15;

	x2 = bmp085.calibration_coefficients.mc;
	x2 <<= 11;
	x2 /= (x1 + bmp085.calibration_coefficients.md);

	bmp085.calibration_coefficients.b5 = x1 + x2;

	return (int16_t) ((bmp085.calibration_coefficients.b5 + 8) >> 4);
}
