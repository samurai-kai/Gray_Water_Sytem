#include "Display.h"
#include <Wire.h>

#define LCD_ADDRESS 0x27   // change to 0x3F if needed

// ---- Constructor ----
Display::Display(uint8_t bootPin, unsigned long timeoutMs)
: lcd(LCD_ADDRESS, 16, 2), BOOT_BUTTON(bootPin), timeout(timeoutMs) {}
// timeoutMs is now stored in `timeout`

// ---- Setup ----
void Display::begin() {
    Wire.begin(21, 22);  // SDA, SCL
    lcd.init();
    lcd.backlight();

    pinMode(BOOT_BUTTON, INPUT_PULLUP);

    lastInteraction = millis();   // initialize timer

    showScreen(currentScreen);
}

// ---- Update (debounce + screen change + timeout) ----
void Display::update() {
    bool reading = digitalRead(BOOT_BUTTON);

    // debounce check
    if (reading != lastStable) {
        lastDebounce = millis();
    }

    if ((millis() - lastDebounce) > debounceDelay) {
        if (reading != lastState) {
            lastState = reading;

            if (lastState == LOW) {  // active LOW
                Serial.println("BOOT pressed");
                nextScreen();
                lastInteraction = millis();   // reset inactivity timer
            }
        }
    }

    lastStable = reading;

    // ---- TIMEOUT LOGIC ----
    if (timeout > 0 && currentScreen != 0) {
        if (millis() - lastInteraction > timeout) {
            currentScreen = 0;
            showScreen(0);
        }
    }
}

// ---- Screen definitions ----
void Display::showScreen(uint8_t screen) {
    lcd.clear();

    switch (screen) {
        case 0:
            lcd.setCursor(0, 0);
            lcd.print("SCREEN 0");
            lcd.setCursor(0, 1);
            lcd.print("Hello!");
            break;

        case 1:
            lcd.setCursor(0, 0);
            lcd.print("SCREEN 1");
            lcd.setCursor(0, 1);
            lcd.print("Tank: 42%");
            break;

        case 2:
            lcd.setCursor(0, 0);
            lcd.print("SCREEN 2");
            lcd.setCursor(0, 1);
            lcd.print("Cycles: 123");
            break;

        default:
            lcd.setCursor(0, 0);
            lcd.print("Invalid");
            lcd.setCursor(0, 1);
            lcd.print("Screen");
            currentScreen = 0;
            break;
    }
}

// ---- Advance screen ----
void Display::nextScreen() {
    currentScreen = (currentScreen + 1) % 3;
    showScreen(currentScreen);
}
