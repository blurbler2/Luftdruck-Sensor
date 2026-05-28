/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SiebenSeg.h"
#include "bmp180.h"
#include "test_bmp180.h"
#include <stdint.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Sensor readings (volatile = visible in debug watch while running)
volatile int32_t TemperatureX10 = 0;  // 0.1 degC
volatile int32_t PressurePa     = 0;  // Pascal
volatile int32_t AltitudeM      = 0;  // metres
volatile int32_t PressureDelta  = 0;  // Pa change since last sample (watchable in debug)

// Internal scaling values for 7-seg display, multiplied to avoid floating point calculations
uint16_t PressureBarX1000 = 0;  // Pressure in bar * 1000  (e.g. 994 = 0.994 bar)
uint16_t TempX100         = 0;  // Temperature * 100       (e.g. 830 = 08.30 C)
uint16_t AltitudeDispM    = 0;  // Altitude in whole metres (e.g. 266)
uint16_t DeltaAbs         = 0;  // |PressureDelta| in Pa    (e.g. 50 = 50 Pa)

// Altitude calibration (one-shot from first measurement + known reference)
/* If you know the local reference altitude (e.g. from GPS), set this
 * value (meters). If left as 0 the firmware will treat the first
 * measured pressure as the local sea-level reference (relative mode).
 */
int32_t KnownAltitudeMeters     = 0;       /* Set this to non-zero to enable absolute altitude, e.g. average for Vienna is 171m NN*/

int32_t SeaLevelPressure        = 101325;  // Pa (integer reference)
uint8_t AltitudeCalibrated      = 0;       // Flag to indicate if altitude calibration has been done (first measurement used as reference if KnownAltitudeMeters is zero)
int32_t PressurePrev            = 0;       // Previous sample for delta computation

// Display mode (cycled by buttons)
typedef enum { DISP_PRESSURE = 0, DISP_DELTA, DISP_TEMPERATURE, DISP_ALTITUDE } DispMode_t;
volatile DispMode_t DispMode = DISP_PRESSURE;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  Reset_7Seg();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  BMP180_Start();

#ifdef BMP180_TEST
  /* Optional test harness: run a single test cycle and stop so values are
   * inspectable in the debugger. Enable by adding -DBMP180_TEST to your CFLAGS.
   */
  TestBMP180_Init();
  {
    TestBMP180_Result_t tr = TestBMP180_RunOnce();
    /* expose results to the volatile debug variables used elsewhere */
    TemperatureX10 = tr.temperature_x10;
    PressurePa     = tr.pressure_pa;
    AltitudeM      = tr.altitude_m;
    PressureDelta  = tr.pressure_delta;
    /* stay here so tester can inspect values; i2c_ok shows wiring/device presence */
    while (1) { HAL_Delay(1000); }
  }
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Sample sensors continuously */
    TemperatureX10 = BMP180_GetTemp();
    PressurePa     = BMP180_GetPress(0);

    /* Initialize altitude calibration if no KnownAltitudeMeters is provided */
    if (!AltitudeCalibrated)
    {
      /* If the user provided a known reference altitude, compute the
       * corresponding sea-level pressure p0 from the current measured
       * pressure using the barometric formula inversion:
       *   p0 = p * (1 - h0 / 44330)^{-5.255}
       * This gives an absolute altitude reference. If KnownAltitudeMeters
       * is zero, keep the existing behavior and treat the first measured
       * pressure as the local reference (relative mode).
       */
      PressurePrev = PressurePa;
      if (KnownAltitudeMeters != 0)
      {
        float p = (float)PressurePa;
        float h0 = (float)KnownAltitudeMeters;
        /* Guard against invalid tiny/large values */
        if (h0 < 40000.0f && h0 > -500.0f)
        {
          float factor = 1.0f - (h0 / 44330.0f);
          if (factor <= 0.0f) factor = 1e-6f;
          float p0f = p * powf(factor, -5.255f);
          SeaLevelPressure = (int32_t)(p0f + 0.5f);
        }
        else
        {
          /* Fallback: use measured pressure */
          SeaLevelPressure = PressurePa;
        }
      }
      else
      {
        SeaLevelPressure = PressurePa;
      }
      AltitudeCalibrated = 1;
    }

    /* Derived values */
    AltitudeM     = BMP180_GetAlt(0);
    PressureDelta = PressurePa - PressurePrev;
    PressurePrev  = PressurePa;

    /* 7-segment display – scale to 4-digit integer per active mode */
    switch (DispMode)
    {
      case DISP_DELTA:
        DeltaAbs = (uint16_t)((PressureDelta < 0) ? -PressureDelta : PressureDelta); // no negative values: absolute value of pressure change since last sample
        if (DeltaAbs > 9999) DeltaAbs = 9999; // edge case: cap to display max
        Write_7Seg(DeltaAbs, 4);  /* e.g. 0050 = 50 Pa change (no decimal) */
        break;

      case DISP_PRESSURE:
        PressureBarX1000 = (uint16_t)((PressurePa + 50) / 100);
        if (PressureBarX1000 > 9999) PressureBarX1000 = 9999;
        Write_7Seg(PressureBarX1000, 0);  /* e.g. 0994 = 0.994 bar */
        break;

      case DISP_TEMPERATURE:
        if (TemperatureX10 < 0)
        {
          /* negative temp not displayable with current 7-seg helper; show 0 */
          Write_7Seg(0, 4);
        }
        else
        {
          TempX100 = (uint16_t)(TemperatureX10 * 10); /* 0.1 degC -> 0.01 display format */
          if (TempX100 > 9999) TempX100 = 9999;
          Write_7Seg(TempX100, 1);  /* e.g. 0830 = 08.30 C */
        }
        break;

      case DISP_ALTITUDE:
        AltitudeDispM = (uint16_t)((AltitudeM < 0) ? 0 : AltitudeM);
        if (AltitudeDispM > 9999) AltitudeDispM = 9999;
        Write_7Seg(AltitudeDispM, 4);  /* e.g. 0266 = 266 m (no decimal) */
        break;
    }

    HAL_Delay(500); // sample every 500 ms
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA7 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC5 PC9 PC10 PC11
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD0 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : Button1_Pin Button2_Pin */
  GPIO_InitStruct.Pin = Button1_Pin|Button2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  EXTI callback – handles Button1 (PD1) and Button2 (PD2) with debounce.
  *
  * Button1: toggles between DISP_PRESSURE and DISP_DELTA.
  * Button2: cycles DISP_TEMPERATURE → DISP_ALTITUDE → DISP_PRESSURE → ...
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t lastTick1 = 0;
  static uint32_t lastTick2 = 0;

  if (GPIO_Pin == Button1_Pin)
  {
    if (HAL_GetTick() - lastTick1 > 200) // debounce: ignore if less than 200 ms since last press
    {
      lastTick1 = HAL_GetTick(); // update last tick for debounce
      DispMode = (DispMode == DISP_DELTA) ? DISP_PRESSURE : DISP_DELTA;
    }
  }
  else if (GPIO_Pin == Button2_Pin)
  {
    if (HAL_GetTick() - lastTick2 > 200) // debounce: ignore if less than 200 ms since last press
    {
      lastTick2 = HAL_GetTick(); // update last tick for debounce
      switch (DispMode)
      {
        case DISP_TEMPERATURE: DispMode = DISP_ALTITUDE;    break;
        case DISP_ALTITUDE:    DispMode = DISP_PRESSURE;    break;
        default:               DispMode = DISP_TEMPERATURE; break;
      }
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
