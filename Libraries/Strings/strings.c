#include "strings.h"

unsigned char* adc2str(uint_fast16_t d,unsigned char* out)
{
    out[4] = '\0';
    out[3] = '0' + ( d       )    % 10;
    out[2] = '0' + ( d /= 10 )    % 10;
    out[1] = '0' + ( d /= 10 )    % 10;
    out[0] = '0' + ( d /  10 )    % 10;
    return out;
}

unsigned long bin2bcd_U32_soft(unsigned long data, unsigned char result_bytes) 
{
    unsigned long result = 0; /*result*/
	unsigned char cnt_bytes,cnt_bits;
    for (cnt_bytes=(4 - result_bytes); cnt_bytes; cnt_bytes--) /* adjust input bytes */
        data <<= 8;
    for (cnt_bits=(result_bytes << 3); cnt_bits; cnt_bits--) 
	{ /* bits shift loop */
        /*result BCD nibbles correction*/
        result += 0x33333333;
        /*result correction loop*/
        for (cnt_bytes=4; cnt_bytes; cnt_bytes--) 
		{
            unsigned char corr_byte = result >> 24;
            if (!(corr_byte & 0x08)) corr_byte -= 0x03;
            if (!(corr_byte & 0x80)) corr_byte -= 0x30;
            result <<= 8; /*shift result*/
            result += corr_byte; /*set 8 bits of result*/
        }
        /*shift next bit of input to result*/
        result <<= 1;
        if (((unsigned char)(data >> 24)) & 0x80)
            result |= 1;
        data <<= 1;
    }
    return(result);
}

unsigned long bcd2bin_U32_soft(unsigned long data, unsigned char input_bytes) {
    unsigned long result = 0; /*result*/
	unsigned char cnt_bytes,cnt_bits;
    for (cnt_bits = (input_bytes << 3); cnt_bits; cnt_bits--) 
	{
        /*shift next bit*/
        result >>= 1;
        if (((unsigned char)(data)) & (unsigned char)0x01) result |= 0x80000000;
        data >>= 1;
        /* result BCD correction */
        for (cnt_bytes = 4; cnt_bytes; cnt_bytes--) 
		{
            unsigned char tmp_byte = (data >> 24);
            if (tmp_byte & 0x80) tmp_byte -= 0x30;
            if (tmp_byte & 0x08) tmp_byte -= 0x03;
            data <<= 8;
            data |= tmp_byte;
        }
    }
    /*adjust result bytes*/
    for (cnt_bits = (4 - input_bytes); cnt_bits; cnt_bits--)
        result >>= 8;
    return(result);
}
