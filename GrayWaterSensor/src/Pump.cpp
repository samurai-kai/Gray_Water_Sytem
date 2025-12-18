#include "Pump.h"

Pump::Pump(uint8_t relayPin, float upperThres, float lowerThres, bool isDirtyPump)
    : pin(relayPin), upper(upperThres), lower(lowerThres), dirtyMode(isDirtyPump),
      running(false), state(PUMP_INIT)
{
}

void Pump::begin() {
    pinMode(pin, OUTPUT);
    stop();         // ensure pump is off
}

void Pump::applyState() {
    digitalWrite(pin, running ? HIGH : LOW);
}

void Pump::start() {
    running = true;
    applyState();
}

void Pump::stop() {
    running = false;
    applyState();
}

void Pump::update(
    float thisPct,
    float thisGallons,
    float otherGallons,
    float otherCapacity
) {
    switch (state)
    {
        // ---------------------------------------------------------
        case PUMP_INIT:
            // Immediately transition into sensing state
            state = PUMP_SENSE;
            break;

        // ---------------------------------------------------------
        case PUMP_SENSE:
        {
            // If the tank is below lower threshold, pump stops forever until refilled
            if (thisPct <= lower) {
                stop();
                return;
            }

            // CLEAN PUMP LOGIC (simple SENSE→RUN)
            if (!dirtyMode) {
                if (thisPct >= upper) {
                    start();
                    state = PUMP_RUN;
                }
            }

            // DIRTY PUMP LOGIC (SENSE→WAIT)
            else {
                if (thisPct >= upper) {
                    state = PUMP_WAIT;
                }
            }
        }
        break;

        // ---------------------------------------------------------
        case PUMP_WAIT:
        {
            if (!dirtyMode) {
                // Should never be here for clean pump
                state = PUMP_SENSE;
                break;
            }

            // Dirty pump waits until clean tank has enough space
            //
            // We want to ensure:
            //   clean_free_space >= dirty_gallons_to_move
            //
            float requiredSpace = thisGallons;               // dirty gallons that would be pumped
            float availableSpace = (otherCapacity - otherGallons);

            if (availableSpace >= requiredSpace) {
                start();
                state = PUMP_RUN;
            }

            // Cancel wait if dirty tank falls too low
            if (thisPct <= lower) {
                stop();
                state = PUMP_SENSE;
            }
        }
        break;

        // ---------------------------------------------------------
        case PUMP_RUN:
        {
            // Pump continues running until tank drops to lower threshold
            if (thisPct <= lower) {
                stop();
                state = PUMP_SENSE;
            }
        }
        break;
    }
}
