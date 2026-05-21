#ifndef _BMP180_H_
#define _BMP180_H_


#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @file bmp180.h
 * @brief BMP180 Sensor Startup and Reading Functions
 * 
 * Call this function in main() before using any other function to read the calliberation data from the sensor and store it in global variables
 */
void BMP180_Start (void);

/**
 * @brief Gets temperature from BMP180.
 * @return Temperature in 0.1 degree Celsius (e.g. 253 = 25.3 degree Celsius)
 */
int32_t BMP180_GetTemp (void);

/**
 * @brief Gets pressure from BMP180.
 * @param oss Oversampling setting
 * @return Pressure in Pascal (Pa)
 */
int32_t BMP180_GetPress (int oss);

/**
 * @brief Gets altitude estimate from pressure.
 * @param oss Oversampling setting
 * @return Altitude in whole meters (m)
 */
int32_t BMP180_GetAlt (int oss);

#endif /* INC_BMP180_H_ */
