#ifndef __BEMFA_CONFIG_H
#define __BEMFA_CONFIG_H

/*
 * 配置模板。
 *
 * 使用方法：
 *   1. 把本文件复制为 bemfa_config.h（同目录）
 *   2. 填入你自己的 WiFi 和巴法云信息
 *
 * bemfa_config.h 已被 .gitignore 排除，不会上传到仓库，
 * 这样密码和私钥就不会泄漏。
 */

/* 2.4GHz WiFi（ESP8266 不支持 5G） */
#define BEMFA_WIFI_SSID       "YOUR_WIFI_SSID"
#define BEMFA_WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"   /* 开放网络留空 "" */

/* 巴法云：控制台 → 私钥（32 位）/ 主题名 */
#define BEMFA_UID             "YOUR_32BIT_UID"
#define BEMFA_TOPIC           "sensor004"

#endif /* __BEMFA_CONFIG_H */
