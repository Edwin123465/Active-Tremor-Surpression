// calib.cpp  –  Calibration EEPROM load/save
#include "calib.h"
#include <EEPROM.h>

CalibData g_calib;
bool      g_calib_valid = false;

void calib_defaults(CalibData &c) {
    c.fa_gx_bias = c.fa_gy_bias = c.fa_gz_bias = 0;
    c.ua_gx_bias = c.ua_gy_bias = c.ua_gz_bias = 0;
    c.emg_baseline   = 50;
    c.servo_min      = 100;
    c.servo_max      = 900;
    c.pot_mid        = 512;
    c.trem_threshold = 200;   // tremor_amp×10 > 20.0 deg/s triggers detection
    c.emg_threshold  = 200;   // EMG ADC > 200 = voluntary contraction
    c.magic          = 0;     // deliberately invalid
}

bool calib_load(CalibData &c) {
    EEPROM.get(CALIB_ADDR, c);
    if (c.magic != CALIB_MAGIC) {
        calib_defaults(c);
        return false;
    }
    return true;
}

void calib_save(const CalibData &c) {
    EEPROM.put(CALIB_ADDR, c);
}
