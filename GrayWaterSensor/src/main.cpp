#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "Ultrasonic.h"
#include "Tank.h"
#include "config_secret.h"


// ------------------ USER SETTINGS ------------------
const char* MQTT_SERVER = "192.168.1.87";  // Your MQTT broker IP
const int   MQTT_PORT   = 1883;

// Hardware
Ultrasonic sensor(5, 18);  // TRIG, ECHO pins
Tank tank(12, 4, 1);       // height, diameter, capacity (gallons)
// ---------------------------------------------------

// WiFi + MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// ---------------------------------------------------
// Connect to WiFi
void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
}

// Connect to MQTT broker
void connectMQTT() {
    client.setServer(MQTT_SERVER, MQTT_PORT);

    Serial.print("Connecting to MQTT...");
    while (!client.connected()) {
        if (client.connect("graywater_sensor", MQTT_USER, MQTT_PASS)) {
            Serial.println("connected!");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 2s...");
            delay(2000);
        }
    }
}
// ---------------------------------------------------

void setup() {
    Serial.begin(115200);
    sensor.begin();

    connectWiFi();
    connectMQTT();
}

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();

    float distanceIn = sensor.readIn();

    if (distanceIn < 0) {
        Serial.println("No echo");
        delay(500);
        return;
    }

    // Convert ultrasonic distance to water height
    float waterHeight = tank.getTankHeight() - distanceIn;
    if (waterHeight < 0) waterHeight = 0;

    // Calculate tank values
    float gallons = tank.getGallons(waterHeight);
    float percent = tank.getPercentFull(waterHeight);

    // Publish MQTT data
    client.publish("graywater/distance_in", String(distanceIn).c_str());
    client.publish("graywater/water_height_in", String(waterHeight).c_str());
    client.publish("graywater/gallons",        String(gallons).c_str());
    client.publish("graywater/percent",        String(percent).c_str());

    Serial.println("Published tank data to MQTT!");

    delay(2000);
}
