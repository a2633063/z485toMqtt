#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"

#include "user_mqtt.h"

/**
 * 函  数  名: _json_timer_fun
 * 函数说明: json处理定時器回调函数,当需要保存flash时,延时2秒再保存数据
 * 参        数: 无
 * 返        回: 无
 */
static os_timer_t _timer_json;
static void _json_timer_fun(void *arg)
{
    zlib_setting_save_config(&user_config, sizeof(user_config_t));
}
/**
 * 函  数  名: _json_json_to_task
 * 函数说明: json中的task数据转化为实际的user_config_task_t类型
 * 参        数: p_task: 需要处理task的json
 *        task: 需要接收的task
 * 返        回: 无
 */
bool ICACHE_FLASH_ATTR _json_json_to_task(cJSON * p_task, user_config_task_t * task)
{
    uint8_t i;
    int16_t val;
    bool return_flag = false;
    char *p;
    //设置名称
    cJSON *p_name = cJSON_GetObjectItem(p_task, "name");
    if(cJSON_IsString(p_name))        //cJSON_IsXxxx中已经判断是否为NULL
    {
        return_flag = true;
        os_sprintf(task->name, p_name->valuestring);
    }

    //解析on
    cJSON *p_task_on = cJSON_GetObjectItem(p_task, "on");
    if(cJSON_IsNumber(p_task_on) && (p_task_on->valueint == 1 || p_task_on->valueint == 0))
    {
        task->on = p_task_on->valueint;
        return_flag = true;
    }

    //开始解析各字段
    cJSON *p_task_type = cJSON_GetObjectItem(p_task, "type");
    if(!cJSON_IsNumber(p_task_type) || p_task_type->valueint >= TASK_TYPE_MAX) goto EXIT;

    cJSON *p_task_dat_uart = cJSON_GetObjectItem(p_task, "uart_dat");
    cJSON *p_task_mqtt_send = cJSON_GetObjectItem(p_task, "mqtt_send");
    cJSON *p_task_reserved = cJSON_GetObjectItem(p_task, "reserved");
    cJSON *p_task_hour = cJSON_GetObjectItem(p_task, "hour");
    cJSON *p_task_minute = cJSON_GetObjectItem(p_task, "minute");
    cJSON *p_task_repeat = cJSON_GetObjectItem(p_task, "repeat");

    cJSON *p_task_mqtt = cJSON_GetObjectItem(p_task, "mqtt");
    cJSON *p_task_wol = cJSON_GetObjectItem(p_task, "wol");
    cJSON *p_task_uart = cJSON_GetObjectItem(p_task, "uart");

    switch (p_task_type->valueint)
    {
        case TASK_TYPE_TIME_MQTT:
        case TASK_TYPE_MQTT:
        {
            if(!p_task_mqtt) goto EXIT;

            cJSON *p_task_mqtt_topic = cJSON_GetObjectItem(p_task_mqtt, "topic");
            cJSON *p_task_mqtt_payload = cJSON_GetObjectItem(p_task_mqtt, "payload");
            cJSON *p_task_mqtt_qos = cJSON_GetObjectItem(p_task_mqtt, "qos");
            cJSON *p_task_mqtt_retained = cJSON_GetObjectItem(p_task_mqtt, "retained");
            cJSON *p_task_mqtt_reserved = cJSON_GetObjectItem(p_task_mqtt, "reserved");

            //确认执行条件
            if(!cJSON_IsString(p_task_mqtt_topic) || !cJSON_IsString(p_task_mqtt_payload)
                    || !cJSON_IsNumber(p_task_mqtt_qos) || !cJSON_IsNumber(p_task_mqtt_retained)) goto EXIT;

            //确认触发条件
            if(p_task_type->valueint == TASK_TYPE_MQTT)
            {
                if(!cJSON_IsString(p_task_dat_uart)) goto EXIT;
                //TASK_TYPE_MQTT,更新dat_length condition_dat reserved字段
                //更新dat_length及condition_dat
                p = p_task_dat_uart->valuestring;
                task->dat_length = 0;
                task->condition_dat[0] = 0;
                for (i = 0; *p != '\0' && task->dat_length < TASK_DATA_REC_BUF_LENGTH; i++, p++)
                {
                    val = char2nibble(*p);
                    if(val < 0) break;
                    task->condition_dat[task->dat_length] = (task->condition_dat[task->dat_length] << 4) | val;
                    if(i % 2 == 1)
                    {
                        task->dat_length++;
                        task->condition_dat[task->dat_length] = 0;
                    }
                }
                //更新reserved
                task->reserved = 255;
                if(cJSON_IsNumber(p_task_reserved))
                {
                    task->reserved = p_task_reserved->valueint;
                }

                //解析mqtt_send
                task->mqtt_send = 0;
                if(cJSON_IsNumber(p_task_mqtt_send)
                        && (p_task_mqtt_send->valueint == 1 || p_task_mqtt_send->valueint == 0))
                {
                    task->mqtt_send = p_task_mqtt_send->valueint;
                }

            }
            else if(p_task_type->valueint == TASK_TYPE_TIME_MQTT)
            {
                if(!cJSON_IsNumber(p_task_hour) || !cJSON_IsNumber(p_task_minute) || !cJSON_IsNumber(p_task_repeat))
                    goto EXIT;
                //TASK_TYPE_TIME_MQTT,更新hour minute repeat mqtt_send字段
                task->hour = p_task_hour->valueint;
                task->minute = p_task_minute->valueint;
                task->repeat = p_task_repeat->valueint;
            }
            //获取mqtt相关
            task->type = p_task_type->valueint;
            return_flag = true;
            os_sprintf(task->data.mqtt.topic, p_task_mqtt_topic->valuestring);
            os_sprintf(task->data.mqtt.payload, p_task_mqtt_payload->valuestring);
            task->data.mqtt.qos = p_task_mqtt_qos->valueint;
            task->data.mqtt.retained = p_task_mqtt_retained->valueint;

            if(cJSON_IsNumber(p_task_mqtt_reserved))
            {
                task->data.mqtt.reserved = p_task_mqtt_reserved->valueint;
            }
            else
            {
                task->data.mqtt.reserved = 255;
            }

            task->data.mqtt.udp = 0;
            task->data.mqtt.port = 0;

            //开始处理task中mqtt中udp相关数据
            cJSON *p_task_mqtt_udp = cJSON_GetObjectItem(p_task_mqtt, "udp");
            cJSON *p_task_mqtt_ip = cJSON_GetObjectItem(p_task_mqtt, "ip");
            cJSON *p_task_mqtt_port = cJSON_GetObjectItem(p_task_mqtt, "port");
            if(!cJSON_IsNumber(p_task_mqtt_udp) || !cJSON_IsArray(p_task_mqtt_ip) || !cJSON_IsNumber(p_task_mqtt_port)
                    || cJSON_GetArraySize(p_task_mqtt_ip) != 4) goto EXIT;

            if(p_task_mqtt_port->valueint == 0) goto EXIT;

            for (i = 0; i < 4; i++)
                task->data.mqtt.ip[i] = cJSON_GetArrayItem(p_task_mqtt_ip, i)->valueint;
            task->data.mqtt.udp = p_task_mqtt_udp->valueint;
            task->data.mqtt.port = p_task_mqtt_port->valueint;

            break;
        }
        case TASK_TYPE_WOL:
        {
            if(!p_task_wol) goto EXIT;

            cJSON *p_task_wol_mac = cJSON_GetObjectItem(p_task_wol, "mac");
            cJSON *p_task_wol_port = cJSON_GetObjectItem(p_task_wol, "port");
            cJSON *p_task_wol_ip = cJSON_GetObjectItem(p_task_wol, "ip");
            cJSON *p_task_wol_secure = cJSON_GetObjectItem(p_task_wol, "secure");
            //确认执行条件
            if(cJSON_GetArraySize(p_task_wol_mac) != 6) goto EXIT;

            //确认触发条件
            if(!cJSON_IsString(p_task_dat_uart)) goto EXIT;
            //TASK_TYPE_WOL,更新dat_length condition_dat reserved mqtt_send字段
            p = p_task_dat_uart->valuestring;
            task->dat_length = 0;
            task->condition_dat[0] = 0;
            for (i = 0; *p != '\0' && task->dat_length < TASK_DATA_REC_BUF_LENGTH; i++, p++)
            {
                val = char2nibble(*p);
                if(val < 0) break;
                task->condition_dat[task->dat_length] = (task->condition_dat[task->dat_length] << 4) | val;
                if(i % 2 == 1)
                {
                    task->dat_length++;
                    task->condition_dat[task->dat_length] = 0;
                }
            }

            task->reserved = 255;
            if(cJSON_IsNumber(p_task_reserved))
            {
                task->reserved = p_task_reserved->valueint;
            }

            //开始获取执行内容
            return_flag = true;
            task->type = p_task_type->valueint;

            for (i = 0; i < 6; i++)
                task->data.wol.mac[i] = cJSON_GetArrayItem(p_task_wol_mac, i)->valueint;
            task->data.wol.port = 9;
            if(cJSON_IsNumber(p_task_wol_port) && p_task_wol_port->valueint > 0)
                task->data.wol.port = p_task_wol_port->valueint;
            if(cJSON_GetArraySize(p_task_wol_ip) == 4)
            {
                for (i = 0; i < 4; i++)
                    task->data.wol.ip[i] = cJSON_GetArrayItem(p_task_wol_ip, i)->valueint;
            }
            else
            {
                os_memset(task->data.wol.ip, 255, sizeof(task->data.wol.ip));
            }
            if(cJSON_GetArraySize(p_task_wol_ip) == 6)
            {
                for (i = 0; i < 6; i++)
                    task->data.wol.secure[i] = cJSON_GetArrayItem(p_task_wol_secure, i)->valueint;
            }
            else
            {
                os_memset(task->data.wol.secure, 0, sizeof(task->data.wol.secure));
            }

            break;
        }
        case TASK_TYPE_UART:
        case TASK_TYPE_TIME_UART:
        {
            if(!p_task_uart) goto EXIT;

            cJSON *p_task_uart_uart = cJSON_GetObjectItem(p_task_uart, "uart");
            cJSON *p_task_uart_reserved_rec = cJSON_GetObjectItem(p_task_uart, "reserved_rec");
            cJSON *p_task_uart_reserved_send = cJSON_GetObjectItem(p_task_uart, "reserved_send");

            //确认执行条件
            if(!cJSON_IsString(p_task_uart_uart)) goto EXIT;

            //确认触发条件
            if(p_task_type->valueint == TASK_TYPE_UART)
            {
                if(!cJSON_IsString(p_task_dat_uart)) goto EXIT;
                //TASK_TYPE_MQTT,更新dat_length condition_dat reserved mqtt_send字段
                p = p_task_dat_uart->valuestring;
                task->dat_length = 0;
                task->condition_dat[0] = 0;
                for (i = 0; *p != '\0' && task->dat_length < TASK_DATA_REC_BUF_LENGTH; i++, p++)
                {
                    val = char2nibble(*p);
                    if(val < 0) break;
                    task->condition_dat[task->dat_length] = (task->condition_dat[task->dat_length] << 4) | val;
                    if(i % 2 == 1)
                    {
                        task->dat_length++;
                        task->condition_dat[task->dat_length] = 0;
                    }
                }

                task->reserved = 255;
                if(cJSON_IsNumber(p_task_reserved))
                {
                    task->reserved = p_task_reserved->valueint;
                }

                //解析mqtt_send
                task->mqtt_send = 0;
                if(cJSON_IsNumber(p_task_mqtt_send)
                        && (p_task_mqtt_send->valueint == 1 || p_task_mqtt_send->valueint == 0))
                {
                    task->mqtt_send = p_task_mqtt_send->valueint;
                    return_flag = true;
                }

            }
            else if(p_task_type->valueint == TASK_TYPE_TIME_UART)
            {
                if(!cJSON_IsNumber(p_task_hour) || !cJSON_IsNumber(p_task_minute) || !cJSON_IsNumber(p_task_repeat))
                    goto EXIT;
                //TASK_TYPE_TIME_MQTT,更新hour minute repeat字段
                task->hour = p_task_hour->valueint;
                task->minute = p_task_minute->valueint;
                task->repeat = p_task_repeat->valueint;
            }
            //获取执行相关内容
            task->type = p_task_type->valueint;
            return_flag = true;

            p = p_task_uart_uart->valuestring;
            task->data.uart.dat_length = 0;
            task->data.uart.dat[0] = 0;
            for (i = 0; *p != '\0' && task->data.uart.dat_length < TASK_DATA_MESSAGE_LENGTH; i++, p++)
            {
                val = char2nibble(*p);
                if(val < 0) break;
                task->data.uart.dat[task->data.uart.dat_length] = (task->data.uart.dat[task->data.uart.dat_length] << 4)
                        | val;
                if(i % 2 == 1)
                {
                    task->data.uart.dat_length++;
                    task->data.uart.dat[task->data.uart.dat_length] = 0;
                }
            }

            task->data.uart.reserved_rec = 255;
            task->data.uart.reserved_send = 255;
            if(cJSON_IsNumber(p_task_uart_reserved_rec))
            {
                task->data.uart.reserved_rec = p_task_uart_reserved_rec->valueint;
            }

            if(cJSON_IsNumber(p_task_uart_reserved_send))
            {
                task->data.uart.reserved_send = p_task_uart_reserved_send->valueint;
            }

            break;
        }
    }

    EXIT: return return_flag;
}

/**
 * 函  数  名: _json_task_to_json
 * 函数说明: 将task转为能够发送的json
 * 参        数: p_task: 需要处理task的json
 *        task: 需要接收的task
 * 返        回: 无
 */
bool ICACHE_FLASH_ATTR _json_task_to_json(user_config_task_t * task, cJSON * p_task)
{
    uint16_t i;
    cJSON *p_json_array;
    char send_str[TASK_DATA_MESSAGE_LENGTH * 2 + 1] = { 0 };
    char *p = send_str;

    cJSON_AddStringToObject(p_task, "name", task->name);
    cJSON_AddNumberToObject(p_task, "type", task->type);
    cJSON_AddNumberToObject(p_task, "on", task->on);

    if(task->type == TASK_TYPE_MQTT || task->type == TASK_TYPE_WOL || task->type == TASK_TYPE_UART)
    {        //串口触发
        cJSON_AddNumberToObject(p_task, "mqtt_send", task->mqtt_send);
        p = send_str;
        os_memset(send_str,0,sizeof(send_str));
        for (i = 0; i < task->dat_length && i < TASK_DATA_REC_BUF_LENGTH; i++)
        {
            os_sprintf(p, "%02X", task->condition_dat[i]);
            p += 2;
        }
        *p = 0;
        cJSON_AddStringToObject(p_task, "uart_dat", send_str);

        if(task->reserved > 0 && task->reserved <= task->dat_length)
        {
            cJSON_AddNumberToObject(p_task, "reserved", task->reserved);
        }
    }
    else if(task->type == TASK_TYPE_TIME_MQTT || task->type == TASK_TYPE_TIME_UART)
    {        //定时触发
        cJSON_AddNumberToObject(p_task, "hour", task->hour);
        cJSON_AddNumberToObject(p_task, "minute", task->minute);
        cJSON_AddNumberToObject(p_task, "repeat", task->repeat);
    }

    switch (task->type)
    {
        case TASK_TYPE_MQTT:
        case TASK_TYPE_TIME_MQTT:
        {
            cJSON *p_task_mqtt = cJSON_CreateObject();

            cJSON_AddStringToObject(p_task_mqtt, "topic", task->data.mqtt.topic);
            cJSON_AddStringToObject(p_task_mqtt, "payload", task->data.mqtt.payload);
            cJSON_AddNumberToObject(p_task_mqtt, "qos", task->data.mqtt.qos);
            cJSON_AddNumberToObject(p_task_mqtt, "retained", task->data.mqtt.retained);

            if(task->type == TASK_TYPE_MQTT && task->data.mqtt.reserved - 1 < 254)
            {   //task->data.mqtt.reserved为0 或大于串口接收长度时 不使用替换功能
                cJSON_AddNumberToObject(p_task_mqtt, "reserved", task->data.mqtt.reserved);
            }
            //处理udp部分
            if(task->data.mqtt.port > 0)
            {
                cJSON_AddNumberToObject(p_task_mqtt, "udp", task->data.mqtt.udp);

                p_json_array = cJSON_CreateArray();
                for (i = 0; i < 4; i++)
                    cJSON_AddItemToArray(p_json_array, cJSON_CreateNumber(task->data.mqtt.ip[i]));
                cJSON_AddItemToObject(p_task_mqtt, "ip", p_json_array);

                cJSON_AddNumberToObject(p_task_mqtt, "port", task->data.mqtt.port);
            }
            cJSON_AddItemToObject(p_task, "mqtt", p_task_mqtt);
            break;
        }
        case TASK_TYPE_WOL:
        {
            cJSON *p_task_wol = cJSON_CreateObject();

            p_json_array = cJSON_CreateArray();
            for (i = 0; i < 6; i++)
                cJSON_AddItemToArray(p_json_array, cJSON_CreateNumber(task->data.wol.mac[i]));
            cJSON_AddItemToObject(p_task_wol, "mac", p_json_array);

            p_json_array = cJSON_CreateArray();
            for (i = 0; i < 4; i++)
                cJSON_AddItemToArray(p_json_array, cJSON_CreateNumber(task->data.wol.ip[i]));
            cJSON_AddItemToObject(p_task_wol, "ip", p_json_array);

            cJSON_AddNumberToObject(p_task_wol, "port", task->data.wol.port);

            if(task->data.wol.secure[0] != 0 || task->data.wol.secure[1] != 0 || task->data.wol.secure[2] != 0
                    || task->data.wol.secure[3] != 0 || task->data.wol.secure[4] != 0 || task->data.wol.secure[5] != 0)
            {
                p_json_array = cJSON_CreateArray();
                for (i = 0; i < 6; i++)
                    cJSON_AddItemToArray(p_json_array, cJSON_CreateNumber(task->data.wol.secure[i]));
                cJSON_AddItemToObject(p_task_wol, "secure", p_json_array);

            }

            cJSON_AddItemToObject(p_task, "wol", p_task_wol);
            break;
        }
        case TASK_TYPE_UART:
        case TASK_TYPE_TIME_UART:
        {
            cJSON *p_task_uart = cJSON_CreateObject();
            p = send_str;
            os_memset(send_str,0,sizeof(send_str));
            for (i = 0; i < task->data.uart.dat_length && i < TASK_DATA_MESSAGE_LENGTH; i++)
            {
                os_sprintf(p, "%02X", task->data.uart.dat[i]);
                p += 2;
            }
            *p = 0;
            cJSON_AddStringToObject(p_task_uart, "uart", send_str);

            if(task->type == TASK_TYPE_UART && task->data.uart.reserved_send - 1 < task->data.uart.dat_length
                    && task->data.uart.reserved_rec - 1 < 254)
            {
                cJSON_AddNumberToObject(p_task_uart, "reserved_rec", task->data.uart.reserved_rec);
                cJSON_AddNumberToObject(p_task_uart, "reserved_send", task->data.uart.reserved_send);
            }
            cJSON_AddItemToObject(p_task, "uart", p_task_uart);
            break;
        }
    }
}
/**
 * 函  数  名: _json_deal_cb
 * 函数说明: json数据处理初始化回调函数,在此函数中处理json
 * 参        数: x: 任务编号
 *        pJsonRoot: 需要处理的json
 *        pJsonSend: 需要反馈的json
 * 返        回: 无
 */
bool ICACHE_FLASH_ATTR json_task_analysis(unsigned char x, cJSON * pJsonRoot, cJSON * pJsonSend)
{
    uint8_t i;
    bool return_flag = false;

    if(!pJsonRoot) return false;
    if(!pJsonSend) return false;

    char task_str[8];
    os_sprintf(task_str, "task_%d", x);
    if(x >= TASK_NUM) return false;

    cJSON *p_task = cJSON_GetObjectItem(pJsonRoot, task_str);
    if(!p_task) return false;

    user_config_task_t * task = &user_config.task[x];
    cJSON *json_task_send = cJSON_CreateObject();

    //开始解析各字段
    return_flag = _json_json_to_task(p_task, task);

    //准备返回的json字段
    _json_task_to_json(task, json_task_send);

    cJSON_AddItemToObject(pJsonSend, task_str, json_task_send);
    return return_flag;
}

/**
 * 函  数  名: _json_deal_cb
 * 函数说明: json数据处理初始化回调函数,在此函数中处理json
 * 参        数: 无
 * 返        回: 无
 */
static void ICACHE_FLASH_ATTR _json_deal_cb(void *arg, Wifi_Comm_type_t type, cJSON * pJsonRoot, void *p)
{
    uint8_t i;
    bool update_user_config_flag = false;   //标志位,记录最后是否需要更新储存的数据
    uint8_t retained = 0;
    //解析device report
    cJSON *p_cmd = cJSON_GetObjectItem(pJsonRoot, "cmd");
    if(p_cmd && cJSON_IsString(p_cmd) && os_strcmp(p_cmd->valuestring, "device report") == 0)
    {

        os_printf("device report\r\n");
        cJSON *pRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pRoot, "name", user_config.name);
        cJSON_AddStringToObject(pRoot, "ip", zlib_wifi_get_ip_str());
        cJSON_AddStringToObject(pRoot, "mac", zlib_wifi_get_mac_str());
        cJSON_AddNumberToObject(pRoot, "type", TYPE);
        cJSON_AddStringToObject(pRoot, "type_name", TYPE_NAME);

        char json_temp_str[64] = { 0 };
        os_sprintf(json_temp_str, "[\"%s\",\"%s\",\"%s\",\"%s\"]", user_mqtt_get_state_topic(),
                user_mqtt_get_set_topic(), user_mqtt_get_sensor_topic(), user_mqtt_get_will_topic());
        cJSON_AddItemToObject(pRoot, "topic", cJSON_Parse(json_temp_str));

        char *s = cJSON_Print(pRoot);
        os_printf("pRoot: [%s]\r\n", s);

        zlib_fun_wifi_send(arg, type, user_mqtt_get_state_topic(), s, 1, 0);

        cJSON_free((void *) s);
        cJSON_Delete(pRoot);
    }

    //解析
    //cJSON *p_name = cJSON_GetObjectItem(pJsonRoot, "name");
    cJSON *p_mac = cJSON_GetObjectItem(pJsonRoot, "mac");

    if((p_mac && cJSON_IsString(p_mac) && os_strcmp(p_mac->valuestring, zlib_wifi_get_mac_str()) != 0)
            && (p_mac && cJSON_IsString(p_mac) && os_strcmp(p_mac->valuestring, strlwr(TYPE_NAME)) != 0)
            && (type != WIFI_COMM_TYPE_HTTP && type != WIFI_COMM_TYPE_TCP)) return;

    cJSON *json_send = cJSON_CreateObject();
    //mac字段
    cJSON_AddStringToObject(json_send, "mac", zlib_wifi_get_mac_str());

    //version版本
    cJSON *p_version = cJSON_GetObjectItem(pJsonRoot, "version");
    if(p_version)
    {
        cJSON_AddStringToObject(json_send, "version", VERSION);
    }
    //重启命令
    if(p_cmd && cJSON_IsString(p_cmd) && strcmp(p_cmd->valuestring, "restart") == 0)
    {
        os_printf("cmd:restart");
        zlib_reboot_delay(2000);
        cJSON_AddStringToObject(json_send, "cmd", "restart");
    }

    //返回wifi ssid及rssi
    cJSON *p_ssid = cJSON_GetObjectItem(pJsonRoot, "ssid");
    if(p_ssid)
    {
        struct station_config ssidGet;
        if(wifi_station_get_config(&ssidGet))
        {
            cJSON_AddStringToObject(json_send, "ssid", ssidGet.ssid);
            cJSON_AddNumberToObject(json_send, "rssi", ssidGet.threshold.rssi);
        }
        else
        {
            cJSON_AddStringToObject(json_send, "ssid", "get wifi_ssid fail");
        }
    }

    //查询上一次串口接收到的内容
    cJSON *p_uart_last = cJSON_GetObjectItem(pJsonRoot, "uart_last");
    if(p_uart_last)
    {
        uint16_t re_num;
        uint8_t * re_data;
        re_data = user_uart_get_data(&re_num);

        char send_str[UART_REC_BUFF_MAX * 2 + 1] = { 0 };
        char *p = send_str;
        for (i = 0; i < re_num; i++)
        {
            os_sprintf(p, "%02X", re_data[i]);
            p += 2;
        }
        *p = 0;
        cJSON_AddStringToObject(json_send, "uart_last", send_str);

    }

    cJSON *p_setting = cJSON_GetObjectItem(pJsonRoot, "setting");
    if(p_setting)
    {
        //解析ota
        uint8_t userBin = system_upgrade_userbin_check() + 1;
        cJSON *p_ota1 = cJSON_GetObjectItem(p_setting, "ota1");
        cJSON *p_ota2 = cJSON_GetObjectItem(p_setting, "ota2");
        if(p_ota1 && userBin - 1 == UPGRADE_FW_BIN2 && cJSON_IsString(p_ota1))
        {
            zlib_ota_start(p_ota1->valuestring);
        }
        else if(p_ota2 && userBin - 1 == UPGRADE_FW_BIN1 && cJSON_IsString(p_ota2))
        {
            zlib_ota_start(p_ota2->valuestring);
        }

        //设置设备名称
        cJSON *p_setting_name = cJSON_GetObjectItem(p_setting, "name");
        if(p_setting_name && cJSON_IsString(p_setting_name))
        {
            update_user_config_flag = true;
            os_sprintf(user_config.name, p_setting_name->valuestring);
        }

        //设置wifi ssid
        cJSON *p_setting_wifi_ssid = cJSON_GetObjectItem(p_setting, "wifi_ssid");
        cJSON *p_setting_wifi_password = cJSON_GetObjectItem(p_setting, "wifi_password");
        if(p_setting_wifi_ssid && cJSON_IsString(p_setting_wifi_ssid) && p_setting_wifi_password
                && cJSON_IsString(p_setting_wifi_password))
        {
            zlib_wifi_set_ssid_delay(p_setting_wifi_ssid->valuestring, p_setting_wifi_password->valuestring, 1000);
        }

        //设置mqtt ip
        cJSON *p_mqtt_ip = cJSON_GetObjectItem(p_setting, "mqtt_uri");
        if(p_mqtt_ip && cJSON_IsString(p_mqtt_ip))
        {
            update_user_config_flag = true;
            os_sprintf(user_config.mqtt_ip, p_mqtt_ip->valuestring);
        }

        //设置mqtt port
        cJSON *p_mqtt_port = cJSON_GetObjectItem(p_setting, "mqtt_port");
        if(p_mqtt_port && cJSON_IsNumber(p_mqtt_port))
        {
            update_user_config_flag = true;
            user_config.mqtt_port = p_mqtt_port->valueint;
        }

        //设置mqtt user
        cJSON *p_mqtt_user = cJSON_GetObjectItem(p_setting, "mqtt_user");
        if(p_mqtt_user && cJSON_IsString(p_mqtt_user))
        {
            update_user_config_flag = true;
            os_sprintf(user_config.mqtt_user, p_mqtt_user->valuestring);
        }

        //设置mqtt password
        cJSON *p_mqtt_password = cJSON_GetObjectItem(p_setting, "mqtt_password");
        if(p_mqtt_password && cJSON_IsString(p_mqtt_password))
        {
            update_user_config_flag = true;
            os_sprintf(user_config.mqtt_password, p_mqtt_password->valuestring);
        }

        //配置setting返回数据
        cJSON *json_setting_send = cJSON_CreateObject();
        cJSON *p_userbin = cJSON_GetObjectItem(p_setting, "userbin");
        if(p_userbin || p_ota1 || p_ota2)
        {
            cJSON_AddNumberToObject(json_setting_send, "userbin", userBin);
        }
        //返回设备ota
        if(p_ota1) cJSON_AddStringToObject(json_setting_send, "ota1", p_ota1->valuestring);
        if(p_ota2) cJSON_AddStringToObject(json_setting_send, "ota2", p_ota2->valuestring);

        //设置设备名称
        if(p_setting_name)
        cJSON_AddStringToObject(json_setting_send, "name", user_config.name);

        //设置设备wifi
        if(p_setting_wifi_ssid || p_setting_wifi_password)
        {
            cJSON_AddStringToObject(json_setting_send, "wifi_ssid", p_setting_wifi_ssid->valuestring);
            cJSON_AddStringToObject(json_setting_send, "wifi_password", p_setting_wifi_password->valuestring);
        }

        //设置mqtt ip
        if(p_mqtt_ip)
        cJSON_AddStringToObject(json_setting_send, "mqtt_uri", user_config.mqtt_ip);

        //设置mqtt port
        if(p_mqtt_port)
        cJSON_AddNumberToObject(json_setting_send, "mqtt_port", user_config.mqtt_port);

        //设置mqtt user
        if(p_mqtt_user)
        cJSON_AddStringToObject(json_setting_send, "mqtt_user", user_config.mqtt_user);

        //设置mqtt password
        if(p_mqtt_password)
        cJSON_AddStringToObject(json_setting_send, "mqtt_password", user_config.mqtt_password);

        cJSON_AddItemToObject(json_send, "setting", json_setting_send);

        if(p_mqtt_ip && cJSON_IsString(p_mqtt_ip) && p_mqtt_port && cJSON_IsNumber(p_mqtt_port) && p_mqtt_user
                && cJSON_IsString(p_mqtt_user) && p_mqtt_password && cJSON_IsString(p_mqtt_password)
                && !zlib_mqtt_is_connected())
        {
            zlib_reboot_delay(500);
        }
    }

    cJSON_AddStringToObject(json_send, "name", user_config.name);

    //解析任务字段
    for (i = 0; i < TASK_NUM; i++)
    {
        if(json_task_analysis(i, pJsonRoot, json_send)) update_user_config_flag = true;
    }

    char *json_str = cJSON_Print(json_send);
    os_printf("json_send: %s\r\n", json_str);
    zlib_fun_wifi_send(arg, type, user_mqtt_get_state_topic(), json_str, 1, retained);
    cJSON_free((void *) json_str);

    cJSON_Delete(json_send);
    if(update_user_config_flag)
    {
        os_timer_disarm(&_timer_json);
        os_timer_setfn(&_timer_json, (os_timer_func_t *) _json_timer_fun, NULL);
        os_timer_arm(&_timer_json, 2000, false); //2000毫秒后保存

        //zlib_setting_save_config(&user_config, sizeof(user_config_t));
        update_user_config_flag = false;
    }

}
/**
 * 函  数  名: user_json_init
 * 函数说明: json数据处理初始化
 * 参        数: 无
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR user_json_init(void)
{

    zlib_json_init(_json_deal_cb);
    os_printf("user json init\n");
}
