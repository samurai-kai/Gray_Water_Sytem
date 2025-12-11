#include <Arduino.h>

#define TRIG_PIN 5
#define ECHO_PIN 18

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.println("Ultrasonic test starting...");
}

float readDistanceCm() {
    // Ensure trigger is low
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Send 10 µs trigger pulse
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read echo time (timeout = 30ms)
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) {
        return -1;  // timeout / no reading
    }

    // Convert time to distance
    // Speed of sound 343 m/s = 0.0343 cm/µs
    float distance = duration * 0.0343 / 2.0;
    return distance;
}

void loop() {
    float d = readDistanceCm();

    if (d < 0) {
        Serial.println("No echo (timeout)");
    } else {
        Serial.print("Distance: ");
        Serial.print(d);
        Serial.println(" cm");
    }

    delay(500);
}
