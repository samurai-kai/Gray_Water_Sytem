#include "Ultrasonic.h"

Ultrasonic::Ultrasonic(uint8_t trigPin, uint8_t echoPin)
    : _trigPin(trigPin), _echoPin(echoPin) {}

void Ultrasonic::begin() {
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    digitalWrite(_trigPin, LOW);
}

float Ultrasonic::_readOnce(uint32_t timeoutMicros) {
    // Ensure trigger low
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    // 10 µs trigger pulse
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // Read echo pulse width
    unsigned long duration = pulseIn(_echoPin, HIGH, timeoutMicros);
    if (duration == 0) {
        return -1.0f;  // timeout
    }

    // Convert time → distance in cm
    // Speed of sound: 343 m/s → 0.0343 cm/us
    float distanceCm = (duration * 0.0343f) / 2.0f;
    return distanceCm;
}

float Ultrasonic::readCm(uint32_t timeoutMicros) {
    return _readOnce(timeoutMicros);
}

float Ultrasonic::readIn(uint32_t timeoutMicros) {
    float cm = _readOnce(timeoutMicros);
    if (cm < 0) return -1;
    return cm / 2.54f;
}

float Ultrasonic::readMedianCm(uint8_t samples) {
    if (samples == 0) return -1.0f;
    if (samples > 16) samples = 16;  // safety cap

    float readings[16];
    uint8_t count = 0;

    // Collect valid readings
    for (uint8_t i = 0; i < samples; i++) {
        float d = _readOnce(30000);

        // Reject invalid / noisy readings
        if (d > 2.0f && d < 400.0f) {
            readings[count++] = d;
        }

        delay(30); // slight delay between pings
    }

    if (count == 0) return -1.0f; // no valid readings

    // Sort for median
    for (uint8_t i = 1; i < count; i++) {
        float key = readings[i];
        int j = i - 1;
        while (j >= 0 && readings[j] > key) {
            readings[j + 1] = readings[j];
            j--;
        }
        readings[j + 1] = key;
    }

    // Return median
    if (count % 2 == 1) {
        return readings[count / 2];
    } else {
        return (readings[count / 2 - 1] + readings[count / 2]) * 0.5f;
    }
}
