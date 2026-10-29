/**
  ******************************************************************************
  * @file    soft_i2c.c
  * @brief   软件 I2C 主机（开漏输出，SCL/SDA 由宏在 soft_i2c.h 中定义）
  *          OLED(0x78) 与 BH1750(0x46/0x47) 共用这条总线
  ******************************************************************************
  */

#include "soft_i2c.h"

#define SCL_H()  HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_SET)
#define SCL_L()  HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_RESET)
#define SDA_H()  HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_SET)
#define SDA_L()  HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_RESET)
#define SDA_READ()  HAL_GPIO_ReadPin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN)

/* 约 2~3us，对应 100~200kHz 的总线速度 */
static void IIC_Delay(void)
{
  for (volatile int i = 0; i < 40; i++)
  {
  }
}

void SoftI2C_Init(void)
{
  SCL_H();
  SDA_H();
}

void SoftI2C_Start(void)
{
  SDA_H();
  SCL_H();
  IIC_Delay();
  SDA_L();          /* SCL 高时 SDA 由高变低 = 起始 */
  IIC_Delay();
  SCL_L();
}

void SoftI2C_Stop(void)
{
  SCL_L();
  SDA_L();
  IIC_Delay();
  SCL_H();
  IIC_Delay();
  SDA_H();          /* SCL 高时 SDA 由低变高 = 停止 */
  IIC_Delay();
}

uint8_t SoftI2C_SendByte(uint8_t byte)
{
  uint8_t i;

  for (i = 0; i < 8U; i++)
  {
    if (byte & 0x80U)
    {
      SDA_H();
    }
    else
    {
      SDA_L();
    }
    byte <<= 1;

    IIC_Delay();
    SCL_H();
    IIC_Delay();
    SCL_L();
  }

  /* 第 9 个时钟：释放 SDA，采集从机应答（低电平 = ACK） */
  SDA_H();
  IIC_Delay();
  SCL_H();
  IIC_Delay();

  {
    uint8_t ack = (SDA_READ() == GPIO_PIN_RESET) ? 1U : 0U;
    SCL_L();
    IIC_Delay();
    return ack;
  }
}

uint8_t SoftI2C_ReadByte(uint8_t ack)
{
  uint8_t i;
  uint8_t data = 0U;

  SDA_H();          /* 主机释放 SDA，交给从机驱动 */

  for (i = 0; i < 8U; i++)
  {
    IIC_Delay();
    SCL_H();
    IIC_Delay();

    data <<= 1;
    if (SDA_READ() == GPIO_PIN_SET)
    {
      data |= 0x01U;
    }

    SCL_L();
  }

  /* 第 9 个时钟：主机回 ACK(拉低) 或 NACK(放开) */
  if (ack != 0U)
  {
    SDA_L();
  }
  else
  {
    SDA_H();
  }

  IIC_Delay();
  SCL_H();
  IIC_Delay();
  SCL_L();
  SDA_H();

  return data;
}
