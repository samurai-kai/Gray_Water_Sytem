#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Display.h"
#include "config_secret.h"

// ---------------- DISPLAY DRIVER ----------------
Display display(0, 20000);   // BOOT pin 0, 20-second timeout

// ---------------- MQTT SETTINGS -----------------
const char* MQTT_SERVER = "192.168.1.87";
const int   MQTT_PORT   = 1883;

// CLEAN TANK TOPICS
const char* T_CLEAN_PERCENT     = "graywater/clean/percent";
const char* T_CLEAN_GALLONS     = "graywater/clean/gallons";
const char* T_CLEAN_PUMP        = "graywater/clean/pump_state";
const char* T_CLEAN_CYCLES      = "graywater/clean/cycles";
const char* T_CLEAN_TOTAL       = "graywater/clean/total_gallons";

// DIRTY TANK TOPICS
const char* T_DIRTY_PERCENT     = "graywater/dirty/percent";
const char* T_DIRTY_GALLONS     = "graywater/dirty/gallons";
const char* T_DIRTY_PUMP        = "graywater/dirty/pump_state";

// ------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- WIFI CONNECT ------------------
void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    display.setWiFiStatus(true);
}

// -------------- MQTT CONNECT --------------------
void connectMQTT() {
    client.setServer(MQTT_SERVER, MQTT_PORT);

    Serial.print("Connecting MQTT...");
    while (!client.connected()) {
        if (client.connect("graywater_display", MQTT_USER, MQTT_PASS)) {

            Serial.println("connected!");
            display.setMQTTStatus(true);

            // SUBSCRIBE TO ALL UPDATED TOPICS
            client.subscribe(T_CLEAN_PERCENT);
            client.subscribe(T_CLEAN_GALLONS);
            client.subscribe(T_CLEAN_PUMP);
            client.subscribe(T_CLEAN_CYCLES);
            client.subscribe(T_CLEAN_TOTAL);

            client.subscribe(T_DIRTY_PERCENT);
            client.subscribe(T_DIRTY_GALLONS);
            client.subscribe(T_DIRTY_PUMP);

        } else {
            Serial.print("FAILED rc=");
            Serial.print(client.state());
            Serial.println(" retrying...");
            display.setMQTTStatus(false);
            delay(2000);
        }
    }
}

// -------------- MQTT CALLBACK -------------------
void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    float value = atof((char*)payload);

    Serial.printf("MQTT: %s = %.2f\n", topic, value);
    display.notifyDataUpdate();

    // ---------------- CLEAN TANK ----------------
    if (strcmp(topic, T_CLEAN_PERCENT) == 0)
        display.setCleanLevel((int)value);

    if (strcmp(topic, T_CLEAN_TOTAL) == 0)
        display.setGallonsSaved((int)value);

    // if (strcmp(topic, T_CLEAN_PUMP) == 0)
    //     display.setCleanPumpState((int)value);

    if (strcmp(topic, T_CLEAN_CYCLES) == 0)
        display.setCycles((int)value);

    // ---------------- DIRTY TANK ----------------
    if (strcmp(topic, T_DIRTY_PERCENT) == 0)
        display.setDirtyLevel((int)value);

    // if (strcmp(topic, T_DIRTY_PUMP) == 0)
    //     display.setDirtyPumpState((int)value);
}

// ------------------ SETUP -----------------------
void setup() {
    Serial.begin(115200);

    display.begin();

    connectWiFi();
    display.setWiFiStatus(WiFi.status() == WL_CONNECTED);

    client.setCallback(callback);
    connectMQTT();
}

// ------------------ LOOP ------------------------
void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();

    display.update();
    display.setWiFiStatus(WiFi.status() == WL_CONNECTED);
}
