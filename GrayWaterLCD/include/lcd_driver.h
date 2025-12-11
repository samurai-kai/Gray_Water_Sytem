#pragma once
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(23, 22, 19, 18, 17, 16);


inline void lcd_init() {
    lcd.begin(16, 2);   // 16x2 display
    lcd.clear();
}

inline void lcd_print(int col, int row, const char* text) {
    lcd.setCursor(col, row);
    lcd.print(text);
}
