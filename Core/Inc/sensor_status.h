#ifndef __SENSOR_STATUS_H
#define __SENSOR_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* 全项目统一的状态码：所有传感器驱动都返回这四种值 */
typedef enum
{
  SENSOR_OK = 0,        /* 成功，输出数据有效 */
  SENSOR_ERROR,         /* 通用错误（空指针、外设初始化失败等） */
  SENSOR_TIMEOUT,       /* 在给定时间内没有等到应答 */
  SENSOR_DATA_INVALID   /* 收到了数据，但校验或范围判断失败 */
} Sensor_Status_t;

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_STATUS_H */
