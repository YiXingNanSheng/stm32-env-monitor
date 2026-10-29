/**
  ******************************************************************************
  * @file    bh1750.c
  * @brief   BH1750 光照传感器驱动（I2C，与 OLED 共用 PB6/PB7 总线）
  ******************************************************************************
  */

#include "bh1750.h"
#include "soft_i2c.h"

/* BH1750 命令字 */
#define BH1750_POWER_ON        0x01U
#define BH1750_RESET           0x07U
#define BH1750_CONT_H_RES_MODE 0x10U   /* 连续高分辨率：1 lx，约 120ms 一次 */

static Sensor_Status_t BH1750_WriteCmd(uint8_t cmd)
{
  SoftI2C_Start();

  if (SoftI2C_SendByte(BH1750_ADDR_W) == 0U)   /* 没有 ACK = 设备不在 */
  {
    SoftI2C_Stop();
    return SENSOR_ERROR;
  }

  if (SoftI2C_SendByte(cmd) == 0U)
  {
    SoftI2C_Stop();
    return SENSOR_ERROR;
  }

  SoftI2C_Stop();
  return SENSOR_OK;
}

Sensor_Status_t BH1750_Init(void)
{
  /* 1. 上电 */
  if (BH1750_WriteCmd(BH1750_POWER_ON) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }
  HAL_Delay(10);

  /* 2. 清掉上电时的旧测量值 */
  (void)BH1750_WriteCmd(BH1750_RESET);
  HAL_Delay(10);

  /* 3. 进入连续测量模式，之后芯片自己一直刷新，读取随时拿最新值 */
  if (BH1750_WriteCmd(BH1750_CONT_H_RES_MODE) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  HAL_Delay(180);   /* 第一次测量需要约 180ms 才有效 */
  return SENSOR_OK;
}

Sensor_Status_t BH1750_Read(float *lux)
{
  uint8_t high_byte;
  uint8_t low_byte;
  uint16_t raw;

  if (lux == 0)
  {
    return SENSOR_ERROR;
  }

  SoftI2C_Start();

  if (SoftI2C_SendByte(BH1750_ADDR_R) == 0U)
  {
    SoftI2C_Stop();
    return SENSOR_ERROR;
  }

  high_byte = SoftI2C_ReadByte(1U);   /* 回 ACK，还有一字节 */
  low_byte  = SoftI2C_ReadByte(0U);   /* 最后一字节回 NACK */
  SoftI2C_Stop();

  raw = (uint16_t)(((uint16_t)high_byte << 8) | low_byte);

  /* 手册：Lux = 原始值 / 1.2 */
  *lux = (float)raw / 1.2f;

  return SENSOR_OK;
}
