// state.cpp  –  FSM helpers
#include "state.h"
#include "protocol.h"

AppState g_state = STATE_BOOT;

void state_transition(AppState next) {
    g_state = next;
    // Event log emitted by caller (firmware.ino) so Serial is in scope.
}
