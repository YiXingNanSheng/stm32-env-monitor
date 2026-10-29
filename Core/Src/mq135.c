/**
  ******************************************************************************
  * @file    mq135.c
  * @brief   MQ135 空气质量传感器驱动（ADC1_IN1 / PA1）
  ******************************************************************************
  */

#include "mq135.h"

/* 单次转换：启动 -> 等 EOC -> 取值 -> 停止 */
static Sensor_Status_t MQ135_ReadOnce(uint16_t *value)
{
  if (value == 0)
  {
    return SENSOR_ERROR;
  }

  HAL_ADC_Start(&hadc1);

  if (HAL_ADC_PollForConversion(&hadc1, 20U) != HAL_OK)
  {
    HAL_ADC_Stop(&hadc1);
    return SENSOR_TIMEOUT;
  }

  *value = (uint16_t)HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);

  return SENSOR_OK;
}

Sensor_Status_t MQ135_Init(void)
{
  /* STM32F1 的 ADC 上电后必须校准一次，否则读数整体偏低 */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    return SENSOR_ERROR;
  }

  return SENSOR_OK;
}

Sensor_Status_t MQ135_Read(uint16_t *adc_value)
{
  uint32_t sum = 0U;
  uint16_t sample = 0U;

  if (adc_value == 0)
  {
    return SENSOR_ERROR;
  }

  for (uint8_t i = 0; i < MQ135_SAMPLE_COUNT; i++)
  {
    if (MQ135_ReadOnce(&sample) != SENSOR_OK)
    {
      return SENSOR_TIMEOUT;
    }

    sum += sample;
    HAL_Delay(2);      /* 让采样点分散开，避免同源干扰 */
  }

  *adc_value = (uint16_t)(sum / MQ135_SAMPLE_COUNT);
  return SENSOR_OK;
}

Air_Level_t MQ135_GetLevel(uint16_t adc_value)
{
  if (adc_value < 1200U)
  {
    return AIR_LEVEL_GOOD;
  }
  else if (adc_value < 2500U)
  {
    return AIR_LEVEL_NORMAL;
  }
  else
  {
    return AIR_LEVEL_WARNING;
  }
}

const char *MQ135_GetLevelText(Air_Level_t level)
{
  switch (level)
  {
    case AIR_LEVEL_GOOD:    return "GOOD";
    case AIR_LEVEL_NORMAL:  return "NORMAL";
    case AIR_LEVEL_WARNING: return "WARN";
    default:                return "UNKNOWN";
  }
}
