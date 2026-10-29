# STM32 环境参数监测系统（Environment Sensor）

基于 **STM32F103ZET6 + 巴法云** 的物联网环境监测节点：采集温度、湿度、光照和空气质量，
本地 OLED 实时显示，同时通过 ESP8266 周期上报到巴法云平台。

> 课程设计 / 嵌入式项目管理课程项目

---

## 功能特性

| 功能 | 说明 |
|---|---|
| 温湿度采集 | DHT11 单总线，2 秒一次，含校验和验证与超时保护 |
| 光照采集 | BH1750 数字光照，软件 I2C，原始值按 1.2 换算为 Lux |
| 空气质量 | MQ135 模拟量，ADC 10 次采样取均值，三档分级 |
| 本地显示 | SSD1306 128×64 OLED（I2C），四行实时显示 |
| 调试输出 | USART1 115200，printf 重定向 |
| 云端上报 | ESP8266 AT 指令 + 巴法云 TCP 8344，每 10 秒一次 |
| 异常降级 | 单路传感器失败不影响其他通道，对应行显示 ERROR |
| 断线重连 | 连续上传失败 3 次后自动重连 WiFi（10 秒重试间隔） |

---

## 硬件清单

| 器件 | 型号 | 接口 |
|---|---|---|
| 主控 | STM32F103ZET6（144 脚核心板） | — |
| 温湿度 | DHT11 | 单总线 |
| 光照 | BH1750 | I2C |
| 空气质量 | MQ135 | ADC |
| 显示 | SSD1306 0.96" OLED | I2C |
| 无线 | ESP8266（ESP-01S） | UART |

## 引脚分配

| 外设 | 引脚 | 说明 |
|---|---|---|
| DHT11 DATA | **PE7** | 单总线，需上拉 |
| BH1750 SCL / SDA | **PB6 / PB7** | 与 OLED 共用软件 I2C 总线 |
| OLED SCL / SDA | **PB6 / PB7** | 地址 0x78 |
| MQ135 AO | **PA1** | ADC1_IN1，12 位 |
| USART1 TX / RX | **PA9 / PA10** | 调试串口，115200 |
| USART3 TX / RX | **PB10 / PB11** | ESP8266，115200 |
| SWD | PA13 / PA14 | 下载调试 |

**系统时钟**：HSE 8MHz × PLL9 = 72MHz，APB1 = 36MHz，ADC 时钟 12MHz（PCLK2/6）。

> ⚠️ ESP8266 必须**独立 3.3V 供电**（峰值 300mA+）。接核心板 3.3V 会因为供电不足
> 导致「能通信但扫描不到 WiFi」（`+CWJAP:3`）。

---

## 软件架构

```
              ┌──────────────────────┐
              │ Environment_Data_t   │  统一数据容器
              └──────────┬───────────┘
      ┌──────────────────┼──────────────────┐
 ┌────▼────┐      ┌──────▼──────┐     ┌─────▼──────┐
 │  OLED   │      │  串口 printf │     │ 巴法云上传  │
 └─────────┘      └─────────────┘     └────────────┘
      ▲
      │ EnvSensor_ReadAll(&env)
      │
 ┌────┴─────┬────────────┬─────────────┐
 │  MQ135   │   DHT11    │   BH1750    │
 │ (ADC)    │ (单总线)    │  (I2C)      │
 └──────────┴────────────┴─────────────┘
```

驱动层只提供「能力」，不知道业务；`main.c` 只做调度。

### 目录结构

```
Core/
├── Inc/
│   ├── main.h
│   ├── sensor_status.h        # 统一状态码 OK/ERROR/TIMEOUT/DATA_INVALID
│   ├── env_sensor.h           # 聚合层
│   ├── mq135.h  dht11.h  bh1750.h
│   ├── oled_i2c.h  soft_i2c.h
│   ├── esp8266.h  bemfa_cloud.h
│   └── bemfa_config.example.h # 配置模板（复制为 bemfa_config.h）
└── Src/
    ├── main.c                 # 调度
    ├── env_sensor.c mq135.c dht11.c bh1750.c
    ├── oled_i2c.c soft_i2c.c
    └── esp8266.c bemfa_cloud.c
```

---

## 编译

### 环境

- CMake ≥ 3.22
- Ninja
- arm-none-eabi-gcc（推荐 13.3 及以上）
- VS Code + STM32 扩展（或直接用命令行）

### 命令行

```bash
cmake --preset Debug
cmake --build build/Debug
```

产物：`build/Debug/project.hex` 与 `project.bin`。

> 使用 STM32CubeMX 6.17 生成，固件包 STM32Cube FW_F1 V1.8.7。
> 修改外设配置请打开 `project.ioc` 重新生成；手写代码都在 `USER CODE` 区，不会被覆盖。

## 烧录

串口 ISP（推荐，板载 CH340 支持一键下载）：

1. 用 FlyMcu / mcuisp 选择 `build/Debug/project.hex`
2. 勾选「校验」「编程后执行」，DTR/RTS 自动进入 Bootloader
3. 烧完复位即可

也可用 ST-Link + STM32CubeProgrammer 走 SWD。

---

## 配置

把 `Core/Inc/bemfa_config.example.h` 复制为 `Core/Inc/bemfa_config.h`，填入：

```c
#define BEMFA_WIFI_SSID       "你的 2.4G WiFi 名"
#define BEMFA_WIFI_PASSWORD   "密码"
#define BEMFA_UID             "巴法云 32 位私钥"
#define BEMFA_TOPIC           "sensor004"
```

`bemfa_config.h` 已被 `.gitignore` 排除，不会上传到仓库。

巴法云：[cloud.bemfa.com](https://cloud.bemfa.com) → 控制台 → TCP 创客云 → 新建主题。

## 上报格式

```
cmd=2&uid=<私钥>&topic=<主题>&msg=temp=29.0,humi=58.0,light=85.8,air=1100,level=GOOD
```

某一路传感器失败时，对应字段上报 `ERR`。

---

## 运行效果

```
Air: 1100 GOOD
Temp: 29.0C
Humi: 58.0%
Lux: 85.8
```

串口同步输出，云端每 10 秒收到一条数据。

---

## 已知限制

- 采样周期 2 秒（DHT11 要求 ≥1 秒），上传周期 10 秒
- 上传时主循环阻塞 0.5~2 秒（TCP 建连 + 发送）
- MQ135 为广谱气敏元件，三档阈值仅用于演示，非标定浓度
- 需要 2.4GHz WiFi，不支持 5GHz / 网页认证网络

## 许可

[MIT License](LICENSE) © 2026 YiXingNanSheng
