/**
 * Nokia 1202/1203/1280 LCD display driver
 *
 * (CC) 2011 Ivan A-R <ivan@tuxotronic.org>
 *
 * STE2007 LCD COG driver
 * 96 x 68 pixels
 * 1.3" 27.5 x 19.5 mm active area
 *
 */

#ifndef _LCD1202_H_
#define _LCD1202_H_

#include <inttypes.h>

#define USE_MCU_USART

typedef enum {
    lrNormal = 0,
    lrReverse
} lcd_reverse_t;

typedef struct {
    int inverse_char:1;
} lcd_flags_t;

extern lcd_flags_t lcd_flags;

#define lcdWidth() 96
#define lcdHeight() 68
#define lcdRows() (lcdHeight()/8)

void lcd_init(void);
void lcd_clr(void);
void lcd_home(void);

void lcd_char(uint8_t row, uint8_t col, char chr);
void lcd_str(uint8_t row, uint8_t col, const char* str);
void lcd_str_center(uint8_t row, const char* str);

void lcd_big_char(uint8_t row, uint8_t col, char chr);
void lcd_big_str(uint8_t row, uint8_t col, const char* str);
void lcd_big_str_center(uint8_t row, const char* str);

void lcd_image(uint8_t row, uint8_t col,
        uint8_t height, uint8_t widht,
        const uint8_t* img);

void lcd_inverse_line(int line);
void lcd_start_line(uint8_t line);
void lcd_reverse(lcd_reverse_t inversion);
void lcd_segment_direction(lcd_reverse_t reverse);
void lcd_column_direction(lcd_reverse_t reverse);
void lcd_contrast(int contrast);

#endif /* _LCD1202_H_ */

