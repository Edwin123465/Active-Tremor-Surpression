#pragma once
// haptic.h  —  DRV2605L haptic feedback profiles
//
// Four distinct events, each with a different waveform:
//   haptic_zone_left()     – pot entered left zone  → suppress OFF
//   haptic_zone_middle()   – pot entered dead-band  → no state change
//   haptic_zone_right()    – pot entered right zone → suppress ON
//   haptic_suppress_done() – tremor episode ended, servo just detached
//
// Call haptic_init(drv_ok) once from setup() after drv_init_and_click().

#include <Arduino.h>

// Initialise module.  drv_ok must be the return value of g_drv.begin().
// When false every subsequent call is a fast no-op (no I²C traffic).
void haptic_init(bool drv_ok);

// ── Pot zone entry ─────────────────────────────────────────────────
void haptic_zone_left();    // single soft bump   (~30 ms, subtle)
void haptic_zone_middle();  // single brief tick  (~20 ms, very subtle)
void haptic_zone_right();   // double medium buzz (~80 ms, noticeable)

// ── Tremor episode complete ────────────────────────────────────────
void haptic_suppress_done(); // triple light pulse (~60 ms)
