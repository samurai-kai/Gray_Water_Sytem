#include <Arduino.h>
#include "lcd_driver.h"

void setup() {
    lcd_init();
    lcd_print(0, 0, "1602A Parallel");
    lcd_print(0, 1, "ESP32 Ready!");
}

void loop() {}
