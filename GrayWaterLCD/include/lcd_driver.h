#pragma once
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Most I2C backpacks use address 0x27 or 0x3F
#define LCD_ADDRESS 0x27

// Create LCD object (16 columns, 2 rows)
static LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

inline void lcd_init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

inline void lcd_print(int col, int row, const char* text) {
    lcd.setCursor(col, row);
    lcd.print(text);
}
