#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sensor_status.h"

/*
 * ESP8266 AT 指令收发层（挂在 USART3：PB10=TX, PB11=RX）
 *
 * 接线：
 *   ESP VCC  -> 3.3V（独立供电，峰值 300mA+）
 *   ESP GND  -> GND（必须共地）
 *   ESP TX   -> PB11 (USART3_RX)
 *   ESP RX   -> PB10 (USART3_TX)
 *   ESP CH_PD/EN -> 3.3V（不拉高模块不工作）
 *   ESP RST  -> 3.3V（可选）
 *   GPIO0 必须悬空或拉高（拉低会进下载模式）
 */
#define ESP8266_RX_BUF_SIZE   768U

/* 清空接收缓冲与串口残留数据 */
void ESP8266_Init(void);

/* 发送一条 AT 指令，并等待指定关键字（如 "OK"、"WIFI GOT IP"） */
Sensor_Status_t ESP8266_SendCommand(const char *cmd, const char *ack, uint32_t timeout_ms);

/* 只发不等（用于 CIPSEND 之后发送数据体） */
Sensor_Status_t ESP8266_SendRaw(const char *data);

/* 读取模块最近一次返回的原始内容（调试用） */
const char *ESP8266_GetLastResponse(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_H */
