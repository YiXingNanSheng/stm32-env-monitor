/**
  ******************************************************************************
  * @file    dht11.c
  * @brief   DHT11 温湿度传感器驱动（单总线，DATA -> PE7）
  ******************************************************************************
  */

#include "dht11.h"

/* ---------------------------------------------------------------- 微秒延时 */
/* HAL_Delay 最小 1ms，不够用；这里用 Cortex-M3 的 DWT 周期计数器 */
static void DHT11_DelayUs(uint32_t us)
{
  uint32_t start;
  uint32_t ticks;

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  start = DWT->CYCCNT;
  ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000U);
  while ((DWT->CYCCNT - start) < ticks)
  {
  }
}

/* ---------------------------------------------------------------- 引脚切换 */
static void DHT11_SetOutput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

static void DHT11_SetInput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/* ---------------------------------------------------------------- 等电平 */
/* 等 DATA 线变成 state，超时返回 SENSOR_TIMEOUT（防止断线时死等） */
static Sensor_Status_t DHT11_WaitPin(GPIO_PinState state, uint32_t timeout_us)
{
  while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) != state)
  {
    if (timeout_us == 0U)
    {
      return SENSOR_TIMEOUT;
    }
    timeout_us--;
    DHT11_DelayUs(1);
  }

  return SENSOR_OK;
}

/* ---------------------------------------------------------------- 读一个字节 */
/* 每一位 = 50us 低电平 + 高电平；高电平 26~28us 表示 0，约 70us 表示 1 */
static Sensor_Status_t DHT11_ReadByte(uint8_t *data)
{
  uint8_t i;
  uint8_t value = 0U;

  for (i = 0; i < 8U; i++)
  {
    if (DHT11_WaitPin(GPIO_PIN_SET, 100U) != SENSOR_OK)   /* 等低电平结束 */
    {
      return SENSOR_TIMEOUT;
    }

    DHT11_DelayUs(40);                                   /* 40us 后采样 */
    value <<= 1;
    if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
    {
      value |= 0x01U;                                    /* 还是高 -> 1 */
    }

    if (DHT11_WaitPin(GPIO_PIN_RESET, 100U) != SENSOR_OK) /* 等高电平结束 */
    {
      return SENSOR_TIMEOUT;
    }
  }

  *data = value;
  return SENSOR_OK;
}

/* ---------------------------------------------------------------- 公开接口 */
Sensor_Status_t DHT11_Init(void)
{
  DHT11_SetOutput();
  HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
  HAL_Delay(1000);          /* 跳过上电不稳定期 */
  return SENSOR_OK;
}

Sensor_Status_t DHT11_Read(float *temperature, float *humidity)
{
  uint8_t buffer[5] = {0};
  uint8_t i;
  uint8_t checksum;

  if ((temperature == 0) || (humidity == 0))
  {
    return SENSOR_ERROR;
  }

  /* 1. 起始信号：拉低至少 18ms */
  DHT11_SetOutput();
  HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET);
  HAL_Delay(20);

  /* 2. 拉高 30us 后释放总线，交给传感器 */
  HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
  DHT11_DelayUs(30);
  DHT11_SetInput();

  /* 3. 等传感器应答：低 80us -> 高 80us -> 低 80us */
  if (DHT11_WaitPin(GPIO_PIN_RESET, 100U) != SENSOR_OK)
  {
    return SENSOR_TIMEOUT;
  }
  if (DHT11_WaitPin(GPIO_PIN_SET, 100U) != SENSOR_OK)
  {
    return SENSOR_TIMEOUT;
  }
  if (DHT11_WaitPin(GPIO_PIN_RESET, 100U) != SENSOR_OK)
  {
    return SENSOR_TIMEOUT;
  }

  /* 4. 连续读 5 个字节：湿度整数/小数、温度整数/小数、校验和 */
  for (i = 0; i < 5U; i++)
  {
    if (DHT11_ReadByte(&buffer[i]) != SENSOR_OK)
    {
      return SENSOR_TIMEOUT;
    }
  }

  /* 5. 校验：前 4 字节之和的低 8 位 == 第 5 字节 */
  checksum = (uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3]);
  if (checksum != buffer[4])
  {
    return SENSOR_DATA_INVALID;
  }

  /* DHT11 的小数字节通常为 0，这里仍按 0.1 精度拼接，兼容 DHT22 类格式 */
  *humidity = (float)buffer[0] + ((float)buffer[1] / 10.0f);
  *temperature = (float)buffer[2] + ((float)buffer[3] / 10.0f);

  return SENSOR_OK;
}
