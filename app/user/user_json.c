#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"

#include "user_mqtt.h"

/**
 * 函  数  名: _json_deal_cb
 * 函数说明: json数据处理初始化回调函数,在此函数中处理json
 * 参        数: 无
 * 返        回: 无
 */
static void ICACHE_FLASH_ATTR _json_deal_cb(void *arg, Wifi_Comm_type_t type, cJSON * pJsonRoot, void *p)
{
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
        char *s = cJSON_Print(pRoot);
        os_printf("pRoot: [%s]\r\n", s);

        zlib_fun_wifi_send(arg, type, user_mqtt_get_state_topic(), s, 1, 0);

        cJSON_free((void *) s);
        cJSON_Delete(pRoot);
    }

    //解析
    cJSON *p_name = cJSON_GetObjectItem(pJsonRoot, "name");
    cJSON *p_mac = cJSON_GetObjectItem(pJsonRoot, "mac");

    if((p_mac && cJSON_IsString(p_mac) && os_strcmp(p_mac->valuestring, zlib_wifi_get_mac_str()) != 0)
            && (p_mac && cJSON_IsString(p_mac) && os_strcmp(p_mac->valuestring, strlwr(TYPE_NAME)) != 0)) return;

    cJSON *json_send = cJSON_CreateObject();
    //mac字段
    cJSON_AddStringToObject(json_send, "mac", zlib_wifi_get_mac_str());

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

    cJSON *p_setting = cJSON_GetObjectItem(pJsonRoot, "setting");
    if(p_setting)
    {
        //解析ota
        uint8_t userBin = system_upgrade_userbin_check();
        cJSON *p_ota1 = cJSON_GetObjectItem(p_setting, "ota1");
        cJSON *p_ota2 = cJSON_GetObjectItem(p_setting, "ota2");
        if(p_ota1 && userBin == UPGRADE_FW_BIN2 && cJSON_IsString(p_ota1))
        {
            zlib_ota_start(p_ota1->valuestring);
        }
        else if(p_ota2 && userBin == UPGRADE_FW_BIN1 && cJSON_IsString(p_ota2))
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

    char *json_str = cJSON_Print(json_send);
    os_printf("json_send: %s\r\n", json_str);
    zlib_fun_wifi_send(arg, type, user_mqtt_get_state_topic(), json_str, 1, retained);
    cJSON_free((void *) json_str);

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
