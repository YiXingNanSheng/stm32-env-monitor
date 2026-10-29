#ifndef __BEMFA_CLOUD_H
#define __BEMFA_CLOUD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sensor_status.h"
#include "env_sensor.h"

/* ==================================================================
 *                        配 置 区
 * 真实账号信息放在 bemfa_config.h（不进仓库），
 * 没有这个文件时用下面的占位值，保证编译能过。
 * ================================================================== */
#if defined(__has_include)
#  if __has_include("bemfa_config.h")
#    include "bemfa_config.h"
#  endif
#endif

#ifndef BEMFA_WIFI_SSID
#  define BEMFA_WIFI_SSID     "YOUR_WIFI_SSID"
#endif
#ifndef BEMFA_WIFI_PASSWORD
#  define BEMFA_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif
#ifndef BEMFA_UID
#  define BEMFA_UID           "YOUR_32BIT_UID"
#endif
#ifndef BEMFA_TOPIC
#  define BEMFA_TOPIC         "sensor004"
#endif

#define BEMFA_SERVER          "bemfa.com"
#define BEMFA_PORT            8344U
/* ================================================================== */

/* 连接 WiFi（AT -> ATE0 -> CWMODE=1 -> CWJAP） */
Sensor_Status_t BemfaCloud_Init(void);

/* 上报一次环境数据到巴法云主题（只传一个结构体指针） */
Sensor_Status_t BemfaCloud_Upload(const Environment_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __BEMFA_CLOUD_H */
