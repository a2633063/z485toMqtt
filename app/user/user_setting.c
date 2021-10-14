#include <ctype.h>
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "user_setting.h"

#define USER_SETTING_AUTO_ADDR  (0x71)  //储存地址
void ICACHE_FLASH_ATTR user_setting_init(void)
{
    uint8_t i;

    os_printf("user config size:%d\n", sizeof(user_config_t));
    os_printf("user config.task.data:%d\n", sizeof(user_config.task[0].data));
    zlib_setting_get_config(&user_config, sizeof(user_config_t));
    if(user_config.version != USER_CONFIG_VERSION)
    {
        uint8_t *str;
        zlib_wifi_mac_init();
        str = zlib_wifi_get_mac_str();
        os_sprintf(user_config.name, DEVICE_NAME, str + 8);
        os_printf("device name:%s\n", user_config.name);
        os_sprintf(user_config.mqtt_ip, "zipzhang.top");
        os_sprintf(user_config.mqtt_user, "z");
        os_sprintf(user_config.mqtt_password, "2633063");
        user_config.mqtt_port = 1883;
        user_config.version = USER_CONFIG_VERSION;

        for (i = 0; i < TASK_NUM; i++)
        {
            user_config_task_t *task=&user_config.task[i];
            os_sprintf(task->name, "%d", i+1);
            //os_printf("task name:%s\n", task->name);
            task->type = TASK_TYPE_MQTT;
            task->on = 0;
            task->mqtt_send = 0;
            task->dat_length = 0;
            os_memset(task->condition_dat, 0xff, sizeof(task->condition_dat));
            task->reserved = 0;
            task->hour = 12;
            task->minute = 0;
            task->repeat = 0x7f;
            task->data.mqtt.topic[0] = 0;
            task->data.mqtt.payload[0] = 0;
            task->data.mqtt.qos = 0;
            task->data.mqtt.retained = 0;
            task->data.mqtt.udp = 0;
            task->data.mqtt.reserved = 255;
            os_memset(task->data.mqtt.ip, 0xff, sizeof(task->data.mqtt.ip));
            task->data.mqtt.port = 10182;
        }

        os_printf("config version error.Restore default settings\n");
        zlib_setting_save_config(&user_config, sizeof(user_config_t));
    }

    os_printf("user setting init\n");

}
