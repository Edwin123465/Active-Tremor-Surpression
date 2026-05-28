#pragma once
// control.h  –  Stage 6: servo control loop
// Call control_init() once in setup() after calibration is loaded.
// Call control_update() at 50 Hz when in STATE_SUPPRESS.

#include <Arduino.h>
#include <Servo.h>
#include "calib.h"

// Initialise module.  Pass the shared Servo object and current calibration.
// Re-call after CALIB_SAVE to update servo bounds.
void control_init(Servo *servo, const CalibData *calib);

// Run the servo control algorithm at 50 Hz (STATE_SUPPRESS only).
//   classifier : 0=rest  1=voluntary  2=tremor
//   tremor_amp : bandpass envelope, deg/s (from filter)
//   gain_01    : 0.0–1.0 (from pot or C,SET_GAIN)
// Returns the µs command that was written, or the current hold value.
uint16_t control_update(uint8_t classifier, float tremor_amp, float gain_01);

// Safely return servo to neutral and detach.  Call on SUPPRESS_OFF / ESTOP.
void control_release();

// Last servo command written (µs).  Use for telemetry / debug.
uint16_t control_servo_us();

// Calibrated neutral position (µs).
uint16_t control_neutral_us();
