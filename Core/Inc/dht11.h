#ifndef __DHT11_H
#define __DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sensor_status.h"

/* ===== 接线：DATA 接 PE7（J2 第 43 脚）=====
 * 换引脚只改这两行 + 在 CubeMX 里把新脚配成 Output
 * 模块自带上拉；若是裸传感器，DATA 与 VCC 之间加 4.7k~10k 上拉电阻
 */
#define DHT11_GPIO_PORT   GPIOE
#define DHT11_GPIO_PIN    GPIO_PIN_7

/* 初始化数据线（拉高总线，等传感器稳定） */
Sensor_Status_t DHT11_Init(void);

/* 读取温湿度：成功时写入 *temperature / *humidity（单位 ℃ / %RH） */
Sensor_Status_t DHT11_Read(float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif

#endif /* __DHT11_H */
