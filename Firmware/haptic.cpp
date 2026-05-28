// haptic.cpp  —  DRV2605L haptic feedback profiles
//
// Waveform library 1 (ERM) effect IDs used:
//   7  – Soft Bump 100%           single,  ~30 ms  (zone left:  suppress OFF)
//   50 – Buzz 4                   single,  ~20 ms  (zone mid:   dead-band)
//   48 – Buzz 2                   double,  ~80 ms  (zone right: suppress ON)
//   51 – Buzz 5                   triple,  ~60 ms  (suppress episode done)
//
// NOTE — Brownout risk: Strong Click (effect 1) at 100% was found to draw
// enough inrush current to droop VCC below the ATmega328P BOD threshold,
// causing an infinite reset loop.  All profiles here use soft/buzz effects
// which draw much less peak current.  If you add a 470–1000 µF bulk cap
// across the motor supply rail, stronger effects become safe too.

#include "haptic.h"
#include <Adafruit_DRV2605.h>

// The global handle is owned by firmware.ino.
extern Adafruit_DRV2605 g_drv;

static bool s_ok = false;   // false until haptic_init(true) is called

// ── Module init ──────────────────────────────────────────────────
void haptic_init(bool drv_ok) {
    s_ok = drv_ok;
}

// ── Internal: load an effect sequence and fire it ────────────────
// effects : array of DRV2605 effect IDs (1–123)
// count   : number of entries (max 7, since slot 0–6 + end marker)
// The function writes a 0 end-of-sequence marker after the last effect.
static void play(const uint8_t *effects, uint8_t count) {
    if (!s_ok) return;
    for (uint8_t i = 0; i < count; i++) {
        g_drv.setWaveform(i, effects[i]);
    }
    g_drv.setWaveform(count, 0);  // end-of-sequence marker
    g_drv.go();
}

// ── Profile: left zone (suppress OFF) ────────────────────────────
// One soft bump — a gentle single "clunk" confirming suppression is off.
void haptic_zone_left() {
    static const uint8_t e[] = { 7 };   // Soft Bump 100%  ~30 ms
    play(e, 1);
}

// ── Profile: middle zone (dead-band) ─────────────────────────────
// One brief tick — barely perceptible, confirms you're in the no-change zone.
void haptic_zone_middle() {
    static const uint8_t e[] = { 50 };  // Buzz 4  ~20 ms
    play(e, 1);
}

// ── Profile: right zone (suppress ON) ────────────────────────────
// Two medium buzzes — clearly distinct from the single-event profiles,
// signalling that suppression is now active.
void haptic_zone_right() {
    static const uint8_t e[] = { 48, 48 };  // Buzz 2 × 2  ~80 ms total
    play(e, 2);
}

// ── Profile: tremor episode complete ─────────────────────────────
// Three light pulses — celebratory triple-tap once the servo has detached
// after the tremor window has passed.
void haptic_suppress_done() {
    static const uint8_t e[] = { 51, 51, 51 };  // Buzz 5 × 3  ~60 ms total
    play(e, 3);
}
