/**
  ******************************************************************************
  * @file    bemfa_cloud.c
  * @brief   通过 ESP8266 把环境数据上报到巴法云（TCP 8344，cmd=2 报文）
  ******************************************************************************
  */

#include "bemfa_cloud.h"
#include "esp8266.h"
#include <stdio.h>
#include <string.h>

/* 打开到巴法云的 TCP 连接 */
static Sensor_Status_t BemfaCloud_OpenTcp(void)
{
  char cmd[96];

  snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n",
           BEMFA_SERVER, (unsigned int)BEMFA_PORT);

  if (ESP8266_SendCommand(cmd, "CONNECT", 5000U) == SENSOR_OK)
  {
    return SENSOR_OK;
  }

  /* 已经连着的情况，模块会回 ALREADY CONNECTED，也算成功 */
  if (strstr(ESP8266_GetLastResponse(), "ALREADY") != 0)
  {
    return SENSOR_OK;
  }

  return SENSOR_ERROR;
}

/* 调试开关：调通 WiFi 后改成 0，可去掉串口里的 AT 细节 */
#define BEMFA_DEBUG   0

static Sensor_Status_t BemfaCloud_Step(const char *label, const char *cmd,
                                       const char *ack, uint32_t timeout_ms)
{
  Sensor_Status_t st = ESP8266_SendCommand(cmd, ack, timeout_ms);

#if BEMFA_DEBUG
  const char *state = "ERROR";
  if (st == SENSOR_OK)      { state = "OK"; }
  else if (st == SENSOR_TIMEOUT) { state = "TIMEOUT"; }

  printf("[%s] %s | resp: %s\r\n", label, state, ESP8266_GetLastResponse());
#else
  (void)label;
#endif

  return st;
}

Sensor_Status_t BemfaCloud_Init(void)
{
  char cmd[128];

  ESP8266_Init();

  /* 1. 看模块在不在 */
  if (BemfaCloud_Step("AT", "AT\r\n", "OK", 1000U) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  /* 2. 关回显，减少接收内容干扰 */
  if (BemfaCloud_Step("ATE0", "ATE0\r\n", "OK", 1000U) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  /* 3. 设为 Station 模式 */
  if (BemfaCloud_Step("CWMODE", "AT+CWMODE=1\r\n", "OK", 1000U) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  /* 4. 连 WiFi：密码错回 FAIL，找不到热点则超时 */
#if BEMFA_DEBUG
  /* 临时：扫描附近 2.4G 热点，看目标 SSID 在不在列表里 */
  (void)BemfaCloud_Step("CWLAP", "AT+CWLAP\r\n", "OK", 10000U);

  printf("[CWJAP] try ssid=\"%s\"\r\n", BEMFA_WIFI_SSID);
#endif

  /* 开放网络（无密码）和 WPA2 网络的指令格式不同 */
  if (BEMFA_WIFI_PASSWORD[0] == '\0')
  {
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\"\r\n", BEMFA_WIFI_SSID);
  }
  else
  {
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n",
             BEMFA_WIFI_SSID, BEMFA_WIFI_PASSWORD);
  }

  /*
   * 注意：等待 "OK" 而不是 "WIFI GOT IP"。
   * 已经连着同一个热点时，模块只回 "OK"，不会再回 "WIFI GOT IP"；
   * 只认后者会导致重连永远失败（这是之前的 bug）。
   * 密码错会回 FAIL，找不到热点会超时，都能区分。
   */
  if (BemfaCloud_Step("CWJAP", cmd, "OK", 15000U) != SENSOR_OK)
  {
    HAL_Delay(1000);
    if (BemfaCloud_Step("CWJAP2", cmd, "OK", 15000U) != SENSOR_OK)
    {
      return SENSOR_ERROR;
    }
  }

  return SENSOR_OK;
}

Sensor_Status_t BemfaCloud_Upload(const Environment_Data_t *data)
{
  char msg[96];
  char payload[192];
  char cmd[32];
  char tbuf[12];
  char hbuf[12];
  char lbuf[12];
  char abuf[8];
  const char *level;
  uint16_t length;

  if (data == 0)
  {
    return SENSOR_ERROR;
  }

  /* 每路单独判断：失败的那一路上报 "ERR"，其他照常上报 */
  if (data->dht11_valid != 0U)
  {
    uint32_t t10 = (uint32_t)(data->temperature * 10.0f + 0.5f);
    uint32_t h10 = (uint32_t)(data->humidity * 10.0f + 0.5f);
    snprintf(tbuf, sizeof(tbuf), "%u.%u",
             (unsigned int)(t10 / 10U), (unsigned int)(t10 % 10U));
    snprintf(hbuf, sizeof(hbuf), "%u.%u",
             (unsigned int)(h10 / 10U), (unsigned int)(h10 % 10U));
  }
  else
  {
    snprintf(tbuf, sizeof(tbuf), "ERR");
    snprintf(hbuf, sizeof(hbuf), "ERR");
  }

  if (data->bh1750_valid != 0U)
  {
    uint32_t l10 = (uint32_t)(data->light * 10.0f + 0.5f);
    snprintf(lbuf, sizeof(lbuf), "%u.%u",
             (unsigned int)(l10 / 10U), (unsigned int)(l10 % 10U));
  }
  else
  {
    snprintf(lbuf, sizeof(lbuf), "ERR");
  }

  if (data->mq135_valid != 0U)
  {
    snprintf(abuf, sizeof(abuf), "%u", (unsigned int)data->air_quality);
    level = MQ135_GetLevelText(data->air_level);
  }
  else
  {
    snprintf(abuf, sizeof(abuf), "ERR");
    level = "ERR";
  }

  /* 上报内容：固定字段顺序，方便以后解析 */
  snprintf(msg, sizeof(msg), "temp=%s,humi=%s,light=%s,air=%s,level=%s",
           tbuf, hbuf, lbuf, abuf, level);

  /* 巴法云 TCP 上报格式：cmd=2&uid=私钥&topic=主题&msg=内容 */
  snprintf(payload, sizeof(payload), "cmd=2&uid=%s&topic=%s&msg=%s\r\n",
           BEMFA_UID, BEMFA_TOPIC, msg);

  if (BemfaCloud_OpenTcp() != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  /* 告诉模块要发多少字节，模块回 ">" 后才能真正发数据 */
  length = (uint16_t)strlen(payload);
  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", (unsigned int)length);

  if (ESP8266_SendCommand(cmd, ">", 3000U) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  if (ESP8266_SendCommand(payload, "SEND OK", 5000U) != SENSOR_OK)
  {
    return SENSOR_ERROR;
  }

  (void)ESP8266_SendCommand("AT+CIPCLOSE\r\n", "OK", 1000U);

  return SENSOR_OK;
}
