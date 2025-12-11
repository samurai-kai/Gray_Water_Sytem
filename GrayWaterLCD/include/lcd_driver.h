#pragma once
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
static LiquidCrystal lcd(14, 12, 27, 26, 25, 33);

inline void lcd_init() {
    lcd.begin(16, 2);
    lcd.clear();
}

inline void lcd_print(int col, int row, const char* text) {
    lcd.setCursor(col, row);
    lcd.print(text);
}
