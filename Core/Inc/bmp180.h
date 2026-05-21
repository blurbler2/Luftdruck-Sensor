#ifndef _BMP180_H_
#define _BMP180_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @file bmp180.h
 * @brief BMP180 sensor driver for temperature, pressure and altitude.
 * Diese Bibliothek liest die Kalibrierungsdaten des Sensors ein, misst Rohwerte
 * und kompensiert sie gemäß Datenblatt.
 *
 * Öffentliche API:
 * - `BMP180_Start()` initialisiert den Sensor.
 * - `BMP180_GetTemp()` liefert die Temperatur in 0,1 °C.
 * - `BMP180_GetPress(int oss)` liefert den Luftdruck in Pascal.
 * - `BMP180_GetAlt(int oss)` liefert die Höhe in Metern.
 *
 * Siehe die Doxygen-Seite `bmp180_driver.dox` für die zusammenhängende
 * Bibliotheksdokumentation.
 */

/**
 * @brief Initialize the BMP180 sensor and load calibration data.
 */
void BMP180_Start (void);

/**
 * @brief Get temperature from BMP180.
 * @return Temperature in 0.1 degree Celsius (e.g. 253 = 25.3 degree Celsius)
 */
int32_t BMP180_GetTemp (void);

/**
 * @brief Get pressure from BMP180.
 * @param oss Oversampling setting
 * @return Pressure in Pascal (Pa)
 */
int32_t BMP180_GetPress (int oss);

/**
 * @brief Get altitude estimate from pressure.
 * @param oss Oversampling setting
 * @return Altitude in whole meters (m)
 */
int32_t BMP180_GetAlt (int oss);

#endif /* INC_BMP180_H_ */
