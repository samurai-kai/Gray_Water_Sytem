#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

enum PumpState {
    PUMP_INIT = 0,
    PUMP_SENSE,
    PUMP_WAIT,   // only used by dirty pump
    PUMP_RUN
};

class Pump {
public:
    Pump(uint8_t relayPin, float upperThres, float lowerThres, bool isDirtyPump = false);

    void begin();
    
    // Main FSM update function
    void update(
        float thisTankPercent,
        float thisTankGallons,
        float otherTankGallons,
        float otherTankCapacity
    );

    bool isRunning() const { return running; }
    PumpState getState() const { return state; }

private:
    uint8_t pin;
    float upper;
    float lower;
    bool running;
    bool dirtyMode; // dirty pump uses WAIT state

    PumpState state;

    void start();
    void stop();
    void applyState();
};

#endif
