#include "stm32f4xx_hal.h"
#include <stdint.h>

/* I2C handles for BMP180 */
extern I2C_HandleTypeDef hi2c1;
#define BMP180_I2C &hi2c1
/* BMP180 I2C address */
#define BMP180_ADDRESS 0xEE
/* Atmospheric pressure at sea level (Pa) */
#define ATM_PRESS 101325

/* Calibration constants from sensor EEPROM */
static int16_t AC1 = 0;
static int16_t AC2 = 0;
static int16_t AC3 = 0;
static uint16_t AC4 = 0;
static uint16_t AC5 = 0;
static uint16_t AC6 = 0;
static int16_t B1 = 0;
static int16_t B2 = 0;
static int16_t MB = 0;
static int16_t MC = 0;
static int16_t MD = 0;

/* B5 from last temperature conversion, required by pressure algorithm */
static int32_t g_lastB5 = 0;
static uint8_t g_hasB5 = 0;

/* Calibrated reference pressure from main.c (Pa) */
extern int32_t SeaLevelPressure;

/* reads the calliberation data of the BMP180 from the sensor and stores it in global variables */
void read_calliberation_data (void)
{
	uint8_t Callib_Data[22] = {0};
	uint16_t Callib_Start = 0xAA;
	HAL_I2C_Mem_Read(BMP180_I2C, BMP180_ADDRESS, Callib_Start, 1, Callib_Data, 22, HAL_MAX_DELAY);

	AC1 = (int16_t)((Callib_Data[0] << 8) | Callib_Data[1]);
	AC2 = (int16_t)((Callib_Data[2] << 8) | Callib_Data[3]);
	AC3 = (int16_t)((Callib_Data[4] << 8) | Callib_Data[5]);
	AC4 = (uint16_t)((Callib_Data[6] << 8) | Callib_Data[7]);
	AC5 = (uint16_t)((Callib_Data[8] << 8) | Callib_Data[9]);
	AC6 = (uint16_t)((Callib_Data[10] << 8) | Callib_Data[11]);
	B1  = (int16_t)((Callib_Data[12] << 8) | Callib_Data[13]);
	B2  = (int16_t)((Callib_Data[14] << 8) | Callib_Data[15]);
	MB  = (int16_t)((Callib_Data[16] << 8) | Callib_Data[17]);
	MC  = (int16_t)((Callib_Data[18] << 8) | Callib_Data[19]);
	MD  = (int16_t)((Callib_Data[20] << 8) | Callib_Data[21]);
}


/* Get uncompensated temperature */
uint16_t Get_UTemp (void)
{
	uint8_t datatowrite = 0x2E;
	uint8_t Temp_RAW[2] = {0};
	HAL_I2C_Mem_Write(BMP180_I2C, BMP180_ADDRESS, 0xF4, 1, &datatowrite, 1, 1000);
	HAL_Delay(5);
	HAL_I2C_Mem_Read(BMP180_I2C, BMP180_ADDRESS, 0xF6, 1, Temp_RAW, 2, 1000);
	return (uint16_t)((Temp_RAW[0] << 8) | Temp_RAW[1]);
}

/* Get true temperature in 0.1 degree Celsius */
int32_t BMP180_GetTemp (void)
{
	int32_t UT = (int32_t)Get_UTemp();
	int32_t X1 = ((UT - (int32_t)AC6) * (int32_t)AC5) >> 15;
	int32_t X2 = ((int32_t)MC << 11) / (X1 + (int32_t)MD);
	g_lastB5 = X1 + X2;
	g_hasB5 = 1;
	/* Temperature in 0.1 degC */
	return (g_lastB5 + 8) >> 4;
}


/* Get uncompensated pressure */
uint32_t Get_UPress (int oss)
{
	uint8_t datatowrite = (uint8_t)(0x34 + (oss << 6));
	uint8_t Press_RAW[3] = {0};
	HAL_I2C_Mem_Write(BMP180_I2C, BMP180_ADDRESS, 0xF4, 1, &datatowrite, 1, 1000);
	switch (oss)
	{
		case 0: HAL_Delay(5);  break;
		case 1: HAL_Delay(8);  break;
		case 2: HAL_Delay(14); break;
		case 3: HAL_Delay(26); break;
		default: HAL_Delay(26); break;
	}
	HAL_I2C_Mem_Read(BMP180_I2C, BMP180_ADDRESS, 0xF6, 1, Press_RAW, 3, 1000);
	return (((uint32_t)Press_RAW[0] << 16) | ((uint32_t)Press_RAW[1] << 8) | Press_RAW[2]) >> (8 - oss);
}

/* Get true pressure in Pascals */
int32_t BMP180_GetPress (int oss)
{
	int32_t UP;
	int32_t B6;
	int32_t X1;
	int32_t X2;
	int32_t X3;
	int32_t B3;
	uint32_t B4;
	uint32_t B7;
	int32_t p;

	if (!g_hasB5)
	{
		(void)BMP180_GetTemp();
	}

	UP = (int32_t)Get_UPress(oss);
	B6 = g_lastB5 - 4000;

	X1 = ((int32_t)B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = ((int32_t)AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((int32_t)AC1 * 4 + X3) << oss) + 2) >> 2;

	X1 = ((int32_t)AC3 * B6) >> 13;
	X2 = ((int32_t)B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = (X1 + X2 + 2) >> 2;
	B4 = ((uint32_t)AC4 * (uint32_t)(X3 + 32768)) >> 15;
	B7 = (uint32_t)(UP - B3) * (uint32_t)(50000U >> (uint32_t)oss);

	if (B7 < 0x80000000U)
		p = (int32_t)((B7 << 1) / B4);
	else
		p = (int32_t)((B7 / B4) << 1);

	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;
	p += (X1 + X2 + 3791) >> 4;

	return p;
}

/* Get altitude above sea level in meters */
int32_t BMP180_GetAlt (int oss)
{
	/* Integer approximation around reference pressure:
	 * dh ~= (p0 - p) * 8434 / p0
	 */
	int32_t p = BMP180_GetPress(oss);
	int32_t p0 = SeaLevelPressure > 0 ? SeaLevelPressure : ATM_PRESS;
	return (int32_t)(((int64_t)(p0 - p) * 8434) / p0);
}

/* Initialize BMP180 sensor */
void BMP180_Start (void)
{
	read_calliberation_data();
}

