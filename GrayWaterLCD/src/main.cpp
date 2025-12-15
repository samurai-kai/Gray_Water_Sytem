#include <Arduino.h>
#include "Display.h"

// BOOT button on GPIO0, timeout = 10 seconds
Display display(0, 10000);

void setup() {
    Serial.begin(115200);
    delay(200);
    display.begin();
}

void loop() {
    display.update();

    // other program logic goes here
}
