/**
  ******************************************************************************
  * @file    env_sensor.c
  * @brief   传感器聚合层：把三路传感器统一成 Environment_Data_t
  ******************************************************************************
  */

#include "env_sensor.h"
#include "dht11.h"
#include "bh1750.h"

void EnvSensor_Init(Environment_Data_t *data)
{
  if (data == 0)
  {
    return;
  }

  data->temperature = 0.0f;
  data->humidity = 0.0f;
  data->light = 0.0f;
  data->air_quality = 0U;
  data->air_level = AIR_LEVEL_GOOD;

  data->dht11_valid = 0U;
  data->bh1750_valid = 0U;
  data->mq135_valid = 0U;
}

void EnvSensor_ReadAll(Environment_Data_t *data)
{
  float temperature = 0.0f;
  float humidity = 0.0f;
  float light = 0.0f;
  uint16_t air = 0U;

  if (data == 0)
  {
    return;
  }

  /* 空气质量：ADC + 分级 */
  if (MQ135_Read(&air) == SENSOR_OK)
  {
    data->air_quality = air;
    data->air_level = MQ135_GetLevel(air);
    data->mq135_valid = 1U;
  }
  else
  {
    data->mq135_valid = 0U;
  }

  /* 温湿度 */
  if (DHT11_Read(&temperature, &humidity) == SENSOR_OK)
  {
    data->temperature = temperature;
    data->humidity = humidity;
    data->dht11_valid = 1U;
  }
  else
  {
    data->dht11_valid = 0U;
  }

  /* 光照 */
  if (BH1750_Read(&light) == SENSOR_OK)
  {
    data->light = light;
    data->bh1750_valid = 1U;
  }
  else
  {
    data->bh1750_valid = 0U;
  }
}
