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
    // Only clear when switching between screens.
    if (!suppressClear) {
        lcd.clear();
    }

    suppressClear = false; // reset for next time

    switch (screen) {
        case 0:
          {
              unsigned long now = millis();

              // Redraw only if needed
              bool wifiChanged       = (wifiConnected != lastWiFi);
              bool mqttChanged       = (mqttConnected != lastMQTT);
              bool updateTimeChanged = (lastUpdateTime != lastLastUpdateTime);
              bool intervalPassed    = (now - lastStatusRedraw > statusRedrawInterval);

              if (wifiChanged || mqttChanged || updateTimeChanged || intervalPassed) {

                  // prevent clearing during status refresh
                  suppressClear = true;     

                  lcd.setCursor(0, 0);
                  lcd.print("WiFi:");
                  lcd.print(wifiConnected ? "OK " : "ERR");

                  lcd.print(" SVR:");
                  lcd.print(mqttConnected ? "OK " : "ERR");

                  lcd.setCursor(0, 1);
                  lcd.print("Last:");

                  if (lastUpdateTime == 0) {
                      lcd.print(" -     ");  // pad to clear leftovers
                  } else {
                      unsigned long secs = (now - lastUpdateTime) / 1000;
                      lcd.print(secs);
                      lcd.print("s ago ");
                  }

                  // snapshot tracking
                  lastWiFi = wifiConnected;
                  lastMQTT = mqttConnected;
                  lastLastUpdateTime = lastUpdateTime;
                  lastStatusRedraw = now;
              }
          }
          break;

        case 1:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Dirty Water Lvl");
            lcd.setCursor(0, 1);
            lcd.print("Tank 1: ");
            lcd.print(dirtyLevel);
            lcd.print("%   ");
            break;

        case 2:
            lcd.setCursor(0, 0);
            lcd.print("Clean Water Lvl");
            lcd.setCursor(0, 1);
            lcd.print("Tank 2: ");
            lcd.print(cleanLevel);
            lcd.print("%   ");
            break;

        case 3:
            lcd.setCursor(0, 0);
            lcd.print("Total Cycles");
            lcd.setCursor(0, 1);
            lcd.print(cycles);
            break;

        case 4:
            lcd.setCursor(0, 0);
            lcd.print("Amt Water Saved");
            lcd.setCursor(0, 1);
            lcd.print(gallonsSaved);
            lcd.print(" gals   ");
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
    currentScreen = (currentScreen + 1) % 5;
    showScreen(currentScreen);
}

void Display::setDirtyLevel(int value) {
    dirtyLevel = value;
    if (currentScreen == 0) showScreen(0);
}

void Display::setCleanLevel(int value) {
    cleanLevel = value;
    if (currentScreen == 1) showScreen(1);
}

void Display::setCycles(int value) {
    cycles = value;
    if (currentScreen == 2) showScreen(2);
}

void Display::setGallonsSaved(int value) {
    gallonsSaved = value;
    if (currentScreen == 3) showScreen(3);
}

void Display::setWiFiStatus(bool connected) {
    wifiConnected = connected;
    if (currentScreen == 4) {
        suppressClear = true;   // prevents flash
        showScreen(4);
    }
}

void Display::setMQTTStatus(bool connected) {
    mqttConnected = connected;
    if (currentScreen == 4) {
        suppressClear = true;   // prevents flash
        showScreen(4);
    }
}

void Display::notifyDataUpdate() {
    lastUpdateTime = millis();
    if (currentScreen == 4) {
        suppressClear = true;   // prevents flash
        showScreen(4);
    }
}

