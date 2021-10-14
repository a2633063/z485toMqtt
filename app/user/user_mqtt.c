#include <ctype.h>
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "mqtt_msg.h"
#include "user_function.h"

#define MAX_MQTT_TOPIC_SIZE         (64)

#define MQTT_CLIENT_SUB_TOPIC   "device/%s/%s/set"
#define MQTT_CLIENT_PUB_TOPIC   "device/%s/%s/state"
#define MQTT_CLIENT_SENSOR_TOPIC   "device/%s/%s/sensor"
#define MQTT_CLIENT_WILL_TOPIC   "device/%s/%s/availability"

static char topic_state[MAX_MQTT_TOPIC_SIZE];
static char topic_set[MAX_MQTT_TOPIC_SIZE];
static char topic_sensor[MAX_MQTT_TOPIC_SIZE];
static char willtopic[MAX_MQTT_TOPIC_SIZE];


/**
 * 函  数  名: _user_mqtt_received_cb
 * 函数说明: mqtt接受数据回调函数
 * 参        数: 无
 * 返        回: true:继续处理json数据      false:不继续处理json数据
 */
bool ICACHE_FLASH_ATTR _user_mqtt_received_cb(uint32_t *arg, const char* topic, uint32_t topic_len, const char *data,
        uint32_t data_len)
{
    //if(os_strlen(data) > 0) zlib_mqtt_send_message(topic, "", 1, 1);
    uint32_t i;
    int16_t val;
    uint8_t *send_buf = NULL;
    uint8_t *send_p;
    char *dat = data;

    if(*data != '{')
    {
        send_buf = (uint8_t *) os_zalloc((data_len+1)/2);
        if(send_buf == NULL)
        {
            os_printf("memery os_zalloc ERROR!\r\n");
            os_free(send_buf);
            return true;
        }
        os_memset(send_buf, 0, (data_len + 1) / 2);
        send_p = send_buf;
        for (i = 0; i < data_len && *dat != '\0'; i++)
        {
            val = char2nibble(*dat);
            if(val == -1)
            {
                os_free(send_buf);
                return true;
            }
            *send_p = ((*send_p) << 4) | val;
            if(i % 2 == 1) send_p++;
            dat++;
        }
        uart0_tx_buffer(send_buf, (data_len + 1) / 2);
        os_printf("Uart0 send hex: %s\r\n",data);
        os_free(send_buf);
        return false;
    }

    return true;
}

/**
 * 函  数  名: user_mqtt_init
 * 函数说明: mqtt初始化
 * 参        数: 无
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR user_mqtt_init(void)
{
    uint8_t i;

    //初始化topic字符串

    os_sprintf(topic_set, MQTT_CLIENT_SUB_TOPIC, strlwr(TYPE_NAME), zlib_wifi_get_mac_str());
    os_sprintf(topic_state, MQTT_CLIENT_PUB_TOPIC, strlwr(TYPE_NAME), zlib_wifi_get_mac_str());
    os_sprintf(topic_sensor, MQTT_CLIENT_SENSOR_TOPIC, strlwr(TYPE_NAME), zlib_wifi_get_mac_str());
    os_sprintf(willtopic, MQTT_CLIENT_WILL_TOPIC, strlwr(TYPE_NAME), zlib_wifi_get_mac_str());

    os_printf("topic_set:%s\n", topic_set);
    os_printf("topic_state:%s\n", topic_state);
    os_printf("topic_sensor:%s\n", topic_sensor);
    os_printf("willtopic:%s\n", willtopic);

    mqtt_connect_info_t mqtt_info;
    mqtt_info.client_id = zlib_wifi_get_mac_str();
    mqtt_info.username = user_config.mqtt_user;
    mqtt_info.password = user_config.mqtt_password;
    mqtt_info.will_topic = willtopic;
    mqtt_info.will_message = "0";
    mqtt_info.keepalive = 20;
    mqtt_info.will_qos = 1;
    mqtt_info.will_retain = 1;
    mqtt_info.clean_session = 0;
    zlib_mqtt_init(user_config.mqtt_ip, user_config.mqtt_port, &mqtt_info);

    static zlib_mqtt_topic_info_t mqtt_topic[1];
    mqtt_topic[0].topic = topic_set;
    mqtt_topic[0].qos = 1;

    static zlib_mqtt_message_info_t message[1];
    message[0].topic = willtopic;
    message[0].message = "1";
    message[0].qos = 1;
    message[0].retain = 1;

    zlib_mqtt_set_online_message(message, sizeof(message) / sizeof(zlib_mqtt_message_info_t));
    zlib_mqtt_subscribe(mqtt_topic, sizeof(mqtt_topic) / sizeof(zlib_mqtt_topic_info_t));
    zlib_mqtt_set_received_callback(_user_mqtt_received_cb); //是否清楚set主题的retain数据
    os_printf("user mqtt init\n");
}

char * ICACHE_FLASH_ATTR user_mqtt_get_state_topic()
{
    return topic_state;
}
char * ICACHE_FLASH_ATTR user_mqtt_get_set_topic()
{
    return topic_set;
}
char * ICACHE_FLASH_ATTR user_mqtt_get_sensor_topic()
{
    return topic_sensor;
}
char * ICACHE_FLASH_ATTR user_mqtt_get_will_topic()
{
    return willtopic;
}
