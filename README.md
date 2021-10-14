# z485toMqtt
ESP8266 串口转mqtt协议

为了方便使用rs485通信,开发此工程.实际为串口转mqtt协议.

ESP8266从串口接收到的数据直接转发至mqtt.  也可以发送mqtt数据给ESP8266转为串口数据从串口发出.



建立了个QQ群,有问题可以加入来讨论:**459996006**  [点这里直接加群](//shang.qq.com/wpa/qunwpa?idkey=9104eabd6131d856b527ad89636fc603eb745a5d047e8b45d183165c8e607e59) 

也可以发送邮件给我:zip_zhang@foxmail.com  (邮件较多,可能会较晚回复)



> ### 作者声明
>
> 注意: 本项目主要目的为作者本人自己学习而开发,本着开源精神及造福网友而开源,仅个人开发,可能无法做到完整的测试,所以不承担他人使用本项目照成的所有后果.
>
> **严禁他人将本项目用户用于任何商业活动.个人在非盈利情况下可以自己使用,严禁其他人用此项目进行任何盈利服务.**
> 
> 有需要请联系作者:zip_zhang@foxmail.com



- **本设备还在开发完善中,可能会有部分bug(特别是自动化触发部分!),请介意者待完善后再使用.**

## 特性

本固件使用ESP8266 12f为基础,实现以下功能:

- [x] 串口接收到的数据转为mqtt反馈

- [x] mqtt接收的数据转为串口发出

- [x] ota在线升级

- [x] 无服务器时使用UDP通信

- [x] web通信

- [x] MQTT服务器连接控制

- [x] web配网

- [x] web配置mqtt信息

- [x] 自动化触发: 串口接收到设置的数据时执行

- [x] 自动化触发: 指定时间时执行

- [x] 自动化执行: mqtt/udp发送

- [x] 自动化执行: Wol局域网唤醒电脑

- [x] 自动化执行: 串口发送数据

- [x] 串口触发mqtt/udp/串口发送时,能够将串口接收到的某个字节填入触发的执行动作内容中

作者实际开发使用ESP8266-12F进行调试,若用其他esp8266模组,可以自行编译代码文件使用.



## 目录

[前言(必看)](#前言必看)

[开始](#开始)

[固件烧录](#固件烧录)

[开始使用/使用方法](#开始使用/使用方法)

~~[接入home assistant](#接入home-assistant)~~

[其他内容](#其他内容)

​	[通信协议](#通信协议)

​	[FAQ](#FAQ)





## 前言(必看)

- 除非写明了`如果你不是开发人员,请忽略此项`之类的字眼,否则,请**一个字一个字看清楚看完**整后再考虑动手及提问!很可能一句话就是你成功与否的关键!
- 烧录固件需要烧录器:一般的ttl串口烧录器即可,一般刷机的人应该都有,淘宝价格大概为2-5元
- 使用此固件,有app端配合方便配置,见[SmartControl_Android_MQTT](https://github.com/a2633063/SmartControl_Android_MQTT). 若不用app,通过web配置wifi及mqtt后也可以直接使用
- app只有android,因ios限制,本人不考虑免费做ios开发.(不要再问是否有ios端).

> 虽然没有ios端,但配网及mqtt设置可直接通过网页配置.自动化配置可以根据通讯协议自行发送相应内容即可
>
>  APP主要仅为第一次使用配对网络及配置mqtt服务器及设置相关自动化流程时使用

> 如果你不知道什么是mqtt或homeassistant,所有有关的内容可以跳过.

> 如果你有任何问题,可以直接在此项目中提交issue,或给我发送邮件:zip_zhang@foxmail.com,邮件标题中请注明[z485toMqtt].
>





## 开始

整体流程:烧录ESP8266固件-,接线,(与rs485/rs232通信时需要增加ttl转rs485/rs232模块)

烧录完成后,首次使用前配对网络并配置mqtt服务器,之后就可以使用了.

请注意: 由于串口(rs485/rs232)通信所用io口与烧录接口共用,因此在与ttl转rs485/rs232模块连接后,可能会导致烧录失败.因此建议先烧录完成后再接线.

之后只需ota升级,不需要再进行烧录过程



## 固件烧录

见[固件烧录](https://github.com/a2633063/z485toMqtt/wiki/固件烧录)

烧录固件完成后,即可开始使用



## 开始使用/使用方法

见[开始使用](https://github.com/a2633063/z485toMqtt/wiki/开始使用)



## ~~接入home assistant~~

无必要接入home assistant. 但是触发的自动化条件或转发到mqtt的串口数据您可以根据您的需要接入ha中配置合适您自己的自动化流程

您可以根据通讯协议自行接入.



## 其他内容



### 通信协议

> 此项为专业开发人员准备,如果你不是开发人员,请跳过此项

为了其他开发人员开发自己的控制软件,将通信协议开放

所有通信协议开源,你可以自己开发控制app或ios端

见[通信协议](https://github.com/a2633063/z485toMqtt/wiki/通信协议)



### FAQ

见 [FAQ](https://github.com/a2633063/SmartControl_Android_MQTT/wiki/FAQ)

