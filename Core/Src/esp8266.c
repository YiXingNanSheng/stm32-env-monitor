/**
  ******************************************************************************
  * @file    esp8266.c
  * @brief   ESP8266 AT 指令收发层（USART3，PB10/PB11）
  ******************************************************************************
  */

#include "esp8266.h"
#include <string.h>

extern UART_HandleTypeDef huart3;

static uint8_t esp_rx_buffer[ESP8266_RX_BUF_SIZE];

/* 把串口里已经收到的数据全部读掉 */
void ESP8266_ClearRx(void)
{
  uint8_t ch;

  while (HAL_UART_Receive(&huart3, &ch, 1U, 1U) == HAL_OK)
  {
  }

  memset(esp_rx_buffer, 0, sizeof(esp_rx_buffer));
}

void ESP8266_Init(void)
{
  ESP8266_ClearRx();
}

const char *ESP8266_GetLastResponse(void)
{
  return (const char *)esp_rx_buffer;
}

/* 逐字符收，直到出现 ack 关键字、出现 ERROR、或超时 */
static Sensor_Status_t ESP8266_WaitFor(const char *ack, uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  uint16_t index = 0U;
  uint8_t ch;

  memset(esp_rx_buffer, 0, sizeof(esp_rx_buffer));

  while ((HAL_GetTick() - start) < timeout_ms)
  {
    if (HAL_UART_Receive(&huart3, &ch, 1U, 10U) != HAL_OK)
    {
      continue;
    }

    if (index < (ESP8266_RX_BUF_SIZE - 1U))
    {
      esp_rx_buffer[index] = ch;
      index++;
      esp_rx_buffer[index] = '\0';
    }

    if ((ack != 0) && (strstr((char *)esp_rx_buffer, ack) != 0))
    {
      return SENSOR_OK;
    }

    if (strstr((char *)esp_rx_buffer, "ERROR") != 0)
    {
      return SENSOR_ERROR;
    }

    if (strstr((char *)esp_rx_buffer, "FAIL") != 0)
    {
      return SENSOR_ERROR;
    }
  }

  return SENSOR_TIMEOUT;
}

Sensor_Status_t ESP8266_SendCommand(const char *cmd, const char *ack, uint32_t timeout_ms)
{
  if (cmd == 0)
  {
    return SENSOR_ERROR;
  }

  ESP8266_ClearRx();
  HAL_UART_Transmit(&huart3, (uint8_t *)cmd, (uint16_t)strlen(cmd), 1000U);

  if (ack == 0)
  {
    return SENSOR_OK;
  }

  return ESP8266_WaitFor(ack, timeout_ms);
}

Sensor_Status_t ESP8266_SendRaw(const char *data)
{
  if (data == 0)
  {
    return SENSOR_ERROR;
  }

  HAL_UART_Transmit(&huart3, (uint8_t *)data, (uint16_t)strlen(data), 1000U);
  return SENSOR_OK;
}
