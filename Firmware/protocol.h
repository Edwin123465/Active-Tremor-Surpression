#pragma once

// =============================================================
//  protocol.h  –  Serial protocol constants
//  115200 baud, 8N1, line-oriented, \n-terminated, ASCII CSV
//  Full human-readable spec: shared/PROTOCOL.md
// =============================================================

#define PROTO_VERSION     1
#define SERIAL_BAUD       115200

// --- Packet type prefixes --------------------------------------
#define PKT_TELEMETRY     'T'
#define PKT_EVENT         'E'
#define PKT_COMMAND       'C'

// --- Event levels ----------------------------------------------
#define EVT_INFO          "INFO"
#define EVT_WARN          "WARN"
#define EVT_ERR           "ERR"

// --- Event keys ------------------------------------------------
#define EVT_KEY_READY     "ready"
#define EVT_KEY_PONG      "pong"
#define EVT_KEY_STATE     "state"
#define EVT_KEY_USB_ISO   "usb_iso_unknown"
#define EVT_KEY_I2C_MISS  "i2c_missing"
#define EVT_KEY_IMU_TOUT  "imu_timeout"

// --- Command tokens --------------------------------------------
#define CMD_PING          "PING"
#define CMD_CALIB_START   "CALIB_START"
#define CMD_CALIB_SAVE    "CALIB_SAVE"
#define CMD_CALIB_ABORT   "CALIB_ABORT"
#define CMD_SET_GAIN      "SET_GAIN"
#define CMD_SET_BAND      "SET_BAND"
#define CMD_SET_THRESH    "SET_THRESH"
#define CMD_SUPPRESS_ON   "SUPPRESS_ON"
#define CMD_SUPPRESS_OFF  "SUPPRESS_OFF"
#define CMD_ESTOP         "ESTOP"
#define CMD_RESET         "RESET"

// --- Telemetry rate --------------------------------------------
#define TELEM_HZ          100
#define TELEM_INTERVAL_US (1000000UL / TELEM_HZ)

// --- FSM state enum values (sent in telemetry 'state' field) ---
#define STATE_BOOT        0
#define STATE_SELFTEST    1
#define STATE_IDLE        2
#define STATE_CALIB       3
#define STATE_STREAM      4
#define STATE_SUPPRESS    5
#define STATE_FAULT       6
