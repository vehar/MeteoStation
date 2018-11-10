#include "DS1822.h"
#include "gpio.h"
#include "1-wire.h"
#include "CRC.h"

unsigned char DS1822_SERIAL_NUMBER[DS1822_SERIAL_NUM_SIZE];

unsigned int DS1822_Start_Conversion_Skip_Rom (unsigned long PIN)			 
{	 																		
	if (One_Wire_Reset(PIN)==One_Wire_Success)												
	{																		   
		One_Wire_Write_Byte(One_Wire_Skip_ROM,PIN);
		One_Wire_Write_Byte(DS1822_CONVERT_T_CMD,PIN);
		
		if (One_Wire_Read_Byte(PIN)==0)	return One_Wire_Success;
		else	return One_Wire_Device_Busy;

	}
	else	return One_Wire_Error_No_Echo;
}

unsigned int DS1822_Read_Temp_NoCRC_Skip_Rom (unsigned long PIN)				 
{
	if (One_Wire_Reset(PIN)==One_Wire_Success)
	{
		One_Wire_Write_Byte(One_Wire_Skip_ROM,PIN);
		One_Wire_Write_Byte(DS1822_READ_STRATCHPAD_CMD,PIN);
		return One_Wire_Read_Byte(PIN)|(One_Wire_Read_Byte(PIN)<<8);
	}
	else	return One_Wire_Error_No_Echo;
}

unsigned int DS1822_Read_Temp_CRC_Check (unsigned long PIN)		    		
{																   
	unsigned char inbuff[DS1822_STRATCHPAD_SIZE];
	unsigned char cnt=0;
	if (One_Wire_Reset(PIN)==One_Wire_Success)
	{
		One_Wire_Write_Byte(One_Wire_Skip_ROM,PIN);
		One_Wire_Write_Byte(DS1822_READ_STRATCHPAD_CMD,PIN);
		while (cnt!=DS1822_STRATCHPAD_SIZE)
		{
			inbuff[cnt]=One_Wire_Read_Byte(PIN);
			cnt++;
		}
		if (Crc8Dallas(DS1822_STRATCHPAD_SIZE,inbuff)==0)
		{
			return inbuff[0]|(inbuff[1]<<8);
		}
		else
		{
			return One_Wire_CRC_Error;
		}
	}
	else
	{
		return One_Wire_Error_No_Echo;
	}
}

unsigned int DS1822_Read_Rom(unsigned long PIN)					
{
	unsigned char cnt=0;
	if (One_Wire_Reset(PIN)==One_Wire_Success) 
	{
		One_Wire_Write_Byte(One_Wire_Read_ROM,PIN);
		while (cnt!=DS1822_SERIAL_NUM_SIZE)
		{
		 	DS1822_SERIAL_NUMBER[cnt]=One_Wire_Read_Byte(PIN);
		  	cnt++;
		}
		if (Crc8Dallas(DS1822_SERIAL_NUM_SIZE,DS1822_SERIAL_NUMBER)==0)
		{
			return One_Wire_Success;
		}
		else
		{
			return One_Wire_CRC_Error;
		}
		
	}
	return One_Wire_Error_No_Echo;
}

unsigned int DS1822_Send_Cmd_By_Rom (unsigned long PIN)   		
{
	unsigned char cnt=0;
	if (One_Wire_Reset(PIN)==One_Wire_Success)
	{ 
		One_Wire_Write_Byte(One_Wire_Match_ROM,PIN);
		while (cnt!=DS1822_SERIAL_NUM_SIZE)
		{
			One_Wire_Write_Byte(DS1822_SERIAL_NUMBER[cnt],PIN);
  	    	cnt++;
		}
		return One_Wire_Success;
	}
	else
	{
		return One_Wire_Error_No_Echo;
	}
}

unsigned int DS1822_Start_Conversion_Cmd (unsigned long PIN)
{
	if (DS1822_Send_Cmd_By_Rom(PIN)==One_Wire_Success)
	{
		One_Wire_Write_Byte(DS1822_CONVERT_T_CMD,PIN);
		return One_Wire_Success;
	}
	else
	{
		return One_Wire_Error_No_Echo;
	}
}

unsigned int DS1822_Read_Temp_NoCRC_By_Rom (unsigned long PIN)				 
{
	if (DS1822_Send_Cmd_By_Rom(PIN)==One_Wire_Success)
	{
		One_Wire_Write_Byte(DS1822_READ_STRATCHPAD_CMD,PIN);
		return One_Wire_Read_Byte(PIN)|(One_Wire_Read_Byte(PIN)<<8);
	}
	else
	{
		return One_Wire_Error_No_Echo;
	}
}

unsigned char DS1822_Search_Rom_One_Device (unsigned long PIN, unsigned char (*Serial_Num)[DS1822_SERIAL_NUM_SIZE])					
{																
	unsigned char cnt_bits;
	unsigned char cnt_bytes;
	unsigned char tmp;
	unsigned char tmp2=0;
	unsigned char dev_cnt=0;

	tmp=One_Wire_Reset(PIN);
	if (tmp!=One_Wire_Success) return tmp;
	One_Wire_Write_Byte(One_Wire_Search_ROM,PIN);
	
	for (cnt_bytes=0;cnt_bytes!=8;cnt_bytes++)
	{						
		for (cnt_bits=0;cnt_bits!=8;cnt_bits++)
		{
			tmp=One_Wire_Read_Bit(PIN);
	 		if (One_Wire_Read_Bit(PIN)==tmp) dev_cnt++;
			One_Wire_Write_Bit(tmp, PIN);
			if (tmp!=0) tmp2|=(1<<cnt_bits);		
		}
	(* Serial_Num)[cnt_bytes]=tmp2;
	tmp2=0;	
	}
	if (Crc8Dallas(DS1822_SERIAL_NUM_SIZE,(* Serial_Num))==0) return dev_cnt;
	else return One_Wire_CRC_Error; 
}

unsigned char DS1822_Start_Conversion_by_ROM (unsigned long PIN, unsigned char (*Serial_Num)[DS1822_SERIAL_NUM_SIZE])
{
	unsigned char cnt;
	cnt=One_Wire_Reset(PIN);
	if (cnt!=One_Wire_Success) return cnt;
	One_Wire_Write_Byte(One_Wire_Match_ROM,PIN);
	for (cnt=0;cnt!=8;cnt++) One_Wire_Write_Byte((*Serial_Num)[cnt],PIN);
	One_Wire_Write_Byte(DS1822_CONVERT_T_CMD,PIN);
	return One_Wire_Success;
}

unsigned char DS1822_Get_Conversion_Result_by_ROM_CRC (unsigned long PIN, unsigned char (*Serial_Num)[DS1822_SERIAL_NUM_SIZE], unsigned int * temp_code)
{
	unsigned char cnt;
	unsigned char inbuff[DS1822_STRATCHPAD_SIZE];
	cnt=One_Wire_Reset(PIN);
	if (cnt!=One_Wire_Success) return cnt;
	One_Wire_Write_Byte(One_Wire_Match_ROM,PIN);
	for (cnt=0;cnt!=8;cnt++) One_Wire_Write_Byte((*Serial_Num)[cnt],PIN);
	One_Wire_Write_Byte(DS1822_READ_STRATCHPAD_CMD,PIN);
	for (cnt=0;cnt!=DS1822_STRATCHPAD_SIZE;cnt++) inbuff[cnt]=One_Wire_Read_Byte(PIN);
	if (Crc8Dallas(DS1822_STRATCHPAD_SIZE,inbuff)==0) *temp_code = inbuff[0]|(inbuff[1]<<8);
	else	return One_Wire_CRC_Error;
	return One_Wire_Success;
}

