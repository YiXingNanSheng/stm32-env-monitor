#ifndef __BH1750_H
#define __BH1750_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sensor_status.h"

/*
 * BH1750 光照传感器（I2C）
 * 接线：VCC -> 3.3V, GND -> GND, SCL -> PB6, SDA -> PB7（与 OLED 同一条总线）
 * ADDR 脚接 GND 或悬空 -> 写地址 0x46 / 读地址 0x47
 */
#define BH1750_ADDR_W   0x46U
#define BH1750_ADDR_R   0x47U

/* 上电 + 设为连续高分辨率模式（1 lx，测量周期约 120ms） */
Sensor_Status_t BH1750_Init(void);

/* 读取光照强度，单位 Lux（原始值 / 1.2） */
Sensor_Status_t BH1750_Read(float *lux);

#ifdef __cplusplus
}
#endif

#endif /* __BH1750_H */
