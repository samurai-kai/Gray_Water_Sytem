#ifndef TANK_H
#define TANK_H

#include <Arduino.h>

class Tank {
public:
    Tank(float heightIn, float diameterIn, float capacity);

    float getVolumeCubicInches(float waterHeightIn);
    float getGallons(float waterHeightIn);
    float getPercentFull(float waterHeightIn);

    float getTankHeight() const { return _height; }
    float getTankDiameter() const { return _diameter; }
    float getCapacity() const { return _capacity; }

private:
    float _height;    // inches
    float _diameter;  // inches
    float _capacity;   // gallons

    float _radius() const { return _diameter / 2.0; }
};

#endif
