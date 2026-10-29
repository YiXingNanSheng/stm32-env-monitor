#ifndef __ENV_SENSOR_H
#define __ENV_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sensor_status.h"
#include "mq135.h"      /* Air_Level_t */

/*
 * 全系统统一的环境数据容器。
 * 采集层负责填充，显示层 / 串口层 / 上传层都只读它，
 * 上层不需要知道数据来自哪个驱动。
 */
typedef struct
{
  /* 采集结果 */
  float temperature;        /* ℃ */
  float humidity;           /* %RH */
  float light;              /* Lux */
  uint16_t air_quality;     /* ADC 原始值 0~4095 */
  Air_Level_t air_level;    /* 空气质量等级 */

  /* 本次读取是否成功：1 = 有效，0 = 该通道失败 */
  uint8_t dht11_valid;
  uint8_t bh1750_valid;
  uint8_t mq135_valid;
} Environment_Data_t;

/* 清空数据与有效标志 */
void EnvSensor_Init(Environment_Data_t *data);

/* 一次性读取全部传感器，失败只清对应 valid 位，不影响其他通道 */
void EnvSensor_ReadAll(Environment_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __ENV_SENSOR_H */
