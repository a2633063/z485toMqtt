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

#define ZLIB_WIFI_STATE_LED_IO_MUX     PERIPHS_IO_MUX_GPIO5_U
#define ZLIB_WIFI_STATE_LED_IO_NUM     5
#define ZLIB_WIFI_STATE_LED_IO_FUNC    FUNC_GPIO5

#define ZLIB_WEB_CONFIG_ONLY  (1)  //若使用了自定义webserver 此应该设置为0 若仅用web配网 此置1 且调用zlib_web_wifi_init即可
#define VERSION "v0.0.6"

#define TYPE 12

#define TYPE_NAME "z485toMqtt"
#define DEVICE_NAME "z485toMqtt_%s"

#define UART_REC_BUFF_MAX 200

#define USER_CONFIG_VERSION 2

#define SETTING_MQTT_STRING_LENGTH_MAX  64      //必须 4 字节对齐。
#define NAME_LENGTH 32		//插座名称字符串最大长度

#define TASK_NUM 20    //最多20组任务
#define TASK_DATA_REC_BUF_LENGTH 20    //接收数据最大长度
#define TASK_DATA_TOPIC_LENGTH 100    //数据最大长度
#define TASK_DATA_MESSAGE_LENGTH 200    //数据最大长度

//不使用枚举,因为枚举占用空间过大,不适合储存于flash
#define TASK_TYPE_MQTT 0
#define TASK_TYPE_WOL 1
#define TASK_TYPE_UART 2
#define TASK_TYPE_HTTP 3
#define TASK_TYPE_TIME_MQTT 4
#define TASK_TYPE_TIME_UART 5
#define TASK_TYPE_MAX 6


//自定义任务 参数结构体  当前大小  总储存flash需要空间!
//为保证4字节对齐情况下占用较小空间,变量顺序不可随意修改
typedef struct {
    uint8_t name[NAME_LENGTH];              //当前任务名称,供显示在app端
    uint8_t type;                           //任务类型编号
    uint8_t on;                             //任务开关
    uint8_t mqtt_send;                      //满足条件时,是否将接收到的数据发至mqtt
    uint8_t dat_length;                     //数据长度
    uint8_t condition_dat[TASK_DATA_REC_BUF_LENGTH];  //接收的数据
    uint8_t reserved;  //是否忽略某个字节,范围[1,TASK_DATA_REC_BUF_LENGTH]
    uint8_t hour;
    uint8_t minute;
    uint8_t repeat;                         //bit0-7 分别表示周一-周日  不支持一次定时
    union { //各任务类型数据共同体
        struct {    //TASK_TYPE_CUSTOM 结构体
            uint8_t topic[TASK_DATA_TOPIC_LENGTH];
            uint8_t payload[TASK_DATA_MESSAGE_LENGTH];
            uint8_t qos;
            uint8_t retained;
            uint8_t udp;
            uint8_t reserved;   //第几个字段替换payload中的%d
            uint8_t ip[4];
            uint16_t port;
        } mqtt;
        struct {    //TASK_TYPE_WOL 结构体
            uint8_t mac[6];
            uint16_t port;
            uint8_t ip[4];
            uint8_t secure[6];
        } wol;
        struct {    //TASK_TYPE_UART 结构体
            uint8_t dat_length;                      //满足条件时,是否将接收到的数据发至mqtt
            uint8_t reserved_rec;   //将接收到的数据中第reserved_rec个值
            uint8_t reserved_send;  //填入要发送的第reserved_send个字段
            uint8_t dat[TASK_DATA_MESSAGE_LENGTH];  //接收的数据
        } uart;
        struct {    //TASK_TYPE_HTTP 结构体 //不超过custome的总长度310     380
            uint8_t dat[300];
        } http;
    } data;

} user_config_task_t;



//用户保存参数结构体
typedef struct
{
    char version;
    uint16_t mqtt_port;        //mqtt service port
    uint8_t name[NAME_LENGTH];
    uint8_t mqtt_ip[SETTING_MQTT_STRING_LENGTH_MAX];   //mqtt service ip
    uint8_t mqtt_user[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user
    uint8_t mqtt_password[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user
    user_config_task_t task[TASK_NUM];
} user_config_t;


extern user_config_t user_config;
#endif

