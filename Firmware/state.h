#pragma once
// state.h  –  FSM: BOOT / SELFTEST / IDLE / CALIB / STREAM / SUPPRESS / FAULT
// Implemented progressively from Stage 1 onwards.

#include <Arduino.h>
#include "protocol.h"

typedef uint8_t AppState;

// Current FSM state (use STATE_* constants from protocol.h)
extern AppState g_state;

void state_transition(AppState next);
