#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

#include "Ultrasonic.h"
#include "Tank.h"
#include "Pump.h"
#include "config_secret.h"

// ------------------ USER SETTINGS ------------------
const char* MQTT_SERVER = "192.168.1.87";
const int   MQTT_PORT   = 1883;

// clean tank hardware
Ultrasonic sensorClean(5, 18);
Tank tankClean(30, 4, 5);
Pump pumpClean(26, 80, 20, false);

// dirty tank hardware
Ultrasonic sensorDirty(17, 16);
Tank tankDirty(30, 4, 5);
Pump pumpDirty(27, 80, 20, true);

// ---------------------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);
Preferences prefs;

// Persisted + runtime variables
float lastCleanGallons = 0;
float totalCleanGallonsOut = 0;
int cleanPumpCycles = 0;

// Pump cycle tracking state
bool cleanPumpPrevState = false;
float cleanPumpStartGallons = 0;
bool requestCleanPump = false;

// ---------------------------------------------------
// WIFI
void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
}

// MQTT
void connectMQTT() {
    client.setServer(MQTT_SERVER, MQTT_PORT);

    Serial.print("Connecting MQTT...");
    while (!client.connected()) {
        if (client.connect("graywater_sensor", MQTT_USER, MQTT_PASS)) {
            Serial.println("connected!");
        } else {
            Serial.print("failed rc=");
            Serial.print(client.state());
            Serial.println(" retrying...");
            delay(2000);
        }
    }
}
// ---------------------------------------------------

void setup() {
    Serial.begin(115200);

    sensorClean.begin();
    sensorDirty.begin();
    pumpClean.begin();
    pumpDirty.begin();

    connectWiFi();
    connectMQTT();

    
    prefs.begin("graywater", false);
    prefs.clear();
    prefs.end();    
    // Restore persistent values
    prefs.begin("graywater", false);
    cleanPumpCycles = prefs.getInt("cleanCycles", 0);
    totalCleanGallonsOut = prefs.getFloat("cleanTotalOut", 0.0f);
    prefs.end();

    Serial.printf("Restored cycles: %d\n", cleanPumpCycles);
    Serial.printf("Restored total gallons out: %.2f\n", totalCleanGallonsOut);

    // Initialize clean tank reference level
    float initialDist = sensorClean.readIn();
    float initialH = tankClean.getTankHeight() - initialDist;
    if (initialH < 0) initialH = 0;
    lastCleanGallons = tankClean.getGallons(initialH);
}

void loop() {

    if (!client.connected()) connectMQTT();
    client.loop();

    // ---------- CLEAN TANK ----------
    float dClean = sensorClean.readIn();
    float hClean = 0, galClean = 0, pctClean = 0;

    if (dClean >= 0) {
        hClean = tankClean.getTankHeight() - dClean;
        if (hClean < 0) hClean = 0;

        galClean = tankClean.getGallons(hClean);
        pctClean = tankClean.getPercentFull(hClean);

        lastCleanGallons = galClean;
    }
    delay(50);
    // ---------- DIRTY TANK ----------
    float dDirty = sensorDirty.readIn();
    float hDirty = 0, galDirty = 0, pctDirty = 0;

    if (dDirty >= 0) {
        hDirty = tankDirty.getTankHeight() - dDirty;
        if (hDirty < 0) hDirty = 0;

        galDirty = tankDirty.getGallons(hDirty);
        pctDirty = tankDirty.getPercentFull(hDirty);
    }

    // ---------- PUMP FSM UPDATES ----------
    pumpClean.update(
        pctClean,
        galClean,
        galDirty,
        tankDirty.getCapacity()
    );

    pumpDirty.update(
        pctDirty,
        galDirty,
        galClean,
        tankClean.getCapacity()
    );

    // ---------- CLEAN PUMP CYCLE TRACKING ----------
    bool cleanPumpNow = pumpClean.isRunning();

    // Pump turned ON → store starting gallons
    if (!cleanPumpPrevState && cleanPumpNow) {
        cleanPumpStartGallons = galClean;
    }

    // Pump turned OFF → cycle completed
    if (cleanPumpPrevState && !cleanPumpNow) {
        float pumped = cleanPumpStartGallons - galClean;
        if (pumped < 0) pumped = 0;

        cleanPumpCycles++;
        totalCleanGallonsOut += pumped;

        // Save persistent data
        prefs.begin("graywater", false);
        prefs.putInt("cleanCycles", cleanPumpCycles);
        prefs.putFloat("cleanTotalOut", totalCleanGallonsOut);
        prefs.end();

        Serial.printf("Cycle complete! Pumped: %.2f gallons\n", pumped);
    }

    cleanPumpPrevState = cleanPumpNow;

    // ---------- MQTT PUBLISH ----------
    // CLEAN tank
    client.publish("graywater/clean/percent", String(pctClean).c_str(), true);
    client.publish("graywater/clean/gallons", String(galClean).c_str(), true);
    client.publish("graywater/clean/cycles", String(cleanPumpCycles).c_str(), true);
    client.publish("graywater/clean/total_gallons", String(totalCleanGallonsOut).c_str(), true);
    client.publish("graywater/clean/pump_state", cleanPumpNow ? "1" : "0", true);

    // DIRTY tank
    client.publish("graywater/dirty/percent", String(pctDirty).c_str(), true);
    client.publish("graywater/dirty/gallons", String(galDirty).c_str(), true);
    client.publish("graywater/dirty/pump_state", pumpDirty.isRunning() ? "1" : "0", true);

    delay(2000);
}
