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
            state = PUMP_SENSE;
            break;

        // ---------------------------------------------------------
        case PUMP_SENSE:
        {
            // BELOW LOWER → pump must stay off, can't run
            if (thisPct <= lower) {
                stop();
                state = PUMP_SENSE;
                return;
            }

            // -------------------------------------------------
            // CLEAN PUMP LOGIC
            // -------------------------------------------------
            if (!dirtyMode)
            {
                // Normal automatic mode:
                if (thisPct >= upper) {
                    start();
                    state = PUMP_RUN;
                }

                // Respond to dirty-pump request:
                if (requestCleanPump)
                {
                    // can only help if clean tank has water
                    if (thisPct > lower)
                    {
                        start();
                        state = PUMP_RUN;
                    }

                    // Either way → request handled
                    requestCleanPump = false;
                }
            }

            // -------------------------------------------------
            // DIRTY PUMP LOGIC
            // -------------------------------------------------
            else
            {
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
                // clean pump should not be here
                state = PUMP_SENSE;
                break;
            }

            // Dirty→Clean transfer conditions:
            float requiredSpace   = thisGallons;                         // dirty tank volume to move
            float availableSpace  = (otherCapacity - otherGallons);      // free space in clean tank

            // Clean tank has enough room, start pumping
            if (availableSpace >= requiredSpace) {
                start();
                state = PUMP_RUN;
            }
            else {
                // Not enough room, request clean pump to create space
                requestCleanPump = true;
            }

            // Dirty tank dropped too low, abort WAIT
            if (thisPct <= lower) {
                stop();
                state = PUMP_SENSE;
            }
        }
        break;

        // ---------------------------------------------------------
        case PUMP_RUN:
        {
            // Stop once tank hits lower limit
            if (thisPct <= lower) {
                stop();
                state = PUMP_SENSE;
            }
        }
        break;
    }
}

