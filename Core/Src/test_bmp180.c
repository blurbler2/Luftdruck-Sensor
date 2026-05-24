/* test_bmp180.c
 * Reusable test helper for BMP180 sensor.
 */

#include "test_bmp180.h"
#include "bmp180.h"
#include <stdint.h>

extern I2C_HandleTypeDef hi2c1;

/* Check if BMP180 is present on I2C by reading chip ID register (0xD0) */
static uint8_t bmp180_i2c_probe(void)
{
    uint8_t chip_id = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, 0xEE, 0xD0, 1, &chip_id, 1, HAL_MAX_DELAY);
    return (status == HAL_OK && chip_id == 0x55U) ? 1U : 0U;
}

static int32_t last_pressure = 0;

uint8_t TestBMP180_CheckI2C(void)
{
    return bmp180_i2c_probe();
}

void TestBMP180_Init(void)
{
    /* Ensure the BMP180 driver is initialized */
    BMP180_Start();
    /* prime last_pressure with an initial read if possible */
    last_pressure = BMP180_GetPress(0);
}

TestBMP180_Result_t TestBMP180_RunOnce(void)
{
    TestBMP180_Result_t r = {0};

    r.i2c_ok = bmp180_i2c_probe();
    if (!r.i2c_ok)
    {
        r.ok = 0;
        return r;
    }

    /* Read temperature (0.1 degC) */
    r.temperature_x10 = BMP180_GetTemp();

    /* Read pressure (Pa) */
    r.pressure_pa = BMP180_GetPress(0);

    /* Compute altitude using existing helper */
    r.altitude_m = BMP180_GetAlt(0);

    /* Pressure delta since last read */
    r.pressure_delta = r.pressure_pa - last_pressure;
    last_pressure = r.pressure_pa;

    /* If pressure is non-zero we consider the read successful */
    r.ok = (r.pressure_pa != 0) ? 1 : 0;

    return r;
}
