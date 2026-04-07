# DuduClock_Firmware

这是已经适配到 `ESP32-S3 N16R8 + ST7789 240x320 SPI 屏` 的 Dudu 天气时钟原始仓库版本。

当前这份代码已经完成了这些适配：

- `ESP32-S3`
- `ST7789`
- `HSPI`
- `ESP32 core 3.3.x` 新版 `timer` / `LEDC` API
- 更稳定的 `NetworkClientSecure` 联网方式
- 和风 `gzip` 响应解压
- 空气质量接口迁移到新版 `Air Quality API v1`
- 页面切换按键改为 `GPIO0`，也就是开发板 `BOOT` 键

## 已验证硬件

- 开发板：`ESP32-S3 N16R8`
- 屏幕驱动：`ST7789`
- 屏幕分辨率：`240x320`
- 屏幕通信：`SPI`
- SPI Host：`HSPI`

## 屏幕接线

屏幕配置在 [tft_setup.h](/Users/wangshixiao/Downloads/DuduClock_Firmware/tft_setup.h)：

- `BL` -> `GPIO42`
- `MOSI` -> `GPIO47`
- `SCLK` -> `GPIO21`
- `DC` -> `GPIO40`
- `RST` -> `GPIO45`
- `CS` -> `GPIO41`

当前默认参数：

- `TFT_MISO = -1`
- `TFT_RGB_ORDER = TFT_BGR`
- `SPI_FREQUENCY = 10000000`

如果出现白屏、花屏、颜色不对，优先检查：

1. 是否仍然使用 [tft_setup.h](/Users/wangshixiao/Downloads/DuduClock_Firmware/tft_setup.h) 里的 `ST7789_DRIVER`
2. 是否保留 `USE_HSPI_PORT`
3. 屏幕接线是否完全一致
4. 是否需要把 `TFT_RGB_ORDER` 改成 `TFT_RGB`
5. 是否需要把 `SPI_FREQUENCY` 降到 `8000000` 或 `4000000`

## 按键说明

当前页面切换按键定义在 [common.h](/Users/wangshixiao/Downloads/DuduClock_Firmware/common.h)：

- `BUTTON = 0`

也就是使用 `ESP32-S3` 开发板上的 `BOOT` 键。

按键行为：

- 单击：计时器页里开始/暂停
- 双击：切换页面
- 长按：在特定页面执行功能，比如恢复出厂、切换主题

注意：

- `GPIO0` 是启动相关引脚
- 正常运行时拿它当按键没问题
- 但如果在上电或复位瞬间一直按住 `BOOT`，可能会进入下载模式

## Arduino IDE 参数

已验证可编译、可启动的参数：

- `Board`: `ESP32S3 Dev Module`
- `Flash Size`: `16MB`
- `PSRAM`: `OPI PSRAM`
- `Partition Scheme`: `Huge APP (3MB No OTA/1MB SPIFFS)`

如果分区没选对，会直接出现固件过大：

- `text section exceeds available space in board`

## arduino-cli 编译命令

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3 \
  --board-options FlashSize=16M,PSRAM=opi,PartitionScheme=huge_app \
  /Users/wangshixiao/Downloads/DuduClock_Firmware
```

当前这版编译结果大约是：

- Flash：`2979830 / 3145728`，约 `94%`
- RAM：`50520 / 327680`，约 `15%`

## 联网方式

当前联网实现位于 [net.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/net.cpp)：

- 使用 `NetworkClientSecure`
- 使用 `Bearer JWT` 访问和风天气私有 Host
- 自动处理 `gzip`

之所以没有继续使用原来的 `HTTPClient` 路径，是因为在当前 `ESP32-S3 + core 3.3.x` 环境下，它更容易出现启动期请求异常或不稳定现象。

## 和风配置

默认认证参数在 [net.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/net.cpp)：

- `PrivateKey`
- `PublicKey`
- `KeyID`
- `ProjectID`
- `ApiHost`

系统启动时会优先从 NVS 读取这些值；如果 NVS 里没有，就自动回退到源码里的默认值。

相关逻辑在 [preferencesUtil.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/preferencesUtil.cpp)。

## 测试配置

测试写入函数在 [preferencesUtil.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/preferencesUtil.cpp)：

```cpp
void setInfo4Test(){
  prefs.putString("ssid", "yunjiu");
  prefs.putString("pass", "610610610");
  prefs.putString("city", "烟台");
  prefs.putString("adm", "");
  prefs.putString("location", "101120501");
  prefs.putString("latitude", "37.46");
  prefs.putString("longitude", "121.45");
}
```

如果你想直接使用测试配置：

1. 在 [DuduClock_Firmware.ino](/Users/wangshixiao/Downloads/DuduClock_Firmware/DuduClock_Firmware.ino) 里取消注释 `setInfo4Test();`
2. 或者把上面的 WiFi / 城市信息改成你自己的

## 天气接口

城市查询和天气接口定义在 [common.h](/Users/wangshixiao/Downloads/DuduClock_Firmware/common.h)：

- 城市查询：`/geo/v2/city/lookup`
- 实时天气：`/v7/weather/now`
- 7 日天气：`/v7/weather/7d`

空气质量当前已经不再使用旧的 `v7 air now` 返回逻辑，代码中实际调用的是：

- `/airquality/v1/current/{latitude}/{longitude}`

原因是旧的 `Web API v7 实时空气质量` 已弃用，当前环境下容易出现 `403`。

## 存储的配置项

当前 NVS 中会保存：

- `ssid`
- `pass`
- `city`
- `adm`
- `location`
- `latitude`
- `longitude`
- `publicKeyMm`
- `privateKeyMm`
- `keyID`
- `apiHost`
- `projectID`
- `backColor`

## 关键源码位置

- 入口文件：[DuduClock_Firmware.ino](/Users/wangshixiao/Downloads/DuduClock_Firmware/DuduClock_Firmware.ino)
- 屏幕配置：[tft_setup.h](/Users/wangshixiao/Downloads/DuduClock_Firmware/tft_setup.h)
- 全局常量：[common.h](/Users/wangshixiao/Downloads/DuduClock_Firmware/common.h)
- 网络逻辑：[net.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/net.cpp)
- 配置存储：[preferencesUtil.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/preferencesUtil.cpp)
- 页面与任务：[task.cpp](/Users/wangshixiao/Downloads/DuduClock_Firmware/task.cpp)

## 常见问题

### 1. 上电就重启

优先检查：

1. 是否使用 `ESP32S3 Dev Module`
2. 是否保留 `USE_HSPI_PORT`
3. 屏幕驱动是否仍为 `ST7789_DRIVER`
4. 屏幕接线是否正确

### 2. 编译报固件过大

说明 `Partition Scheme` 不是 `Huge APP`。

### 3. 联网成功但天气不刷新

优先检查：

1. 和风认证参数是否有效
2. `ApiHost` 是否正确
3. `location / latitude / longitude` 是否存在
4. 串口里是否出现 `请求实时天气错误`、`请求空气质量错误`

### 4. 按 `BOOT` 键后串口下载模式异常

这是 `GPIO0` 的正常特性。不要在上电或复位瞬间一直按住 `BOOT`。
