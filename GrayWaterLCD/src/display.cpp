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

    // ---- STATUS SCREEN REFRESH ----
    if (currentScreen == 0) {
        updateStatusScreen();
    }
}

// ---- Screen definitions ----
void Display::showScreen(uint8_t screen) {

    suppressClear = false;

    lcd.clear();
    currentScreen = screen;

    switch (screen) {

        // -------------------------
        //  SCREEN 0 – SYSTEM STATUS
        // -------------------------
        case 0:
            lcd.setCursor(0, 0);
            lcd.print("WiFi:   SVR:  ");
            lcd.setCursor(0, 1);
            lcd.print("Last:         ");
            break;

        // -------------------------
        //  SCREEN 1 – DIRTY WATER
        // -------------------------
        case 1:
            lcd.setCursor(0, 0);
            lcd.print("Dirty Water Lvl");
            lcd.setCursor(0, 1);
            lcd.print("Tank 1:        ");  // values overwrite
            break;

        // -------------------------
        //  SCREEN 2 – CLEAN WATER
        // -------------------------
        case 2:
            lcd.setCursor(0, 0);
            lcd.print("Clean Water Lvl");
            lcd.setCursor(0, 1);
            lcd.print("Tank 2:        ");
            break;

        // -------------------------
        //  SCREEN 3 – CYCLES
        // -------------------------
        case 3:
            lcd.setCursor(0, 0);
            lcd.print("Total Cycles");

            lcd.setCursor(0, 1);
            lcd.print("                ");          
            break;


        // -------------------------
        //  SCREEN 4 – GALLONS SAVED
        // -------------------------
        case 4:
            lcd.setCursor(0, 0);
            lcd.print("Amt Water Saved");

            lcd.setCursor(0, 1);
            lcd.print("                ");  
            break;

    }
}

void Display::updateStatusScreen() {

    unsigned long now = millis();
    bool wifiChanged       = (wifiConnected != lastWiFi);
    bool mqttChanged       = (mqttConnected != lastMQTT);
    bool updateTimeChanged = (lastUpdateTime != lastLastUpdateTime);
    bool intervalPassed    = (now - lastStatusRedraw > statusRedrawInterval);

    if (!(wifiChanged || mqttChanged || updateTimeChanged || intervalPassed)) return;

    // ---- UPDATE VALUES ONLY ----
    suppressClear = true;

    // WiFi
    lcd.setCursor(5, 0);
    lcd.print(wifiConnected ? "OK " : "ERR");

    // MQTT
    lcd.setCursor(12, 0);
    lcd.print(mqttConnected ? "OK " : "ERR");

    // Last Update
    lcd.setCursor(5, 1);
    if (lastUpdateTime == 0) {
        lcd.print(" -    ");
    } else {
        unsigned long secs = (now - lastUpdateTime) / 1000;
        char buf[7];
        sprintf(buf, "%lu s ", secs);
        lcd.print(buf);
    }

    // Snapshot
    lastWiFi = wifiConnected;
    lastMQTT = mqttConnected;
    lastLastUpdateTime = lastUpdateTime;
    lastStatusRedraw = now;
}

// ---- Advance screen ----
void Display::nextScreen() {
    currentScreen = (currentScreen + 1) % 5;
    showScreen(currentScreen);
}

void Display::setDirtyLevel(int value) {
    dirtyLevel = value;

    if (currentScreen == 1) {
        lcd.setCursor(8, 1);
        lcd.print("     ");       // clear old number
        lcd.setCursor(8, 1);
        lcd.print(dirtyLevel);
        lcd.print("%");
    }
}

void Display::setCleanLevel(int value) {
    cleanLevel = value;

    if (currentScreen == 2) {
        lcd.setCursor(8, 1);
        lcd.print("     ");
        lcd.setCursor(8, 1);
        lcd.print(cleanLevel);
        lcd.print("%");
    }
}

void Display::setCycles(int value) {
    cycles = value;

    if (currentScreen == 3) {
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print(cycles);
    }
}

void Display::setGallonsSaved(int value) {
    gallonsSaved = value;

    if (currentScreen == 4) {
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print(gallonsSaved);
        lcd.print(" gals");
    }
}

void Display::setWiFiStatus(bool connected) {
    wifiConnected = connected;
}

void Display::setMQTTStatus(bool connected) {
    mqttConnected = connected;
}

void Display::notifyDataUpdate() {
    lastUpdateTime = millis();
}

