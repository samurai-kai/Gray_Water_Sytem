#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display {
public:
    Display(uint8_t bootPin, unsigned long timeoutMs);   // UPDATED

    void begin();
    void update();
    void showScreen(uint8_t screen);

private:
    LiquidCrystal_I2C lcd;
    uint8_t BOOT_BUTTON;

    uint8_t currentScreen = 0;

    // debounce variables
    bool lastState = HIGH;
    bool lastStable = HIGH;
    unsigned long lastDebounce = 0;
    const unsigned long debounceDelay = 40;

    // timeout
    unsigned long timeout;            // UPDATED: configurable
    unsigned long lastInteraction = 0;

    void nextScreen();
};
