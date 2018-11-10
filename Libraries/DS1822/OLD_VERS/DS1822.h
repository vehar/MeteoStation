#define DS1822_CONVERT_T_CMD				0x44
#define DS1822_WRITE_STRATCHPAD_CMD			0x4E
#define DS1822_READ_STRATCHPAD_CMD			0xBE
#define DS1822_COPY_STRATCHPAD_CMD			0x48
#define DS1822_RECALL_E_CMD					0xB8
#define DS1822_READ_POWER_SUPPLY_CMD		0xB4

#define DS1822_STRATCHPAD_SIZE				0x09
#define DS1822_SERIAL_NUM_SIZE				0x08

#define One_Wire_Device_Number_MAX				5	//maximum number of 1-wire devices on bus

unsigned int DS1822_Start_Conversion_Skip_Rom (unsigned long PIN);
unsigned int DS1822_Read_Temp_NoCRC_Skip_Rom (unsigned long PIN);
unsigned int DS1822_Read_Temp_CRC_Check (unsigned long PIN);
unsigned int DS1822_Read_Rom(unsigned long PIN);
unsigned int DS1822_Send_Cmd_By_Rom (unsigned long PIN);
unsigned int DS1822_Start_Conversion_Cmd (unsigned long PIN);
unsigned int DS1822_Read_Temp_NoCRC_By_Rom (unsigned long PIN);
unsigned char DS1822_Search_Rom_One_Device (unsigned long PIN, unsigned char (*Serial_Num)[DS1822_SERIAL_NUM_SIZE]);
unsigned char DS1822_Start_Conversion_by_ROM (unsigned long PIN, unsigned char (*Serial_Num)[DS1822_SERIAL_NUM_SIZE]);
unsigned char DS1822_Get_Conversion_Result_by_ROM_CRC (unsigned long PIN, unsigned char (*Serial_Num)[DS1822_SERIAL_NUM_SIZE], unsigned int * temp_code);

