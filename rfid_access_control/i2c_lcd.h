// i2c_lcd.h
#ifndef I2C_LCD_H
#define I2C_LCD_H

void lcd_init(int i2c_addr);
void lcd_clear();
void lcd_set_cursor(int row, int col);
void lcd_print(const char *str);

#endif
