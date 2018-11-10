/**
 * Nokia 1202/1203/1280 LCD display driver
 *
 * (CC) 2011 Ivan A-R <ivan@tuxotronic.org>
 *
 *
 */

#include "lcd1202.h"

#include <stm32f10x.h>
#include <string.h>

#include "font5x8.h"
#include "fnt_libera_24x14.xbm"
// #include "fnt_terminus_24x14.xbm"

#define _LCD1202_C_
#include "lcd1202_hw.h"

/* hw undepended functions */

lcd_flags_t lcd_flags;

void lcd_init(void)
{
    lcd_init_gpio();
    lcd_reset();

    lcd_cs_mark();
    lcd_write_byte(0xE2, 0); // Reset
    lcd_delay_us(10000);
     lcd_write_byte(0x3D, 0);  // Charge pump
     lcd_write_byte(0x01, 0);  // Charge pump = 4 (default 5 is too hight for 3.0 volt)
     lcd_write_byte(0xE1, 0);  // Additional VOP for contrast increase
     lcd_write_byte(0x28, 0);  // from -127 to +127
    lcd_write_byte(0xA4, 0); // Power saver off
    lcd_write_byte(0x2F, 0); // Booster ON Voltage regulator ON Voltage follover ON
		lcd_write_byte(0xA0, 0); // Segment driver direction select: Normal
    lcd_write_byte(0xAF, 0); // LCD display on
    lcd_cs_release();

    lcd_flags.inverse_char = 0;

    lcd_clr();
}

/* Page address set */
#define lcd_set_row(row) lcd_write_byte(0xB0 | ((row) & 0x0F), 0) 

#define lcd_set_col(col) do { \
        lcd_write_byte(0x10 | ((col)>>4), 0); /* Sets the DDRAM column address - upper 3-bit */ \
        lcd_write_byte(0x00 | ((col) & 0x0F), 0); /* lower 4-bit */ \
    } while(0)


void lcd_clr(void)
{
    int i;
    lcd_home();
    lcd_cs_mark();
    for(i=0; i<16*6*9; i++)
    {
        lcd_write_byte(0x00, 1);
    }
    lcd_cs_release();
}

void lcd_reverse(lcd_reverse_t inversion)
{
    lcd_cs_mark();
    if(inversion)
        lcd_write_byte(0xA7, 0); // reverse display
    else
        lcd_write_byte(0xA6, 0); // normal display
    lcd_cs_release();
}

void lcd_segment_direction(lcd_reverse_t reverse)
{
    lcd_cs_mark();
    if(reverse)
        lcd_write_byte(0xA1, 0); // reverse segment direction
    else
        lcd_write_byte(0xA0, 0); // normal segment direction
    lcd_cs_release();
}

void lcd_column_direction(lcd_reverse_t reverse)
{
    lcd_cs_mark();
    if(reverse)
        lcd_write_byte(0xC8, 0); // reverse column direction
    else
        lcd_write_byte(0xC0, 0); // normal column direction
    lcd_cs_release();
}

void lcd_contrast(int contrast)
{
    contrast = (contrast + 16) & 0x1F;
    lcd_cs_mark();
    lcd_write_byte(0x80 | contrast, 0); // Set electronic volume
    lcd_cs_release();
}

void lcd_inverse_line(int line)
{
    lcd_cs_mark();
    if((line >= 0) && (line <32))
    {
        lcd_write_byte(0xAD, 0);
        lcd_write_byte(line | 0x20, 0);
    }
    else
    {
        lcd_write_byte(0xAD, 0);
        lcd_write_byte(0, 0);
    }
    lcd_cs_release();
}

void lcd_start_line(uint8_t line)
{
    lcd_cs_mark();
    lcd_write_byte(0x40 | (line & 0x3F), 0);
    lcd_cs_release();
}

void lcd_home(void)
{
    lcd_cs_mark();
    lcd_set_row(0);
    lcd_set_col(0);
    lcd_cs_release();
}

void lcd_char(uint8_t row, uint8_t col, char chr)
{
    int i;
    const uint8_t* f = font5x8 + chr*5;
    lcd_cs_mark();
    lcd_set_row(row);
    lcd_set_col(col);
    for(i=0; i<fontWidth; i++)
    {
        uint8_t b = *f++;
        if(lcd_flags.inverse_char) b = ~b;
        lcd_write_byte(b, 1);
        if(++col >= lcdWidth())
        {
            lcd_cs_release();
            return;
        }
    }
    for(i=0; i<fontSpace; i++)
    {
        if(lcd_flags.inverse_char)
            lcd_write_byte(0xFF, 1);
        else
            lcd_write_byte(0, 1);
    }
    lcd_cs_release();
}

void lcd_str(uint8_t row, uint8_t col, const char* str)
{
    char c;
    while((c = *str++) != 0)
    {
        lcd_char(row, col, c);
        col += fontWidth + fontSpace;
        if(col >= lcdWidth()) return;
    }
}

void lcd_str_center(uint8_t row, const char* str)
{
    lcd_str(row,
        (lcdWidth()-strlen(str)*(fontWidth+fontSpace))>>1,
        str);
}

void lcd_big_char(uint8_t row, uint8_t col, char chr)
{
    // uint8_t* f0 = fnt_terminus_24x14_bits + chr*14*(24/8);
    const uint8_t* f0 = fnt_libera_24x14_bits + (chr-'0')*14*(24/8);
    const uint8_t* f;
    uint8_t r, c;
    lcd_cs_mark();
    for(r = 0; r<3; r++)
    {
        f = f0 + r;
        lcd_set_row(row+r);
        lcd_set_col(col);
        for(c = 0; c<14; c++)
        {
            uint8_t b = *f;
            if(lcd_flags.inverse_char) b = ~b;
            lcd_write_byte(b, 1);
            f += 3;
        }
    }
    lcd_cs_release();
}

void lcd_big_str(uint8_t row, uint8_t col, const char* str)
{
    char c;
    while((c = *str++) != 0)
    {
        lcd_char(row, col, c);
        col += fnt_libera_24x14_width;
        if(col >= lcdWidth()) return;
    }
}

void lcd_big_str_center(uint8_t row, const char* str)
{
    lcd_big_str(row,
        (lcdWidth()-strlen(str)*fnt_libera_24x14_width)>>1,
        str);
}

void lcd_image(uint8_t row, uint8_t col,
        uint8_t height, uint8_t widht,
        const uint8_t* img)
{
    uint8_t r, c;
    lcd_cs_mark();
    for(r = 0; r < height; r++)
    {
        lcd_set_row(row+r);
        lcd_set_col(col);
        for(c = 0; c < widht; c++)
        {
            uint8_t b = *img++;
            if(lcd_flags.inverse_char) b = ~b;
            lcd_write_byte(b, 1);
        }
    }
    lcd_cs_release();
}

