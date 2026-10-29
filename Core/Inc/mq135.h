#ifndef __MQ135_H
#define __MQ135_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sensor_status.h"

/* 一次读取的采样次数（取平均，抑制模拟噪声） */
#define MQ135_SAMPLE_COUNT   10U

/* 空气质量等级：课堂演示阈值，不是标定过的 ppm 浓度 */
typedef enum
{
  AIR_LEVEL_GOOD = 0,
  AIR_LEVEL_NORMAL,
  AIR_LEVEL_WARNING
} Air_Level_t;

/*
 * hadc1 由 CubeMX 生成在 main.c 里。
 * 这里声明为外部变量，驱动文件才能访问它。
 * （若以后在 CubeMX 里勾选"每个外设生成一对 .c/.h"，
 *   则改为 #include "adc.h" 并删掉这行）
 */
extern ADC_HandleTypeDef hadc1;

/* 校准 ADC（上电后调用一次） */
Sensor_Status_t MQ135_Init(void);

/* 采样 MQ135_SAMPLE_COUNT 次并取平均，结果写入 *adc_value（0~4095） */
Sensor_Status_t MQ135_Read(uint16_t *adc_value);

/* 按阈值把 ADC 值换算成等级 */
Air_Level_t MQ135_GetLevel(uint16_t adc_value);

/* 等级转字符串：GOOD / NORMAL / WARN */
const char *MQ135_GetLevelText(Air_Level_t level);

#ifdef __cplusplus
}
#endif

#endif /* __MQ135_H */
