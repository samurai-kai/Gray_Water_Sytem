#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display {
public:
    Display(uint8_t bootPin, unsigned long timeoutMs);   // UPDATED

    void begin();
    void update();
    void showScreen(uint8_t screen);

    void setDirtyLevel(int value);
    void setCleanLevel(int value);
    void setCycles(int value);
    void setGallonsSaved(int value);

    void setWiFiStatus(bool connected);
    void setMQTTStatus(bool connected);
    void notifyDataUpdate();   // call this whenever MQTT data arrives


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

    // screens
    int dirtyLevel = 0;
    int cleanLevel = 0;
    int cycles = 0;
    int gallonsSaved = 0;

    // System status values
    bool wifiConnected = false;
    bool mqttConnected = false;
    unsigned long lastUpdateTime = 0;   // millis() timestamp of last MQTT message

    // For rate-limiting screen 4 updates
    unsigned long lastStatusRedraw = 0;
    const unsigned long statusRedrawInterval = 500;  // redraw every 2s max

    // Last known values to prevent redundant refreshes
    bool lastWiFi = false;
    bool lastMQTT = false;
    unsigned long lastLastUpdateTime = 0;
    bool suppressClear = false;


    void nextScreen();
};
