#include <ctype.h>
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "key.h"
#include "user_uart.h"

void ICACHE_FLASH_ATTR user_fun_send_wol(uint8_t mac[6], uint8_t ip[4], uint16_t port, uint8_t secure[6])
{
    uint8_t i, j, length = 108;
    uint8_t wol_dat[108];
    if(wifi_station_get_connect_status() == STATION_GOT_IP || wifi_softap_get_station_num() > 0)
    {

        for (i = 0; i < 6; i++)
        {
            wol_dat[i] = 0xff;
        }

        for (i = 1; i < 17; i++)
        {
            for (j = 0; j < 6; j++)
                wol_dat[6 * i + j] = mac[j];
        }

        if(secure[0] != 0 || secure[1] != 0 || secure[2] != 0 || secure[3] != 0 || secure[4] != 0 || secure[5] != 0)
        {
            length = 108;
            for (j = 0; j < 6; j++)
                wol_dat[102 + j] = secure[j];
        }
        else
        {
            length = 102;
        }

        for (i = 0; i < 3; i++)
            zlib_udp_send_ip_port(NULL, wol_dat, length, ip, port);
    }
}

void ICACHE_FLASH_ATTR user_fun_task_deal(user_config_task_t *task)
{
    uint16_t re_num;
    uint8_t * re_data;
    re_data = user_uart_get_data(&re_num);

    uint8_t payload[TASK_DATA_MESSAGE_LENGTH + 2];  //准备发送的数据//可能需要将%d替换为数字,因此比最长长度可能长1
    switch (task->type)
    {
        case TASK_TYPE_MQTT:    //mqtt数据
        {
            if(task->data.mqtt.reserved - 1 < re_num)
            {
                os_sprintf(payload, task->data.mqtt.payload, re_data[task->data.mqtt.reserved]);
            }
            else
            {
                os_sprintf(payload, task->data.mqtt.payload);
            }
            //发送数据
            if(!zlib_mqtt_is_connected() || task->data.mqtt.udp != 0 || os_strlen(task->data.mqtt.topic) == 0)
            {        //udp发送
                if(task->data.mqtt.port > 0)
                    zlib_udp_send_ip_port(NULL, payload, os_strlen(payload), task->data.mqtt.ip, task->data.mqtt.port);
            }
            else
            {        //mqtt发送
                zlib_mqtt_send_message(task->data.mqtt.topic, payload, task->data.mqtt.qos, task->data.mqtt.retained);
            }
            break;
        }
        case TASK_TYPE_TIME_MQTT:
        {
            //发送数据
            if(!zlib_mqtt_is_connected() || task->data.mqtt.udp != 0 || os_strlen(task->data.mqtt.topic) == 0)
            {        //udp发送
                if(task->data.mqtt.port > 0)
                    zlib_udp_send_ip_port(NULL, task->data.mqtt.payload, os_strlen(payload), task->data.mqtt.ip,
                            task->data.mqtt.port);
            }
            else
            {        //mqtt发送
                zlib_mqtt_send_message(task->data.mqtt.topic, task->data.mqtt.payload, task->data.mqtt.qos,
                        task->data.mqtt.retained);
            }
            break;
        }
        case TASK_TYPE_WOL: //局域网唤醒Wol功能
        {
            user_fun_send_wol(task->data.wol.mac, task->data.wol.ip, task->data.wol.port, task->data.wol.secure);
            break;
        }
        case TASK_TYPE_UART:
        {
            os_memcpy(payload, task->data.uart.dat, task->data.uart.dat_length);
            if(task->data.uart.reserved_rec - 1 < re_num
                    && task->data.uart.reserved_send - 1 < task->data.uart.dat_length)
            {
                payload[task->data.uart.reserved_send - 1] = re_data[task->data.uart.reserved_rec - 1];
            }
            uart0_tx_buffer(payload, task->data.uart.dat_length);
            break;
        }
        case TASK_TYPE_TIME_UART:
        {
            uart0_tx_buffer(task->data.uart.dat, task->data.uart.dat_length);
            break;
        }
        case TASK_TYPE_HTTP:    //http
        {
            break;
        }
    }
}
