// filter.cpp  –  Stage 4: Butterworth bandpass + envelope
//
// Design: two cascaded 2nd-order Butterworth biquads
//   Section 1: highpass at low_hz  (removes DC + slow drift)
//   Section 2: lowpass  at high_hz (removes high-freq noise)
// Together they form a bandpass covering [low_hz, high_hz].
//
// Coefficients computed via bilinear transform at Fs = 100 Hz.
// Envelope: full-wave rectify + 1st-order IIR lowpass (tau ~50 ms).
//
// ATmega328P note: floats are 32-bit IEEE 754, computed in software.
// At 100 Hz this runs ~200 µs worst case — well within the 10 ms budget.

#include "filter.h"
#include <math.h>

float g_filter_low_hz  = 2.0f;
float g_filter_high_hz = 8.0f;

static Biquad s_hp;   // highpass section
static Biquad s_lp;   // lowpass section

// Envelope IIR state
static float s_env = 0.0f;
static float s_env_alpha = 0.0f;   // computed in filter_init

// ── Bilinear-transform 2nd-order Butterworth lowpass ───────────
// Normalised cutoff wc = 2*pi*fc/Fs (digital frequency in rad/sample)
// Returns biquad coefficients in b (normalised, a0=1).
static void design_lowpass(Biquad &b, float fc, float fs) {
    float wc  = 2.0f * M_PI * fc / fs;
    float K   = tanf(wc * 0.5f);
    float K2  = K * K;
    float sq2 = 1.41421356f;   // sqrt(2) for Butterworth Q
    float norm = 1.0f / (1.0f + sq2 * K + K2);

    b.b0 =  K2 * norm;
    b.b1 =  2.0f * K2 * norm;
    b.b2 =  b.b0;
    b.a1 =  2.0f * (K2 - 1.0f) * norm;
    b.a2 =  (1.0f - sq2 * K + K2) * norm;
    b.w1 = b.w2 = 0.0f;
}

// ── Bilinear-transform 2nd-order Butterworth highpass ──────────
static void design_highpass(Biquad &b, float fc, float fs) {
    float wc  = 2.0f * M_PI * fc / fs;
    float K   = tanf(wc * 0.5f);
    float K2  = K * K;
    float sq2 = 1.41421356f;
    float norm = 1.0f / (1.0f + sq2 * K + K2);

    b.b0 =  norm;
    b.b1 = -2.0f * norm;
    b.b2 =  norm;
    b.a1 =  2.0f * (K2 - 1.0f) * norm;
    b.a2 =  (1.0f - sq2 * K + K2) * norm;
    b.w1 = b.w2 = 0.0f;
}

// ── Biquad runner (Direct Form II transposed) ───────────────────
float biquad_run(Biquad &b, float x) {
    float y = b.b0 * x + b.w1;
    b.w1    = b.b1 * x - b.a1 * y + b.w2;
    b.w2    = b.b2 * x - b.a2 * y;
    return y;
}

void biquad_reset(Biquad &b) {
    b.w1 = b.w2 = 0.0f;
}

// ── Public API ──────────────────────────────────────────────────
void filter_init(float low_hz, float high_hz) {
    const float FS = 100.0f;

    // Clamp to safe ranges
    if (low_hz  < 0.5f) low_hz  = 0.5f;
    if (high_hz > 45.0f) high_hz = 45.0f;
    if (high_hz <= low_hz) high_hz = low_hz + 1.0f;

    g_filter_low_hz  = low_hz;
    g_filter_high_hz = high_hz;

    design_highpass(s_hp, low_hz,  FS);
    design_lowpass (s_lp, high_hz, FS);

    // Envelope IIR: alpha = dt / (tau + dt), tau = 50 ms, dt = 10 ms
    // alpha closer to 1 = faster tracking, closer to 0 = smoother
    s_env_alpha = 0.01f / (0.05f + 0.01f);  // ~0.167
    s_env = 0.0f;
}

float filter_update(float gyro_mag) {
    // 1. Bandpass: highpass then lowpass
    float bp = biquad_run(s_lp, biquad_run(s_hp, gyro_mag));

    // 2. Full-wave rectify
    float rect = fabsf(bp);

    // 3. IIR envelope follower (smoothed amplitude)
    s_env = s_env + s_env_alpha * (rect - s_env);

    return s_env;
}
