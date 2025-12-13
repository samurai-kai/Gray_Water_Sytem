#include <Arduino.h>
#include "Ultrasonic.h"
#include "Tank.h"

// --- Hardware Setup ---
Ultrasonic sensor(5, 18);  // TRIG, ECHO pins

// --- Tank Setup ---
// height (in), diameter (in), capacity (gallons)
Tank tank(12, 4, 1);

void setup() {
    Serial.begin(115200);
    sensor.begin();
}

void loop() {
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

    // --- Output ---
    Serial.print("Surface Distance: ");
    Serial.print(distanceIn, 2);
    Serial.println(" in");

    Serial.print("Water Height: ");
    Serial.print(waterHeight, 2);
    Serial.println(" in");

    Serial.print("Gallons: ");
    Serial.print(gallons, 2);
    Serial.println(" gal");

    Serial.print("Percent Full: ");
    Serial.print(percent, 1);
    Serial.println("%");

    Serial.println("------------------------");

    delay(2000);
}
