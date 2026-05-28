#pragma once
// ================================================================
//  mpu6050.h  –  Lightweight MPU-6050 driver (zero heap allocation)
//
//  Replaces Adafruit_MPU6050 to recover ~400 bytes of heap on the
//  ATmega328P (2 KB RAM).  Uses direct Wire burst-reads; no dynamic
//  memory allocation.
//
//  Config: ±500 deg/s gyro, ±8 g accel, 21 Hz DLPF.
//  Output units match the serial protocol directly:
//    ax/ay/az → g × 1000   (int16)
//    gx/gy/gz → deg/s × 10 (int16)
// ================================================================

#include <Wire.h>
#include <stdint.h>

struct Mpu6050Raw {
    int16_t ax, ay, az;   // ±8 g    → g × 1000
    int16_t gx, gy, gz;   // ±500°/s → deg/s × 10
};

// Initialise the MPU-6050 at I²C address `addr`.
// Returns true if the device ACKs and configuration succeeds.
bool mpu6050_init(uint8_t addr);

// Burst-read accel + gyro (14 bytes) and convert to protocol units.
// Returns true if the I²C transaction completed without error.
// On failure `out` is left unchanged.
bool mpu6050_read(uint8_t addr, Mpu6050Raw &out);
