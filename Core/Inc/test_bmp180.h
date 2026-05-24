/*
 * test_bmp180.h
 *
 * Small, reusable test API for exercising the BMP180 sensor.
 * The implementation is HAL-independent beyond using the existing BMP180
 * driver (BMP180_Start, BMP180_GetTemp, BMP180_GetPress, BMP180_GetAlt).
 *
 * Usage:
 *  - Include this header and call TestBMP180_Init() once after I2C is up.
 *  - Call TestBMP180_RunOnce() to read the sensor and get results synchronously.
 *
 * The test module intentionally does NOT assume a UART; it returns data in a
 * struct so callers can decide how to report results (7-seg, LED, UART, debugger).
 */

#ifndef TEST_BMP180_H
#define TEST_BMP180_H

#include <stdint.h>

/** Result structure for BMP180 test readings */
typedef struct {
    int32_t temperature_x10; /* temperature in 0.1 degC */
    int32_t pressure_pa;     /* pressure in Pascal */
    int32_t altitude_m;      /* computed altitude in metres */
    int32_t pressure_delta;  /* pressure change since last sample */
    uint8_t i2c_ok;          /* non-zero if the BMP180 answered on I2C */
    uint8_t ok;              /* non-zero if the read succeeded */
} TestBMP180_Result_t;

/** Check whether the BMP180 is reachable on I2C.
 *  Reads the chip-id register (0xD0) and expects the BMP180 ID (0x55).
 *  This is the standard way to verify wiring / device presence on embedded I2C sensors. */
uint8_t TestBMP180_CheckI2C(void);

/** Initialize test helper. Must be called after I2C and HAL are initialized.
 *  Internally calls BMP180_Start() to ensure the sensor is ready. */
void TestBMP180_Init(void);

/** Perform a single synchronous test/read cycle.
 *  Returns a filled TestBMP180_Result_t with measurement values. */
TestBMP180_Result_t TestBMP180_RunOnce(void);

#endif /* TEST_BMP180_H */
