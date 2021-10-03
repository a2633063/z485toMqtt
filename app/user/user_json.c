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

    cJSON *p_ota1 = cJSON_GetObjectItem(pJsonRoot, "ota");
    if(p_ota1)
    {
        zlib_ota_start(p_ota1->valuestring);
    }

    char *s = cJSON_Print(pJsonRoot);
    os_printf("pJsonRoot: \r\n%s\r\n", s);


    //zlib_fun_wifi_send(arg, type, user_get_state_topic(), s, 1, 0);

    cJSON_free((void *) s);

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
