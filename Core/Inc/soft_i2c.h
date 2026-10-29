#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ===== 总线引脚：OLED 与 BH1750 共用同一条 I2C ===== */
#define SOFT_I2C_SCL_PORT   GPIOB
#define SOFT_I2C_SCL_PIN    GPIO_PIN_6
#define SOFT_I2C_SDA_PORT   GPIOB
#define SOFT_I2C_SDA_PIN    GPIO_PIN_7

/* 引脚需在 CubeMX 中配成 Output Open Drain + Pull-up */

void SoftI2C_Init(void);

void SoftI2C_Start(void);
void SoftI2C_Stop(void);

/* 发送一个字节；返回 1 = 收到 ACK，0 = 无应答（设备不在） */
uint8_t SoftI2C_SendByte(uint8_t byte);

/* 读一个字节；ack = 1 时回 ACK，最后一个字节传 0 回 NACK */
uint8_t SoftI2C_ReadByte(uint8_t ack);

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_I2C_H */
