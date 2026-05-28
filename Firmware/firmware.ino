// =============================================================
//  firmware.ino  –  Active Tremor-Suppression Orthosis
//  Board: Arduino Nano (ATmega328P, 2KB RAM, 32KB flash, 1KB EEPROM, 16MHz)
//
//  Stage 1 – smoke test (preserved):  I²C scan, servo sweep, DRV2605
//  Stage 2 – 100 Hz telemetry, C,PING
//  Stage 3 – calibration (CALIB_START / CALIB_SAVE / CALIB_ABORT)
//  Stage 4 – Butterworth 2–8 Hz bandpass tremor detection
//  Stage 5 – EMG gating + 3-class classifier (rest/voluntary/tremor)
//  Stage 6 – 50 Hz servo control loop; pot fully right = SUPPRESS_ON,
//             pot fully left = SUPPRESS_OFF; IMU timeout watchdog
//
//  NOTE: Adafruit_MPU6050 replaced by mpu6050.h (direct Wire reads,
//  zero heap allocation — saves ~400 bytes on the 2 KB ATmega328P).
// =============================================================

#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>
#include <Adafruit_DRV2605.h>

#include "pins.h"
#include "protocol.h"
#include "state.h"
#include "calib.h"
#include "filter.h"
#include "control.h"
#include "haptic.h"
#include "mpu6050.h"

// ---------------------------------------------------------------
// Globals
// ---------------------------------------------------------------
Servo            g_servo;
Adafruit_DRV2605 g_drv;

static bool s_imu_upper_ok   = false;
static bool s_imu_forearm_ok = false;
static bool s_drv_ok         = false;

// Serial input buffer for commands
static char    s_cmd_buf[48];
static uint8_t s_cmd_len = 0;

// ── IMU read buffers — updated every telemetry tick ─────────────
// Shared between send_telemetry() and calib_update().
// Protocol units: ax/ay/az = g×1000 (int16), gx/gy/gz = deg/s×10 (int16)
static Mpu6050Raw g_raw_fa;  // forearm IMU  (zero-initialised = safe default)
static Mpu6050Raw g_raw_ua;  // upper-arm IMU

// ── Stage 5: EMG gating enable/disable ──────────────────────────
// Disable with C,EMG_OFF if the MyoWare sensor is not fitted or
// is false-triggering VOLUNTARY when the arm is at rest.
static bool s_emg_enabled = true;

// ── Stage 5: EMG moving average (5-sample ring buffer) ──────────
#define EMG_AVG_N 5
static uint16_t s_emg_buf[EMG_AVG_N];
static uint8_t  s_emg_idx = 0;
static uint32_t s_emg_sum = 0;
static uint16_t s_emg_avg = 0;

// ── Stage 5: suppression gain (0–100 represents 0.00–1.00) ──────
static uint8_t s_gain_pct    = 60;    // default 60 %
static bool    s_gain_locked = false; // true = C,SET_GAIN override

// ── Stage 5: classifier result ───────────────────────────────────
static uint8_t s_classifier = 0;     // 0=rest 1=voluntary 2=tremor

// ── Stage 6: tremor amplitude shared with 50 Hz control tick ────
static float    s_tremor_amp_last = 0.0f;
static uint32_t s_last_imu_ms    = 0;
static const uint16_t IMU_TIMEOUT_MS = 200;

// ── Potentiometer suppress toggle ───────────────────────────────
// Simple left/right split with a small centre dead-band:
//   pot > POT_ON_THRESH  (right half, ~54–100 %) → SUPPRESS active
//   pot < POT_OFF_THRESH (left  half,   0–46 %) → SUPPRESS off
//   Dead-band 46–54 %: no state change (prevents oscillation at midpoint)
//
// How to use:
//   Turn pot past the MIDPOINT to the right → suppression on.
//   Adjust gain with the right half of travel (more right = stronger hold).
//   Turn pot back to the left of centre → suppression off.
#define POT_ON_THRESH   560   // > 54.7 % → enter SUPPRESS
#define POT_OFF_THRESH  460   // < 45.0 % → exit  SUPPRESS

// ---------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------
static void emit_event(const char *level, const char *key,
                       const char *value = nullptr) {
    Serial.print('E');
    Serial.print(',');
    Serial.print(level);
    Serial.print(',');
    Serial.print(key);
    if (value) {
        Serial.print(',');
        Serial.print(value);
    }
    Serial.print('\n');
}

// ---------------------------------------------------------------
// I²C scan
// ---------------------------------------------------------------
static void i2c_scan() {
    const uint8_t expected[] = { I2C_DRV2605, I2C_IMU_UPPER, I2C_IMU_FOREARM };
    bool found[3] = { false, false, false };

    Serial.println(F("# I2C scan start"));
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print(F("# found 0x"));
            if (addr < 0x10) Serial.print('0');
            Serial.println(addr, HEX);
            for (uint8_t i = 0; i < 3; i++) {
                if (expected[i] == addr) found[i] = true;
            }
        }
    }
    for (uint8_t i = 0; i < 3; i++) {
        if (!found[i]) {
            char val_buf[8];
            snprintf(val_buf, sizeof(val_buf), "0x%02X", expected[i]);
            emit_event(EVT_ERR, EVT_KEY_I2C_MISS, val_buf);
        }
    }
    Serial.println(F("# I2C scan done"));
}

// ---------------------------------------------------------------
// Servo sweep (bench test only — triggered by C,SWEEP)
// ---------------------------------------------------------------
static void servo_sweep() {
    g_servo.attach(PIN_SERVO_PWM);
    g_servo.write(90);  delay(500);
    g_servo.write(30);  delay(800);
    g_servo.write(150); delay(800);
    g_servo.write(90);  delay(500);
    g_servo.detach();
    Serial.println(F("# servo sweep done"));
}

// ---------------------------------------------------------------
// DRV2605 init  (startup click removed — strong click 100% caused
// motor inrush current that dropped VCC below the ATmega BOD
// threshold, triggering an infinite brownout reset loop.
// Add a 470–1000 µF cap across the motor supply before re-enabling.)
// ---------------------------------------------------------------
static void drv_init_and_click() {
    if (!g_drv.begin()) {
        emit_event(EVT_ERR, EVT_KEY_I2C_MISS, "0x5A");
        s_drv_ok = false;
        return;
    }
    s_drv_ok = true;
    g_drv.selectLibrary(1);
    g_drv.setMode(DRV2605_MODE_INTTRIG);
    g_drv.setWaveform(0, 1);   // effect #1 – strong click (Stage 8)
    g_drv.setWaveform(1, 0);
    // g_drv.go();  — removed: causes brownout reset
    Serial.println(F("# DRV2605 init OK (startup click disabled)"));
}

// ---------------------------------------------------------------
// IMU init — uses lightweight Wire driver, no heap allocation
// ---------------------------------------------------------------
static void imu_init() {
    if (!mpu6050_init(I2C_IMU_UPPER)) {
        emit_event(EVT_ERR, EVT_KEY_I2C_MISS, "0x68");
        s_imu_upper_ok = false;
    } else {
        s_imu_upper_ok = true;
        Serial.println(F("# IMU upper arm OK (0x68)"));
    }
    if (!mpu6050_init(I2C_IMU_FOREARM)) {
        emit_event(EVT_ERR, EVT_KEY_I2C_MISS, "0x69");
        s_imu_forearm_ok = false;
    } else {
        s_imu_forearm_ok = true;
        Serial.println(F("# IMU forearm OK (0x69)"));
    }
}

// ---------------------------------------------------------------
// Send one 100 Hz telemetry packet
// ---------------------------------------------------------------
static void send_telemetry() {
    // --- Analog ---
    uint16_t pot      = analogRead(PIN_POT);
    uint16_t emg      = analogRead(PIN_EMG);
    uint16_t servo_fb = analogRead(PIN_SERVO_FB);
    // servo degrees as tenths (no %f on AVR snprintf)
    uint16_t sdeg_x10 = (uint16_t)((uint32_t)servo_fb * 1800UL / 1023UL);

    // --- IMU reads into shared global buffers ---
    if (s_imu_forearm_ok) {
        if (mpu6050_read(I2C_IMU_FOREARM, g_raw_fa)) {
            s_last_imu_ms = millis();   // Stage 6: timestamp for watchdog
        }
    }
    if (s_imu_upper_ok) {
        mpu6050_read(I2C_IMU_UPPER, g_raw_ua);
    }

    // --- Stage 7: differential IMU tremor detection ---
    // Subtract bias-corrected upper-arm gyro from bias-corrected forearm gyro.
    // Common-mode rejection: a voluntary reach rotates both segments together
    // → difference ≈ 0. Tremor is a rapid forearm oscillation the upper arm
    // does not follow → shows up strongly in the difference signal.
    // Falls back to forearm-only (still bias-corrected) if upper IMU absent.
    int16_t diff_gx = (g_raw_fa.gx - g_calib.fa_gx_bias) -
                      (s_imu_upper_ok ? (g_raw_ua.gx - g_calib.ua_gx_bias) : 0);
    int16_t diff_gy = (g_raw_fa.gy - g_calib.fa_gy_bias) -
                      (s_imu_upper_ok ? (g_raw_ua.gy - g_calib.ua_gy_bias) : 0);
    int16_t diff_gz = (g_raw_fa.gz - g_calib.fa_gz_bias) -
                      (s_imu_upper_ok ? (g_raw_ua.gz - g_calib.ua_gz_bias) : 0);
    // L1 norm of differential signal, convert deg/s×10 → deg/s
    float gyro_debiased = (abs(diff_gx) + abs(diff_gy) + abs(diff_gz)) * 0.1f;

    float tremor_amp  = filter_update(gyro_debiased);
    s_tremor_amp_last = tremor_amp;   // Stage 6: shared with 50 Hz control tick

    // --- Stage 5: EMG moving average ---
    s_emg_sum -= s_emg_buf[s_emg_idx];
    s_emg_buf[s_emg_idx] = emg;
    s_emg_sum += emg;
    s_emg_idx  = (s_emg_idx + 1) % EMG_AVG_N;
    s_emg_avg  = (uint16_t)(s_emg_sum / EMG_AVG_N);

    // --- Stage 5: Classifier ---
    // Priority: voluntary > tremor > rest
    // EMG gating can be disabled with C,EMG_OFF when sensor is not fitted.
    float trem_thresh_f = g_calib.trem_threshold * 0.1f;  // stored ×10
    if (s_emg_enabled && s_emg_avg >= g_calib.emg_threshold) {
        s_classifier = 1;   // voluntary — do not suppress
    } else if (tremor_amp >= trem_thresh_f) {
        s_classifier = 2;   // tremor detected
    } else {
        s_classifier = 0;   // rest
    }

    // Encode tremor_amp as integer hundredths (no %f on AVR snprintf)
    uint16_t tamp_x100 = (uint16_t)constrain(tremor_amp * 100.0f, 0.0f, 9999.0f);

    // static: 128 bytes removed from stack → .bss (never re-entrant, safe on AVR)
    static char buf[128];
    snprintf(buf, sizeof(buf),
        "T,%lu,%u,%u,%u.%u,"
        "%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,"
        "%u.%02u,%d,%d",
        (unsigned long)millis(),
        pot, emg,
        sdeg_x10 / 10, sdeg_x10 % 10,
        g_raw_fa.ax, g_raw_fa.ay, g_raw_fa.az,
        g_raw_fa.gx, g_raw_fa.gy, g_raw_fa.gz,
        g_raw_ua.ax, g_raw_ua.ay, g_raw_ua.az,
        g_raw_ua.gx, g_raw_ua.gy, g_raw_ua.gz,
        tamp_x100 / 100, tamp_x100 % 100,
        (int)g_state,
        (int)s_classifier);
    Serial.println(buf);
}

// ---------------------------------------------------------------
// Calibration state machine
// ---------------------------------------------------------------
static uint8_t  s_calib_phase    = 0;
static uint32_t s_calib_start_ms = 0;
static const uint16_t CALIB_STILL_MS = 3000;

static int32_t  s_acc_fa_gx, s_acc_fa_gy, s_acc_fa_gz;
static int32_t  s_acc_ua_gx, s_acc_ua_gy, s_acc_ua_gz;
static int32_t  s_acc_emg;
static uint16_t s_acc_n;

static uint16_t s_rom_min, s_rom_max;
static CalibData s_pending_calib;

static void calib_start() {
    if (g_state != STATE_STREAM && g_state != STATE_IDLE) return;
    state_transition(STATE_CALIB);
    emit_event(EVT_INFO, EVT_KEY_STATE, "CALIB");

    s_calib_phase    = 1;
    s_calib_start_ms = millis();
    s_acc_fa_gx = s_acc_fa_gy = s_acc_fa_gz = 0;
    s_acc_ua_gx = s_acc_ua_gy = s_acc_ua_gz = 0;
    s_acc_emg = 0;
    s_acc_n   = 0;
    calib_defaults(s_pending_calib);

    emit_event(EVT_INFO, "calib_still", "hold_still_3s");
}

static void calib_update() {
    if (g_state != STATE_CALIB) return;
    uint32_t now = millis();

    if (s_calib_phase == 1) {
        // Re-read IMU into shared globals for accumulation.
        // (send_telemetry, called after this, will read again — that's fine.)
        if (s_imu_forearm_ok) mpu6050_read(I2C_IMU_FOREARM, g_raw_fa);
        if (s_imu_upper_ok)   mpu6050_read(I2C_IMU_UPPER,   g_raw_ua);

        if (s_imu_forearm_ok) {
            s_acc_fa_gx += g_raw_fa.gx;
            s_acc_fa_gy += g_raw_fa.gy;
            s_acc_fa_gz += g_raw_fa.gz;
        }
        if (s_imu_upper_ok) {
            s_acc_ua_gx += g_raw_ua.gx;
            s_acc_ua_gy += g_raw_ua.gy;
            s_acc_ua_gz += g_raw_ua.gz;
        }
        s_acc_emg += analogRead(PIN_EMG);
        s_acc_n++;

        if (now - s_calib_start_ms >= CALIB_STILL_MS) {
            if (s_acc_n > 0) {
                s_pending_calib.fa_gx_bias = (int16_t)(s_acc_fa_gx / s_acc_n);
                s_pending_calib.fa_gy_bias = (int16_t)(s_acc_fa_gy / s_acc_n);
                s_pending_calib.fa_gz_bias = (int16_t)(s_acc_fa_gz / s_acc_n);
                s_pending_calib.ua_gx_bias = (int16_t)(s_acc_ua_gx / s_acc_n);
                s_pending_calib.ua_gy_bias = (int16_t)(s_acc_ua_gy / s_acc_n);
                s_pending_calib.ua_gz_bias = (int16_t)(s_acc_ua_gz / s_acc_n);
                s_pending_calib.emg_baseline = (uint16_t)(s_acc_emg / s_acc_n);
                s_pending_calib.pot_mid      = analogRead(PIN_POT);
            }
            s_calib_phase = 2;
            s_rom_min = 1023; s_rom_max = 0;
            emit_event(EVT_INFO, "calib_rom", "move_full_range");
        }

    } else if (s_calib_phase == 2) {
        uint16_t fb = analogRead(PIN_SERVO_FB);
        if (fb < s_rom_min) s_rom_min = fb;
        if (fb > s_rom_max) s_rom_max = fb;
    }
}

// ---------------------------------------------------------------
// Potentiometer suppress toggle (checked every 100 Hz telemetry tick)
// Level-based: hold pot in the upper zone to stay in SUPPRESS.
// Fires a distinct haptic profile each time the pot crosses a zone
// boundary (left / middle dead-band / right).
// ---------------------------------------------------------------
static void poll_pot_suppress(uint16_t pot_adc) {
    // ── Zone classification ─────────────────────────────────────
    //   0 = left  (< POT_OFF_THRESH) — suppress OFF
    //   1 = middle dead-band         — no state change
    //   2 = right (> POT_ON_THRESH)  — suppress ON
    uint8_t zone;
    if      (pot_adc >= POT_ON_THRESH)  zone = 2;
    else if (pot_adc <= POT_OFF_THRESH) zone = 0;
    else                                zone = 1;

    // ── Haptic on zone transitions ──────────────────────────────
    // s_pot_zone is initialised to 0xFF (unknown) so the very first
    // sample sets the zone silently — no spurious buzz at boot.
    static uint8_t s_pot_zone = 0xFF;
    if (zone != s_pot_zone) {
        if (s_pot_zone != 0xFF) {   // skip buzz on first-ever call
            switch (zone) {
                case 0: haptic_zone_left();   break;
                case 1: haptic_zone_middle(); break;
                case 2: haptic_zone_right();  break;
            }
        }
        s_pot_zone = zone;
    }

    // ── State machine ───────────────────────────────────────────
    if (zone == 2) {
        // Right zone — enter SUPPRESS (servo stays detached until tremor fires)
        if (g_state == STATE_STREAM || g_state == STATE_IDLE) {
            control_init(&g_servo, &g_calib);
            state_transition(STATE_SUPPRESS);
            emit_event(EVT_INFO, EVT_KEY_STATE, "SUPPRESS");
        }
    } else if (zone == 0) {
        // Left zone — exit SUPPRESS
        if (g_state == STATE_SUPPRESS) {
            control_release();
            state_transition(STATE_STREAM);
            emit_event(EVT_INFO, EVT_KEY_STATE, "STREAM");
        }
    }
    // Middle zone: no state change; pot value is read by the control
    // loop as gain (0.0–1.0) — no extra code needed here.
}

// ---------------------------------------------------------------
// Command parser
// ---------------------------------------------------------------
static void handle_command(const char *line) {
    if (line[0] != 'C' || line[1] != ',') return;
    const char *cmd = line + 2;

    if (strncmp(cmd, CMD_PING, 4) == 0) {
        emit_event(EVT_INFO, EVT_KEY_PONG);

    } else if (strncmp(cmd, "SWEEP", 5) == 0) {
        emit_event(EVT_WARN, "sweep", "arm_off_confirm");
        servo_sweep();

    } else if (strncmp(cmd, CMD_CALIB_START, 11) == 0) {
        calib_start();

    } else if (strncmp(cmd, CMD_CALIB_SAVE, 10) == 0) {
        if (g_state == STATE_CALIB && s_calib_phase == 2) {
            s_pending_calib.servo_min = s_rom_min;
            s_pending_calib.servo_max = s_rom_max;
            s_pending_calib.magic     = CALIB_MAGIC;
            calib_save(s_pending_calib);
            g_calib       = s_pending_calib;
            g_calib_valid = true;
            control_init(&g_servo, &g_calib);
            s_calib_phase = 3;
            emit_event(EVT_INFO, "calib_saved", "ok");
            state_transition(STATE_STREAM);
            emit_event(EVT_INFO, EVT_KEY_STATE, "STREAM");
        }

    } else if (strncmp(cmd, CMD_CALIB_ABORT, 11) == 0) {
        s_calib_phase = 0;
        state_transition(STATE_STREAM);
        emit_event(EVT_INFO, EVT_KEY_STATE, "STREAM");
        emit_event(EVT_WARN, "calib_aborted");

    } else if (strncmp(cmd, CMD_SET_BAND, 8) == 0) {
        const char *args = cmd + 9;
        float lo = atof(args);
        const char *comma = strchr(args, ',');
        float hi = comma ? atof(comma + 1) : g_filter_high_hz;
        filter_init(lo, hi);
        emit_event(EVT_INFO, "band_set", "ok");

    } else if (strncmp(cmd, CMD_SET_GAIN, 8) == 0) {
        const char *args = cmd + 9;
        float g = atof(args);
        if (g < 0.0f) g = 0.0f;
        if (g > 1.0f) g = 1.0f;
        s_gain_pct    = (uint8_t)(g * 100.0f);
        s_gain_locked = true;
        emit_event(EVT_INFO, "gain_set", "ok");

    } else if (strncmp(cmd, CMD_SET_THRESH, 10) == 0) {
        // C,SET_THRESH,<deg_s>  e.g. C,SET_THRESH,25.0
        // RAM-only; not written to EEPROM
        const char *args = cmd + 11;
        float t = atof(args);
        if (t < 1.0f)   t = 1.0f;
        if (t > 200.0f) t = 200.0f;
        g_calib.trem_threshold = (uint16_t)(t * 10.0f);
        emit_event(EVT_INFO, "thresh_set", "ok");

    } else if (strncmp(cmd, "EMG_OFF", 7) == 0) {
        // Disable EMG voluntary-detection — use when sensor is not fitted
        // or is false-triggering VOLUNTARY at rest.
        s_emg_enabled = false;
        emit_event(EVT_INFO, "emg_gating", "off");

    } else if (strncmp(cmd, "EMG_ON", 6) == 0) {
        s_emg_enabled = true;
        emit_event(EVT_INFO, "emg_gating", "on");

    } else if (strncmp(cmd, CMD_SUPPRESS_ON, 11) == 0) {
        if (g_state == STATE_STREAM || g_state == STATE_IDLE) {
            control_init(&g_servo, &g_calib);
            state_transition(STATE_SUPPRESS);
            emit_event(EVT_INFO, EVT_KEY_STATE, "SUPPRESS");
        }

    } else if (strncmp(cmd, CMD_SUPPRESS_OFF, 12) == 0) {
        if (g_state == STATE_SUPPRESS) {
            control_release();
            state_transition(STATE_STREAM);
            emit_event(EVT_INFO, EVT_KEY_STATE, "STREAM");
        }

    } else if (strncmp(cmd, CMD_ESTOP, 5) == 0) {
        control_release();
        state_transition(STATE_FAULT);
        emit_event(EVT_ERR, "estop", "servo_detached");

    } else if (strncmp(cmd, CMD_RESET, 5) == 0) {
        if (g_state == STATE_FAULT) {
            state_transition(STATE_STREAM);
            emit_event(EVT_INFO, EVT_KEY_STATE, "STREAM");
        }
    }
}

// ---------------------------------------------------------------
// Non-blocking serial reader
// ---------------------------------------------------------------
static void poll_serial_input() {
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (s_cmd_len > 0) {
                s_cmd_buf[s_cmd_len] = '\0';
                handle_command(s_cmd_buf);
                s_cmd_len = 0;
            }
        } else {
            if (s_cmd_len < (uint8_t)(sizeof(s_cmd_buf) - 1)) {
                s_cmd_buf[s_cmd_len++] = c;
            }
        }
    }
}

// ---------------------------------------------------------------
// Setup
// ---------------------------------------------------------------
void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000) {}

    Wire.begin();
    Wire.setClock(400000);

    state_transition(STATE_SELFTEST);
    emit_event(EVT_INFO, EVT_KEY_STATE, "SELFTEST");

    emit_event(EVT_WARN, EVT_KEY_USB_ISO);
    Serial.println(F("# SAFETY: confirm USB isolator is connected before EMG use."));
    Serial.println(F("# SAFETY: servo should be disconnected from arm cuff during first flash."));

    i2c_scan();
    imu_init();
    drv_init_and_click();
    haptic_init(s_drv_ok);   // must be called after drv_init_and_click()

    filter_init(2.0f, 8.0f);

    if (calib_load(g_calib)) {
        g_calib_valid = true;
        emit_event(EVT_INFO, "calib_loaded", "eeprom");
    } else {
        emit_event(EVT_WARN, "calib_defaults", "not_calibrated");
    }

    control_init(&g_servo, &g_calib);
    s_last_imu_ms = millis();

    state_transition(STATE_STREAM);
    emit_event(EVT_INFO, EVT_KEY_STATE, "STREAM");

    char ver_buf[8];
    snprintf(ver_buf, sizeof(ver_buf), "v=%d", PROTO_VERSION);
    emit_event(EVT_INFO, EVT_KEY_READY, ver_buf);

    Serial.println(F("# Pot RIGHT third=SUPPRESS, middle=gain, LEFT third=OFF."));
    Serial.println(F("# If dashboard shows VOLUNTARY at rest: send C,EMG_OFF"));
}

// ---------------------------------------------------------------
// Loop  –  100 Hz telemetry + 50 Hz servo control
// ---------------------------------------------------------------
#define CTRL_HZ           50
#define CTRL_INTERVAL_US  (1000000UL / CTRL_HZ)   // 20 000 µs

static uint32_t s_next_telem_us = 0;
static uint32_t s_next_ctrl_us  = 0;

void loop() {
    uint32_t now_us = micros();

    // ── 100 Hz telemetry tick ───────────────────────────────────
    if ((int32_t)(now_us - s_next_telem_us) >= 0) {
        s_next_telem_us += TELEM_INTERVAL_US;

        // Pot suppress toggle — read once and share with send_telemetry
        uint16_t pot_adc = analogRead(PIN_POT);
        poll_pot_suppress(pot_adc);

        calib_update();      // no-op outside STATE_CALIB
        send_telemetry();    // IMU re-read here; updates s_tremor_amp_last
    }

    // ── 50 Hz servo control tick (STATE_SUPPRESS only) ──────────
    if ((int32_t)(now_us - s_next_ctrl_us) >= 0) {
        s_next_ctrl_us += CTRL_INTERVAL_US;

        if (g_state == STATE_SUPPRESS) {
            // IMU watchdog: forearm IMU must respond within 200 ms
            if (millis() - s_last_imu_ms > IMU_TIMEOUT_MS) {
                control_release();
                state_transition(STATE_FAULT);
                emit_event(EVT_ERR, EVT_KEY_IMU_TOUT);
            } else {
                float gain_01;
                if (s_gain_locked) {
                    gain_01 = s_gain_pct * 0.01f;
                } else {
                    gain_01 = analogRead(PIN_POT) * (1.0f / 1023.0f);
                }
                control_update(s_classifier, s_tremor_amp_last, gain_01);
            }
        }
    }

    poll_serial_input();
}
