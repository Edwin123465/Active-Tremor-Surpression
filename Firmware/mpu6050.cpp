// ================================================================
//  mpu6050.cpp  –  Lightweight MPU-6050 driver
//  See mpu6050.h for API and rationale.
// ================================================================

#include <Arduino.h>
#include "mpu6050.h"

// ── Register addresses ───────────────────────────────────────────
#define MPU_PWR1    0x6B   // Power management 1
#define MPU_CONFIG  0x1A   // DLPF configuration (bits 2:0)
#define MPU_GCFG    0x1B   // Gyro full-scale select (bits 4:3)
#define MPU_ACFG    0x1C   // Accel full-scale select (bits 4:3)
#define MPU_DATA    0x3B   // First of 14 data bytes (ACCEL_XOUT_H)

// ── Internal helpers ─────────────────────────────────────────────
static void write_reg(uint8_t addr, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

// ── Public API ───────────────────────────────────────────────────
bool mpu6050_init(uint8_t addr) {
    // Verify device presence via I²C ACK
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() != 0) return false;

    write_reg(addr, MPU_PWR1,   0x00);  // clear SLEEP bit — wake up
    delay(5);                            // oscillator settle time
    write_reg(addr, MPU_CONFIG, 0x04);  // DLPF_CFG=4 → 21 Hz BW
    write_reg(addr, MPU_GCFG,   0x08);  // FS_SEL=1   → ±500 deg/s
    write_reg(addr, MPU_ACFG,   0x10);  // AFS_SEL=2  → ±8 g
    return true;
}

bool mpu6050_read(uint8_t addr, Mpu6050Raw &out) {
    // Point register pointer to first data byte
    Wire.beginTransmission(addr);
    Wire.write(MPU_DATA);
    if (Wire.endTransmission(false) != 0) return false;

    // Burst-read: 6 accel + 2 temp + 6 gyro = 14 bytes
    if (Wire.requestFrom(addr, (uint8_t)14) != 14) return false;

    int16_t raw_ax = (int16_t)((Wire.read() << 8) | Wire.read());
    int16_t raw_ay = (int16_t)((Wire.read() << 8) | Wire.read());
    int16_t raw_az = (int16_t)((Wire.read() << 8) | Wire.read());
    Wire.read(); Wire.read();   // discard temperature (2 bytes)
    int16_t raw_gx = (int16_t)((Wire.read() << 8) | Wire.read());
    int16_t raw_gy = (int16_t)((Wire.read() << 8) | Wire.read());
    int16_t raw_gz = (int16_t)((Wire.read() << 8) | Wire.read());

    // Accel: ±8 g, sensitivity = 4096 LSB/g
    //   → protocol unit g×1000: raw * 1000 / 4096 (exact integer)
    out.ax = (int16_t)((int32_t)raw_ax * 1000 / 4096);
    out.ay = (int16_t)((int32_t)raw_ay * 1000 / 4096);
    out.az = (int16_t)((int32_t)raw_az * 1000 / 4096);

    // Gyro: ±500 deg/s, sensitivity = 65.5 LSB/(deg/s)
    //   → protocol unit deg/s×10: raw * 10 / 65.5 ≈ raw * 10 / 66
    //   Error < 0.8 % — acceptable for tremor detection.
    out.gx = (int16_t)((int32_t)raw_gx * 10 / 66);
    out.gy = (int16_t)((int32_t)raw_gy * 10 / 66);
    out.gz = (int16_t)((int32_t)raw_gz * 10 / 66);

    return true;
}
