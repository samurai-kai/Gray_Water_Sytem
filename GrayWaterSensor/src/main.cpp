// #include <Arduino.h>

// #define TRIG_PIN 5
// #define ECHO_PIN 18
// #define tank_height 33.5 // in
// #define tank_diameter 22.5 // in 

// void setup() {
//     Serial.begin(115200);
//     delay(1000);

//     pinMode(TRIG_PIN, OUTPUT);
//     pinMode(ECHO_PIN, INPUT);

//     Serial.println("Ultrasonic test starting...");
// }

// float readDistanceCm() {
//     // Ensure trigger is low
//     digitalWrite(TRIG_PIN, LOW);
//     delayMicroseconds(2);

//     // Send 10 µs trigger pulse
//     digitalWrite(TRIG_PIN, HIGH);
//     delayMicroseconds(10);
//     digitalWrite(TRIG_PIN, LOW);

//     // Read echo time (timeout = 30ms)
//     long duration = pulseIn(ECHO_PIN, HIGH, 30000);

//     if (duration == 0) {
//         return -1;  // timeout / no reading
//     }

//     // Convert time to distance
//     // Speed of sound 343 m/s = 0.0343 cm/µs (2.54 cm/inch)
//     float distance = duration * 0.0343 / 2.0 / 2.54;
//     return distance;
// }

// float calculateVolume(float water_height) {
//     // Calculate the volume of water in the tank using the formula for the volume of a cylinder
//     float radius = tank_diameter / 2.0;
//     float volume = PI * pow(radius, 2) * water_height;
//     return volume;
// }

// void loop() {
//     float d = readDistanceCm();

//     if (d < 0) {
//         Serial.println("nan");
//     } else {
//         Serial.print("Distance: ");
//         Serial.print(d);
//         Serial.println(" inches");
//     }

//     // Calculate the volume of water in the tank
//     float water_height = tank_height - d; // Height of water in the tank
//     Serial.print("Water height: ");
//     Serial.print(water_height);
//     Serial.println(" inches");

//     float volume = calculateVolume(water_height);
//     Serial.print("Volume: ");   // Print the volume in inches^3 
//     Serial.print(volume);
//     Serial.println(" inches^3");

//     delay(500);
// }

#include <Arduino.h>
#include "Ultrasonic.h"

Ultrasonic sensor(5, 18);

void setup() {
    Serial.begin(115200);
    sensor.begin();
}

void loop() {
    float inches = sensor.readIn();
    
    if (inches < 0) {
        Serial.println("No echo");
    } else {
        Serial.print("Distance: ");
        Serial.print(inches, 2);
        Serial.println(" in");
    }

    delay(500);
}
