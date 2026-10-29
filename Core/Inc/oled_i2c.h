#ifndef __OLED_I2C_H
#define __OLED_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ===== 设备地址（引脚定义见 soft_i2c.h，与 BH1750 共用总线）===== */
#define OLED_I2C_ADDR   0x78

/* ===== 对外接口 ===== */
/* 初始化 SSD1306：发送配置序列 + 清屏 */
void OLED_Init(void);

/* 清空整个显存（128x64） */
void OLED_Clear(void);

/* 在 (x 列, page 页 0~7) 显示一个字符 */
void OLED_ShowChar(uint8_t x, uint8_t page, char ch);

/* 从 (x, page) 开始显示字符串 */
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);

/* 显示定长字符串：不足 width 个字符时自动补空格（用于消除残影） */
void OLED_ShowStringPad(uint8_t x, uint8_t page, const char *str, uint8_t width);

/* 显示定长数字，len 为位数，高位补 0 */
void OLED_ShowNum(uint8_t x, uint8_t page, uint32_t num, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_I2C_H */
