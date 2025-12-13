#include "Tank.h"

Tank::Tank(float heightIn, float diameterIn, float capacity)
    : _height(heightIn), _diameter(diameterIn), _capacity(capacity) {}

float Tank::getVolumeCubicInches(float waterHeightIn) {
    // Clamp height to prevent invalid input
    waterHeightIn = constrain(waterHeightIn, 0.0f, _height);

    //volume for a cylinder equation: V = πr^2h
    float r = _radius();
    float volume = PI * r * r * waterHeightIn;  // cubic inches
    return volume;
}

float Tank::getGallons(float waterHeightIn) {
    float cubicInches = getVolumeCubicInches(waterHeightIn);
    return cubicInches / 231.0f; // 1 gallon = 231 cubic inches
}

float Tank::getPercentFull(float waterHeightIn) {
    waterHeightIn = constrain(waterHeightIn, 0.0f, _height);
    return (waterHeightIn / _height) * 100.0f;
}
