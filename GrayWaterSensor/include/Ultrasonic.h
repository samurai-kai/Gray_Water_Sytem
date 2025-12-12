#ifndef ULTARSONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

class Ultrasonic {
public:
  Ultrasonic(uint8_t trigPin, uint8_t echoPin);

  void begin();

  float readCm(uint32_t timeoutMicros = 30000);
  float readIn(uint32_t timeoutMicros = 30000);
  float readMedianCm(uint8_t samples = 5);

private:
  uint8_t _trigPin;
  uint8_t _echoPin;

  float _readOnce(uint32_t timeoutMicros);
};

#endif