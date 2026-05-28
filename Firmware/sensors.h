#pragma once
// sensors.h  –  IMU / EMG / pot / servo-angle read
// Implemented from Stage 2 onwards.

#include <Arduino.h>

struct ImuData {
    int16_t ax, ay, az;  // accel raw (units: g * 1000)
    int16_t gx, gy, gz;  // gyro  raw (units: deg/s * 10)
};

struct SensorFrame {
    uint32_t ms;
    uint16_t pot;        // 0-1023
    uint16_t emg;        // 0-1023
    float    servo_deg;  // 0-180
    ImuData  forearm;
    ImuData  upper;
};

void sensors_init();
bool sensors_read(SensorFrame &out);
