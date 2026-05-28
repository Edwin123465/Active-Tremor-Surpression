#pragma once
// calib.h  –  Calibration data struct + EEPROM load/save
// ATmega328P: 1024 bytes EEPROM, address 0 onwards.

#include <Arduino.h>

// ── Calibration data (36 bytes total, fits easily in 1KB EEPROM) ──
struct CalibData {
    // IMU gyro bias (forearm), integer units matching protocol (deg/s×10)
    int16_t fa_gx_bias, fa_gy_bias, fa_gz_bias;
    // IMU gyro bias (upper arm)
    int16_t ua_gx_bias, ua_gy_bias, ua_gz_bias;
    // EMG baseline (ADC counts, 0–1023)
    uint16_t emg_baseline;
    // Servo feedback range (ADC counts, 0–1023)
    uint16_t servo_min, servo_max;
    // Potentiometer midpoint
    uint16_t pot_mid;
    // Tremor detection thresholds (set during Stage 5)
    uint16_t trem_threshold;  // tremor_amp×10 threshold
    uint16_t emg_threshold;   // EMG ADC threshold for voluntary detection
    // Magic number to detect uninitialised EEPROM
    uint16_t magic;
};

#define CALIB_MAGIC   0xCAB1   // arbitrary sentinel
#define CALIB_ADDR    0        // EEPROM start address

// Default safe values used before calibration
void calib_defaults(CalibData &c);

// Load from EEPROM; returns false and loads defaults if not calibrated yet
bool calib_load(CalibData &c);

// Save to EEPROM
void calib_save(const CalibData &c);

// Global calibration state
extern CalibData g_calib;
extern bool      g_calib_valid;   // true once calib_load succeeds or calib_save called
