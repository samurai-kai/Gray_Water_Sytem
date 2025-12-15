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

// Incoming topics (from your SENSOR ESP32)
const char* TOPIC_DISTANCE = "graywater/distance_in";
const char* TOPIC_HEIGHT   = "graywater/water_height_in";
const char* TOPIC_GALLONS  = "graywater/gallons";
const char* TOPIC_PERCENT  = "graywater/percent";

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

            // SUBSCRIBE TO SENSOR TOPICS
            client.subscribe(TOPIC_DISTANCE);
            client.subscribe(TOPIC_HEIGHT);
            client.subscribe(TOPIC_GALLONS);
            client.subscribe(TOPIC_PERCENT);

        } else {
            Serial.print("FAILED rc=");
            Serial.print(client.state());
            Serial.println(" retry in 2s...");

            display.setMQTTStatus(false);
            delay(2000);
        }
    }
}

// -------------- MQTT CALLBACK -------------------
void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String valueStr = String((char*)payload);
    float value = valueStr.toFloat();

    Serial.print("MQTT: ");
    Serial.print(topic);
    Serial.print(" = ");
    Serial.println(value);

    // NEW DATA RECEIVED
    display.notifyDataUpdate();

    // Interpret Topics → Display Variables
    if (strcmp(topic, TOPIC_PERCENT) == 0) {
        display.setDirtyLevel((int)value);
    }

    if (strcmp(topic, TOPIC_GALLONS) == 0) {
        display.setGallonsSaved((int)value);
    }

    if (strcmp(topic, TOPIC_DISTANCE) == 0) {
        display.setCleanLevel((int)value);
    }

    if (strcmp(topic, TOPIC_HEIGHT) == 0) {
        display.setCycles((int)value);
    }
}

// ------------------ SETUP -----------------------
void setup() {
    Serial.begin(115200);
    delay(300);

    display.begin();

    connectWiFi();
    display.setWiFiStatus(WiFi.status() == WL_CONNECTED);

    client.setCallback(callback);
    connectMQTT();
}

// ------------------ LOOP ------------------------
void loop() {
    // MQTT reconnect if needed
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();

    // Update display (buttons, timeout, screen refresh)
    display.update();

    // Keep WiFi status updated on Screen 4
    display.setWiFiStatus(WiFi.status() == WL_CONNECTED);
}
