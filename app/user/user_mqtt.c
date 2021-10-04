#include <ctype.h>
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "mqtt_msg.h"

#define MAX_MQTT_TOPIC_SIZE         (64)

#define MQTT_CLIENT_SUB_TOPIC   "device/%s/%s/set"
#define MQTT_CLIENT_PUB_TOPIC   "device/%s/%s/state"
#define MQTT_CLIENT_SENSOR_TOPIC   "device/%s/%s/sensor"
#define MQTT_CLIENT_WILL_TOPIC   "device/%s/%s/availability"

static char topic_state[MAX_MQTT_TOPIC_SIZE];
static char topic_set[MAX_MQTT_TOPIC_SIZE];
static char topic_sensor[MAX_MQTT_TOPIC_SIZE];
static char willtopic[MAX_MQTT_TOPIC_SIZE];

bool ICACHE_FLASH_ATTR _user_mqtt_received_cb(uint32_t *arg, const char* topic, const char *data)
{
    if(os_strlen(data) > 0) zlib_mqtt_send_message(topic, "", 1, 1);
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
