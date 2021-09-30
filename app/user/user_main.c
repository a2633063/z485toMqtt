#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
void ICACHE_FLASH_ATTR _ota_result_cb(state_ota_result_state_t b)
{
    os_printf("_ota_result_cb: %d\r\n", b);
}
bool ICACHE_FLASH_ATTR _json_deal_cb(void *arg, Wifi_Comm_type_t type, cJSON * pJsonRoot, void *p)
{

    cJSON *p_ota1 = cJSON_GetObjectItem(pJsonRoot, "ota");
    if(p_ota1)
    {
        zlib_ota_start(p_ota1->valuestring);
    }

    char *s = cJSON_Print(pJsonRoot);
    os_printf("pJsonRoot: \r\n%s\r\n", s);

    switch (type)
    {
        case WIFI_COMM_TYPE_UDP:
            zlib_udp_reply(arg, s);
            break;
        case WIFI_COMM_TYPE_TCP:
            zlib_tcp_reply(arg, s);
            break;
        case WIFI_COMM_TYPE_HTTP:
            zlib_web_server_reply(arg, true, APPLICATIOIN_JSON, s);
            break;
    }
    cJSON_free((void *) s);

}

user_config_t user_config;
static os_timer_t _timer_main;
static void _user_timer_fun(void *arg)
{
    static uint32_t size_last;
    uint32_t size;
    size = system_get_free_heap_size();
    if(size_last != size)
    {
        size_last = size;
        os_printf("system_get_free_heap_size:%d\n", size);
    }
}
void user_init(void)
{
    uint8_t i;
    uart_init(115200, 115200);
    //os_delay_us(60000);
    os_printf(" \n \nStart user%d.bin\n", system_upgrade_userbin_check() + 1);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    os_printf("FW version:%s\n", VERSION);
    //UART_SetPrintPort(1);
    os_printf(" \n \nStart user%d.bin\n", system_upgrade_userbin_check() + 1);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    os_printf("FW version:%s\n", VERSION);

    zlib_wifi_init(false);

    zlib_web_wifi_init();
    //zlib_mqtt_init(1);
//	user_io_init();
//	user_setting_init();
//	user_wifi_init();
//	user_sntp_init();
//	user_os_timer_init();
    //UDP初始化,监听端口10182,当接收到特定字符串时,返回本设备IP及MAC地址
//	user_devicefind_init(10182);
//	user_webserver_init(80);
//	TCP初始化,监听端口10191
//	user_tcp_init(10191);
    //TCP初始化 80端口 webserver
    //user_webserver_init(80);

    zlib_udp_init(10181);
    zlib_tcp_init(10182);
    zlib_json_init(_json_deal_cb);
    zlib_ota_set_result_callback(_ota_result_cb);

    os_timer_disarm(&_timer_main);
    os_timer_setfn(&_timer_main, (os_timer_func_t *) _user_timer_fun, NULL);
    os_timer_arm(&_timer_main, 1000, true);

}
