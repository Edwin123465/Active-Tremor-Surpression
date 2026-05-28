// control.cpp  –  Stage 6/7: engage-on-tremor servo control
//
// Behaviour:
//   REST / VOLUNTARY  → servo DETACHED (arm moves completely freely)
//   TREMOR (onset)    → servo ATTACHES at arm's current position, holds it
//   TREMOR (sustained)→ servo holds captured position; gain adjusts strength
//   TREMOR → REST     → servo stays attached for DETACH_DELAY_MS (avoids
//                        chattering if tremor is intermittent), then detaches
//   SUPPRESS_OFF/ESTOP → control_release() → neutral + detach immediately
//
// The servo only ever fights the arm when tremor is actively detected.
// During all other phases the arm moves with zero resistance.

#include <Arduino.h>
#include "control.h"
#include "haptic.h"
#include "pins.h"    // PIN_SERVO_FB

// ── Tuning constants ─────────────────────────────────────────────
// Ramp rate when driving toward the hold target: µs per 50 Hz tick
// 10 µs/tick × 50 Hz = 500 µs/s ≈ 28°/s — smooth but responsive
static const uint8_t HOLD_RATE_US    = 10;

// How long (ms) to keep servo attached after tremor stops.
// Prevents rapid attach/detach cycling if tremor is intermittent.
static const uint16_t DETACH_DELAY_MS = 400;

// ── State ────────────────────────────────────────────────────────
static Servo    *s_servo        = nullptr;
static uint16_t  s_min_us       = 1000;
static uint16_t  s_max_us       = 2000;
static uint16_t  s_neutral_us   = 1500;
static uint16_t  s_cmd_us       = 1500;
static uint16_t  s_target_us    = 1500;
static uint8_t   s_prev_cls     = 0;
static uint32_t  s_tremor_end_ms = 0;   // time tremor last stopped

// ── Helpers ──────────────────────────────────────────────────────
static uint16_t clamp_us(uint16_t v) {
    if (v < s_min_us) return s_min_us;
    if (v > s_max_us) return s_max_us;
    return v;
}

static uint16_t step_toward(uint16_t val, uint16_t goal, uint8_t rate) {
    if (val < goal) { uint16_t d = goal - val; return val + (d < rate ? d : rate); }
    if (val > goal) { uint16_t d = val - goal; return val - (d < rate ? d : rate); }
    return val;
}

// ── Public API ───────────────────────────────────────────────────
void control_init(Servo *servo, const CalibData *calib) {
    s_servo      = servo;
    s_min_us     = 1000 + (uint16_t)((uint32_t)calib->servo_min * 1000UL / 1023);
    s_max_us     = 1000 + (uint16_t)((uint32_t)calib->servo_max * 1000UL / 1023);
    s_neutral_us = (s_min_us + s_max_us) / 2;
    s_cmd_us     = s_neutral_us;
    s_target_us  = s_neutral_us;
    s_prev_cls   = 0;
    s_tremor_end_ms = 0;
    // Do NOT attach here — servo only attaches when tremor is detected.
}

uint16_t control_update(uint8_t classifier, float /*tremor_amp*/, float gain_01) {
    if (!s_servo) return s_cmd_us;

    // Current arm position from servo feedback pot (A3)
    uint16_t fb_us = clamp_us(
        1000 + (uint16_t)((uint32_t)analogRead(PIN_SERVO_FB) * 1000UL / 1023));

    if (classifier == 2) {
        // ── TREMOR ─────────────────────────────────────────────────────
        s_tremor_end_ms = 0;   // clear release timer

        if (!s_servo->attached()) {
            // Engage: attach from current arm position to avoid a jerk
            s_target_us = fb_us;
            s_cmd_us    = fb_us;
            s_servo->attach(PIN_SERVO_PWM);
        } else if (s_prev_cls != 2) {
            // First tick back in tremor after a brief gap — re-capture
            s_target_us = fb_us;
        }

        // Blend: gain=0 → neutral (no force), gain=1 → full hold at capture
        uint16_t goal = clamp_us((uint16_t)(
            (float)s_neutral_us * (1.0f - gain_01) +
            (float)s_target_us  * gain_01));
        s_cmd_us = step_toward(s_cmd_us, goal, HOLD_RATE_US);
        s_servo->writeMicroseconds(s_cmd_us);

    } else {
        // ── REST or VOLUNTARY ───────────────────────────────────────────
        // Keep target fresh so it's correct when tremor re-engages
        s_target_us = fb_us;
        s_cmd_us    = fb_us;

        if (s_servo->attached()) {
            // Start release countdown on first non-tremor tick
            if (s_tremor_end_ms == 0) s_tremor_end_ms = millis();

            if (millis() - s_tremor_end_ms >= DETACH_DELAY_MS) {
                s_servo->detach();     // arm is now completely free
                s_tremor_end_ms = 0;
                haptic_suppress_done(); // triple-pulse: episode complete
            }
        }
    }

    s_prev_cls = classifier;
    return s_cmd_us;
}

void control_release() {
    // Immediate safe stop: neutral then detach.
    // Called by SUPPRESS_OFF, ESTOP, and IMU timeout.
    if (s_servo && s_servo->attached()) {
        s_servo->writeMicroseconds(s_neutral_us);
        delay(50);          // give servo ~50 ms to reach neutral before detach
        s_servo->detach();
    }
    s_cmd_us       = s_neutral_us;
    s_target_us    = s_neutral_us;
    s_tremor_end_ms = 0;
    s_prev_cls     = 0;
}

uint16_t control_servo_us()   { return s_cmd_us;     }
uint16_t control_neutral_us() { return s_neutral_us; }
