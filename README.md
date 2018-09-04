# esp8266_aliyun_mqtt_app

基于ESP8266官方SDK接入[阿里云物联网平台](https://www.aliyun.com/product/iot)，只需填入阿里云设备认证三元组即可快速接入。

相关博客：https://blog.csdn.net/yannanxiu/article/details/81334230


## 本人所用的环境

- Windows 10
- [安信可 ESP 系列一体化开发环境](http://wiki.ai-thinker.com/ai_ide_install)（基于 Windows + Cygwin + Eclipse + GCC 的综合 IDE 环境）
- ESP8266_NONOS_SDK-2.2.1（理论上支持SDK 2.0+）


## 使用步骤

1. 首先到乐鑫[官网](https://www.espressif.com/zh-hans/products/hardware/esp8266ex/resources)或[github](https://github.com/espressif/ESP8266_NONOS_SDK)下载ESP8266_NONOS_SDK；
2. 下载本仓库，然后拷贝`aliyun_mqtt_app`文件夹到SDK主目录，用法类似于`ESP8266_NONOS_SDK/examples`里面的工程；
3. 最后导入工程即可（编译前建议先clean）。

![project_list](screenshot/project_list.png)


## 配置信息

当在阿里云物联网平台获取到设备认证三元组后，编辑`app/include/user_config.h`文件，修改下面信息，然后编译下载并重启ESP8266，最后应该就能在控制台看到设备上线了。

```C
#define PRODUCT_KEY     "PRODUCT_KEY"
#define DEVICE_NAME     "DEVICE_NAME"
#define DEVICE_SECRET   "DEVICE_SECRET"

#define WIFI_SSID       "WIFI_SSID"
#define WIFI_PASS       "WIFI_PASS"

//#define SMARTCONFIG_ENABLE
```

当打印出以下信息表示编译成功：

```
...
!!!
No boot needed.
Generate eagle.flash.bin and eagle.irom0text.bin successully in folder bin.
eagle.flash.bin-------->0x00000
eagle.irom0text.bin---->0x10000
!!!
```

## 主要模块说明

- `mqtt/`：mqtt主模块，从官方SDK中的`example/esp_mqtt_proj`项目移植过来的；
- `user/md5.c`：md5加密模块，生成阿里云mqtt password需要用到**hmacmd5**；
- `user/aliyun_mqtt.c`：生成阿里云mqtt信息的核心模块，主要包括`mqtt host`、`mqtt port`、`mqtt client id`、`mqtt username`以及动态生成的`mqtt password`；
- `user/user_smartconfig.c`：smartconfig模块，当定义了`SMARTCONFIG_ENABLE`可以以手机配网的方式给ESP8266连接Wi-Fi。


## 注意事项

- 关于串口

    ESP8266串口打印默认为74880bps，如果串口工具没有74880bps也可以选择76800bps。

    或者修改下`user/user_main.c/user_init()`的代码：

    ```C
    // ...
    void user_init(void) {
        uart_init(BIT_RATE_115200, BIT_RATE_115200);
        //...
    }
    ```

- 关于源码文件中的中文乱码

    因为源码文件编码默认为UTF-8，而Windows Eclipse IDE默认为GBK，所以可能需要设置一下：

    菜单栏Window -> Preferences -> General -> Workspace -> 面板Text file encoding -> 选择UTF-8 -> OK

- 关于wifi连接的状态led灯

    本项目使用了GPIO0作为wifi状态led灯，移植本项目时请留意。
    如果wifi连接成功，则保持led常亮（输出低电平）；否则进行闪烁。

    ```C
    /* wifi led 引脚配置 */
    #define WIFI_STATUS_LED_PIN         0
    ```

## 其他补充

- 如果想要学习使用ESP8266 mqtt原始app，可以参考我这篇[博客](https://blog.csdn.net/yannanxiu/article/details/53088534)
- 安信可IDE开发环境资料：
  - 如何安装安信可一体化开发环境：http://wiki.ai-thinker.com/ai_ide_install
  - 如何使用安信可 ESP 系列一体化开发环境：http://wiki.ai-thinker.com/ai_ide_use
  - 如何为 ESP 系列模组烧录固件：http://wiki.ai-thinker.com/esp_download
