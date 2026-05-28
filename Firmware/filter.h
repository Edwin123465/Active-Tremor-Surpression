#pragma once
// filter.h  –  Tremor bandpass + envelope
// Stage 4: 2nd-order Butterworth bandpass, Direct Form II biquad
// Default band: 2–8 Hz at 100 Hz sample rate
// Output: rectified + low-pass envelope (tremor_amp in deg/s)

#include <Arduino.h>

// ── Biquad section (Direct Form II) ────────────────────────────
// One section: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
//                           - a1*y[n-1] - a2*y[n-2]
struct Biquad {
    float b0, b1, b2;   // feed-forward coefficients
    float a1, a2;       // feed-back coefficients (a0 = 1 normalised)
    float w1, w2;       // delay state
};

// Run one sample through a biquad section, returns output
float biquad_run(Biquad &b, float x);

// Reset biquad state to zero
void biquad_reset(Biquad &b);

// ── Filter module ───────────────────────────────────────────────
// Call filter_init() once (or when band changes).
// Call filter_update() at exactly 100 Hz with forearm gyro magnitude.
// Returns the tremor amplitude envelope in the same units as input.

void  filter_init(float low_hz, float high_hz);
float filter_update(float gyro_mag);   // gyro_mag in deg/s (float)

// Current band settings (readable by firmware for telemetry)
extern float g_filter_low_hz;
extern float g_filter_high_hz;
