#pragma once

// =============================================================
//  pins.h  –  Single source of truth for all pin assignments
//  Board: Arduino Nano (ATmega328P, 16 MHz, 2KB RAM, 1KB EEPROM)
//  Do NOT hard-code pin numbers anywhere else.
// =============================================================

// --- Power / I²C -----------------------------------------------
// A4 = SDA, A5 = SCL (shared bus: MPU-6050 x2 + DRV2605L)

// --- Digital outputs -------------------------------------------
#define PIN_SERVO_PWM    3   // D3 (PWM) → S1213 servo orange wire

// --- Digital inputs --------------------------------------------
#define PIN_DRV_INT      2   // D2 → DRV2605L INT (haptic-done, active-low)

// --- Analog inputs ---------------------------------------------
#define PIN_POT          A0  // Potentiometer wiper — dual role:
                             //   ADC > 560 (right half, >54 %) → SUPPRESS active
                             //   ADC < 460 (left  half, <45 %) → SUPPRESS off
                             //   460–560 centre dead-band: no change
                             //   Right-half position = suppression gain 0–1
#define PIN_EMG          A1  // MyoWare SIG – EMG envelope 0–5 V
#define PIN_SERVO_FB     A3  // Servo white wire – angle feedback 0–1023

// --- I²C addresses ---------------------------------------------
#define I2C_IMU_UPPER    0x68  // MPU-6050 upper arm  (AD0 = GND)
#define I2C_IMU_FOREARM  0x69  // MPU-6050 forearm    (AD0 = 3.3 V)
#define I2C_DRV2605      0x5A  // DRV2605L haptic driver
