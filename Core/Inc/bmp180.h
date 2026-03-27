/**
 * @file bmp180.h
 * @brief BMP180 Sensor Library
 * 
 * Diese Datei enthält die Schnittstellen für die BMP180-Sensorfunktionen.
 */
#ifndef BMP180_H
#define BMP180_H

#include <stdint.h>

// BMP180 I²C-Adresse
#define BMP180_I2C_ADDRESS 0x77
// Fehlercodes
#define BMP180_OK 0          ///< Erfolgreiche Ausführung
#define BMP180_ERROR -1      ///< Allgemeiner Fehler
#define BMP180_I2C_ERROR -2  ///< I²C-Kommunikationsfehler

// Kalibrierungsdatenstruktur
typedef struct {
    int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
    uint16_t AC4;
} BMP180_CalibrationData;

 /**
 * @brief Initialisiert den BMP180-Sensor.
 * 
 * Diese Funktion liest die Kalibrierungsdaten aus dem Sensor.
 */
void BMP180_Init(void);

/**
 * @brief Liest den Luftdruck aus dem BMP180-Sensor.
 * 
 * @return Luftdruck in Pascal.
 */
int32_t BMP180_ReadPressure(void);

/**
 * @brief Liest die Temperatur aus dem BMP180-Sensor.
 * 
 * @return Temperatur in Grad Celsius.
 */
float BMP180_ReadTemperature(void);

#endif // BMP180_H