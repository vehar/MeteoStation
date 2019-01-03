#include "pms.h"

  uint8_t 	_status = STATUS_WAITING;
  bool pms_data_available = false; 

  uint8_t 	_mode;
  uint8_t 	_payload[12];
  uint8_t* 	_msgBuff;
  PMS_DATA* 		_data;
  uint8_t 	_index = 0;
  uint16_t 	_frameLen;
  uint16_t 	_checksum;
  uint16_t 	_calculatedChecksum;	


// Non-blocking function for parse response.
bool PMS_read(PMS_DATA* data, uint8_t* msgBuff)
{
  _data = data;
  _msgBuff = msgBuff;
  bool result = false;
  do{
	result = loop();
	}while(result == true);
  
  return _status == STATUS_OK;
}

bool loop()
{
	_status = STATUS_WAITING;
    uint8_t ch = _msgBuff[_index];//_stream->read();

    switch (_index)
    {
    case 0:
      if (ch != 0x42)
      {
        return false;
      }
      _calculatedChecksum = ch;
      break;

    case 1:
      if (ch != 0x4D)
      {
        _index = 0;
        return false;
      }
      _calculatedChecksum += ch;
      break;

    case 2:
      _calculatedChecksum += ch;
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
      _calculatedChecksum += ch;
      break;

    default:
      if (_index == _frameLen + 2)
      {
        _checksum = ch << 8;
      }
      else if (_index == _frameLen + 2 + 1)
      {
        _checksum |= ch;

        if (_calculatedChecksum == _checksum)
        {
          _status = STATUS_OK;

          // Standard Particles, CF=1.
          _data->PM_SP_UG_1_0 = MAKEWORD(_payload[0], _payload[1]);
          _data->PM_SP_UG_2_5 = MAKEWORD(_payload[2], _payload[3]);
          _data->PM_SP_UG_10_0 = MAKEWORD(_payload[4], _payload[5]);

          // Atmospheric Environment.
          _data->PM_AE_UG_1_0 = MAKEWORD(_payload[6], _payload[7]);
          _data->PM_AE_UG_2_5 = MAKEWORD(_payload[8], _payload[9]);
          _data->PM_AE_UG_10_0 = MAKEWORD(_payload[10], _payload[11]);
        }

        _index = 0;
        return false;
      }
      else
      {
        _calculatedChecksum += ch;
        uint8_t payloadIndex = _index - 4;

        // Payload is common to all sensors (first 2x6 bytes).
        if (payloadIndex < sizeof(_payload))
        {
          _payload[payloadIndex] = ch;
        }
      }
      break;
    }
    _index++;
  return true;
}