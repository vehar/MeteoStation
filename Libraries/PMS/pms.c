#include <stm32f10x.h>
#include "pms.h"
#include "macros.h"
#include "pinmap.h"

RingBuffer Pms_RxBuff;
RingBuffer Pms_TxBuff;

  uint8_t 	_status = STATUS_WAITING;
  bool pms_data_available = false; 

  uint8_t 	_mode;
  uint8_t 	_payload[24];
  uint8_t* 	_msgBuff;
  volatile PMS_DATA		_pms;
  uint8_t 	_index = 0;
  uint16_t 	_frameLen;
  uint16_t 	_checksum;
  uint16_t 	_crc;	

void PMS_Config()
{
	PIN_CONFIGURATION(PMS_Rst);
	PIN_CONFIGURATION(PMS_Set);
	PIN_ON(PMS_Rst); //active LOW
	PIN_ON(PMS_Set);
	
}

// Non-blocking function for parse response.
bool PMS_read(PMS_DATA* data, uint8_t* msgBuff)
{
  //_pms = data;
  _msgBuff = msgBuff;
  bool result = false;
  do{
//	result = PMS_fsm();
	}while(result == true);
  
  return _status == STATUS_OK;
}

bool PMS_fsm(char ch)
{
	if (_status == STATUS_OK){return true;}// not procecced yet
	
	_status = STATUS_WAITING;

    switch (_index)
    {
    case 0:
						if (ch != 0x42){return false;}
						_crc = ch;
    break;

    case 1:
						if (ch != 0x4D)
						{
							_index = 0;
							return false;
						}
						_crc += ch;
    break;

    case 2:
						_crc += ch;
						_frameLen = ch << 8;
    break;

    case 3:
						_frameLen |= ch;
						// Unsupported sensor, different frame length, transmission error e.t.c.
						if (_frameLen != 2 * 9 + 2 && _frameLen != 2 * 13 + 2)
						{
							_index = 0;
							return false;
						}
						_crc += ch;
    break;

    default:
						if (_index == _frameLen + 2)
						{
							_checksum = ch << 8;
						}
						else if (_index != _frameLen + 2 + 1)
						{
							_crc += ch;
							uint8_t payloadIndex = _index - 4;

							// Payload is common to all sensors (first 2x6 bytes).
							if (payloadIndex < sizeof(_payload))
							{
								_payload[payloadIndex] = ch;
							}
						}
						else
						{
							_checksum |= ch;

							if (_crc == _checksum)
							{
								_status = STATUS_OK;

								// Standard Particles, CF=1.
								_pms.PM_SP_UG_1_0 = MAKEWORD(_payload[0], _payload[1]);
								_pms.PM_SP_UG_2_5 = MAKEWORD(_payload[2], _payload[3]);
								_pms.PM_SP_UG_10_0 = MAKEWORD(_payload[4], _payload[5]);

								// Atmospheric Environment.
								_pms.PM_AE_UG_1_0 = MAKEWORD(_payload[6], _payload[7]);
								_pms.PM_AE_UG_2_5 = MAKEWORD(_payload[8], _payload[9]);
								_pms.PM_AE_UG_10_0 = MAKEWORD(_payload[10], _payload[11]);

									// The number of particles
								_pms.PM_NP_UG_0_3 = MAKEWORD(_payload[12], _payload[13]);
								_pms.PM_NP_UG_0_5 = MAKEWORD(_payload[14], _payload[15]);
								_pms.PM_NP_UG_1_0 = MAKEWORD(_payload[16], _payload[17]);								
								_pms.PM_NP_UG_2_5 = MAKEWORD(_payload[18], _payload[19]);
								_pms.PM_NP_UG_5_0 = MAKEWORD(_payload[20], _payload[21]);
								_pms.PM_NP_UG_10_0 = MAKEWORD(_payload[22], _payload[23]);									
							}
							_index = 0;
							return false;
						}
      break;
    }
		_index++;
  return true;
}

void PMS_IRQHandler(void)
{
	char t = 0;
  if(USART_GetITStatus(PMS_UART, USART_IT_RXNE) != RESET)
  {
			t = USART_ReceiveData(PMS_UART);
			WriteByte(&PMS_RX_BUFF, t);
			PMS_fsm(t);
  }
	if(USART_GetITStatus(PMS_UART, USART_IT_TC) != RESET)
  {
    USART_ClearITPendingBit(PMS_UART, USART_IT_TC);
    if(!IsEmpty(&PMS_TX_BUFF))
		{
			USART_SendData(PMS_UART,ReadByte(&PMS_TX_BUFF));
		}
		else
		{
			USART_ITConfig(PMS_UART, USART_IT_TC, DISABLE);
		}
  }
}