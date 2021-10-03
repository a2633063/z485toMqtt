#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

//#define USE_OPTIMIZE_PRINTF

#include "os_type.h"

#define BCDtoDEC(x) ( ((x)>>4)*10+(x)%0x10  )           //BCD码转换为十进制表示方式
#define DECtoBCD(x) ( (((x)/10)<<4)+(x)%10  )           //十进制转换为BCD码表示方式

#define ZLIB_WIFI_CALLBACK_REPEAT  (0)  //wifi连接过程中 是否重复调用回调函数
#define ZLIB_WIFI_MDSN_ENABLI  (1)      //是否启用mdns,默认不启用mdns

#define ZLIB_UDP_REPLY_PORT  (10181)  //udp回复默认端口号
#define ZLIB_WIFI_CONNECTED_STATE_LED  (1)  //wifi连接后 wifi状态指示灯io口电平
//#define ZLIB_WIFI_STATE_LED_IO_MUX     PERIPHS_IO_MUX_GPIO2_U
//#define ZLIB_WIFI_STATE_LED_IO_NUM     2
//#define ZLIB_WIFI_STATE_LED_IO_FUNC    FUNC_GPIO2

#define ZLIB_WEB_CONFIG_ONLY  (1)  //若使用了自定义webserver 此应该设置为0 若仅用web配网 此置1 且调用zlib_web_wifi_init即可
#define VERSION "v0.0.2"

#define TYPE 99

#define TYPE_NAME "zC1"
#define DEVICE_NAME "zC1_%s"


#define USER_CONFIG_VERSION 1

#define SETTING_MQTT_STRING_LENGTH_MAX  64      //必须 4 字节对齐。
#define NAME_LENGTH 32		//插座名称字符串最大长度

//布防值
#define ALERT_OFF		0xff	//关布防
#define ALERT_NO_BEEP	0x02	//开布防关蜂鸣器
#define ALERT_WITH_BEEP	0x01	//开布防开蜂鸣器

//用户保存参数结构体
typedef struct
{
    char version;
    uint8_t name[NAME_LENGTH];
    uint8_t mqtt_ip[SETTING_MQTT_STRING_LENGTH_MAX];   //mqtt service ip
    uint16_t mqtt_port;        //mqtt service port
    uint8_t mqtt_user[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user
    uint8_t mqtt_password[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user

} user_config_t;

/*
 //串口通信数据结构体
 typedef struct {
 uint8_t flag;	//是否已经更新
 uint8_t cmd;	//cmd
 uint16_t msgID1;	//msgID
 uint8_t reed;	//干簧管状态
 struct struct_time now;
 uint8_t msgID2;	//msgID
 uint8_t battery;	//电量
 uint8_t alert;	//布防开关
 struct struct_time alert_time_start;	//布防时间
 struct struct_time alert_time_end;	//布防时间
 } user_sensor_t;*/
extern user_config_t user_config;

#endif

