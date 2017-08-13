/*
 * File: mcu_i2c.c
 * Date: 12.03.2013
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */

#include "main.h"

//------------------------------------------------------------------------------
void mcu_i2c_init(void)
{
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
	I2C_InitStructure.I2C_OwnAddress1 = 0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 100000;
	I2C_Init(I2C1, &I2C_InitStructure);

	I2C_Cmd(I2C1, ENABLE);
}

//------------------------------------------------------------------------------
void mcu_i2c_write_byte(uint8_t device_address, uint8_t value)
{
	// формируем сигнал СТАРТ
	I2C1->CR1 |= I2C_CR1_START;

	// ждем окончания передачи сигнала СТАРТ
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	(void) I2C1->SR1;

	// передаем адрес ведомого устройства
	I2C1->DR = (uint8_t) (device_address << 1);

	// ждем окончания передачи адреса
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}
	(void) I2C1->SR1;
	(void) I2C1->SR2;

	// передаем значение
	I2C1->DR = value;

	// ожидаем окончания передачи
	while (!(I2C1->SR1 & I2C_SR1_BTF))
	{
	}
	(void) I2C1->SR1;

	// формируем сигнал СТОП
	I2C1->CR1 |= I2C_CR1_STOP;
}

//------------------------------------------------------------------------------
void mcu_i2c_write_two_byte(uint8_t device_address, uint8_t value_a, uint8_t value_b)
{
	// формируем сигнал СТАРТ
	I2C1->CR1 |= I2C_CR1_START;

	// ждем окончания передачи сигнала СТАРТ
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	(void) I2C1->SR1;

	// передаем адрес ведомого устройства
	I2C1->DR = (uint8_t) (device_address << 1);

	// ждем окончания передачи адреса
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}
	(void) I2C1->SR1;
	(void) I2C1->SR2;

	// передаем значение
	I2C1->DR = value_a;

	// ожидаем окончания передачи
	while (!(I2C1->SR1 & I2C_SR1_BTF))
	{
	}
	(void) I2C1->SR1;

	// передаем значение
	I2C1->DR = value_b;

	// ожидаем окончания передачи
	while (!(I2C1->SR1 & I2C_SR1_BTF))
	{
	}
	(void) I2C1->SR1;

	// формируем сигнал СТОП
	I2C1->CR1 |= I2C_CR1_STOP;
}

//------------------------------------------------------------------------------
uint8_t mcu_i2c_read_byte(uint8_t device_address)
{
	// формируем сигнал СТАРТ
	I2C1->CR1 |= I2C_CR1_START;

	// ждем окончания передачи сигнала СТАРТ
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	(void) I2C1->SR1;

	// передаем адрес ведомого устройства
	I2C1->DR = (uint8_t) (device_address << 1) | 0x01;

	// ждем окончания передачи адреса
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}

	I2C1->CR1 &= ~I2C_CR1_ACK;

	(void) I2C1->SR1;
	(void) I2C1->SR2;

	// формируем сигнал СТОП
	I2C1->CR1 |= I2C_CR1_STOP;

	// ждем окончания приема данных
	while (!(I2C1->SR1 & I2C_SR1_RXNE))
	{
	}

	// считываем данные
	return I2C1->DR;
}

