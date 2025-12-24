#include "Display.h"
#include <Wire.h>

#define LCD_ADDRESS 0x27   // change to 0x3F if needed

// ---- Constructor ----
Display::Display(uint8_t bootPin, unsigned long timeoutMs)
: lcd(LCD_ADDRESS, 16, 2), BOOT_BUTTON(bootPin), timeout(timeoutMs) {}

// ---- Setup ----
void Display::begin() {
    Wire.begin(21, 22);  // SDA, SCL
    lcd.init();
    lcd.backlight();

    pinMode(BOOT_BUTTON, INPUT_PULLUP);

    lastInteraction = millis();
    showScreen(currentScreen);
}

// ---- Update ----
void Display::update() {

    // ---------- PUMP WARNING OVERRIDE ----------
    if (cleanPumpOn || dirtyPumpOn) {
        if (!pumpWarningActive) {
            screenBeforeWarning = currentScreen;
            pumpWarningActive = true;
            displayLocked = true;
            showPumpWarning();
        }
        return;   // HARD LOCK: nothing else runs
    }

    // ---------- EXIT WARNING MODE ----------
    if (pumpWarningActive) {
        pumpWarningActive = false;
        displayLocked = false;
        showScreen(screenBeforeWarning);
    }

    // ---------- BUTTON DEBOUNCE ----------
    bool reading = digitalRead(BOOT_BUTTON);

    if (reading != lastStable) {
        lastDebounce = millis();
    }

    if ((millis() - lastDebounce) > debounceDelay) {
        if (reading != lastState) {
            lastState = reading;
            if (lastState == LOW) {
                nextScreen();
                lastInteraction = millis();
            }
        }
    }

    lastStable = reading;

    // ---------- TIMEOUT ----------
    if (timeout > 0 && currentScreen != 0) {
        if (millis() - lastInteraction > timeout) {
            currentScreen = 0;
            showScreen(0);
        }
    }

    // ---------- STATUS SCREEN ----------
    if (currentScreen == 0) {
        updateStatusScreen();
    }
}

// ---- Warning Screen ----
void Display::showPumpWarning() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" !! PUMPS ON !! ");
    lcd.setCursor(0, 1);
    lcd.print("Do NOT run wash");
}

// ---- Screen Definitions ----
void Display::showScreen(uint8_t screen) {
    lcd.clear();
    currentScreen = screen;

    switch (screen) {
        case 0:
            lcd.setCursor(0, 0);
            lcd.print("WiFi:   SVR:  ");
            lcd.setCursor(0, 1);
            lcd.print("Last:         ");
            break;

        case 1:
            lcd.setCursor(0, 0);
            lcd.print("Dirty Water Lvl");
            lcd.setCursor(0, 1);
            lcd.print("Tank 1:        ");
            break;

        case 2:
            lcd.setCursor(0, 0);
            lcd.print("Clean Water Lvl");
            lcd.setCursor(0, 1);
            lcd.print("Tank 2:        ");
            break;

        case 3:
            lcd.setCursor(0, 0);
            lcd.print("Total Cycles");
            lcd.setCursor(0, 1);
            lcd.print("                ");
            break;

        case 4:
            lcd.setCursor(0, 0);
            lcd.print("Amt Water Saved");
            lcd.setCursor(0, 1);
            lcd.print("                ");
            break;
    }
}

// ---- Status Screen ----
void Display::updateStatusScreen() {
    if (displayLocked) return;

    unsigned long now = millis();
    bool wifiChanged = (wifiConnected != lastWiFi);
    bool mqttChanged = (mqttConnected != lastMQTT);
    bool updateTimeChanged = (lastUpdateTime != lastLastUpdateTime);
    bool intervalPassed = (now - lastStatusRedraw > statusRedrawInterval);

    if (!(wifiChanged || mqttChanged || updateTimeChanged || intervalPassed)) return;

    lcd.setCursor(5, 0);
    lcd.print(wifiConnected ? "OK " : "ERR");

    lcd.setCursor(12, 0);
    lcd.print(mqttConnected ? "OK " : "ERR");

    lcd.setCursor(5, 1);
    if (lastUpdateTime == 0) {
        lcd.print(" -    ");
    } else {
        unsigned long secs = (now - lastUpdateTime) / 1000;
        char buf[7];
        sprintf(buf, "%lu s ", secs);
        lcd.print(buf);
    }

    lastWiFi = wifiConnected;
    lastMQTT = mqttConnected;
    lastLastUpdateTime = lastUpdateTime;
    lastStatusRedraw = now;
}

// ---- Screen Cycling ----
void Display::nextScreen() {
    currentScreen = (currentScreen + 1) % 5;
    showScreen(currentScreen);
}

// ---- Data Setters (LCD SAFE) ----
void Display::setDirtyLevel(int value) {
    dirtyLevel = value;
    if (displayLocked || currentScreen != 1) return;

    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.setCursor(8, 1);
    lcd.print(dirtyLevel);
    lcd.print("%");
}

void Display::setCleanLevel(int value) {
    cleanLevel = value;
    if (displayLocked || currentScreen != 2) return;

    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.setCursor(8, 1);
    lcd.print(cleanLevel);
    lcd.print("%");
}

void Display::setCycles(int value) {
    cycles = value;
    if (displayLocked || currentScreen != 3) return;

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(cycles);
}

void Display::setGallonsSaved(int value) {
    gallonsSaved = value;
    if (displayLocked || currentScreen != 4) return;

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(gallonsSaved);
    lcd.print(" gals");
}

// ---- Pump State ----
void Display::setCleanPumpState(bool on) {
    cleanPumpOn = on;
}

void Display::setDirtyPumpState(bool on) {
    dirtyPumpOn = on;
}

// ---- Connectivity ----
void Display::setWiFiStatus(bool connected) {
    wifiConnected = connected;
}

void Display::setMQTTStatus(bool connected) {
    mqttConnected = connected;
}

void Display::notifyDataUpdate() {
    lastUpdateTime = millis();
}
