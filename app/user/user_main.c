#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "user_mqtt.h"
#include "zlib_rtc.h"
#include "user_json.h"

user_config_t user_config;

void ICACHE_FLASH_ATTR _ota_result_cb(state_ota_result_state_t b)
{
    os_printf("_ota_result_cb: %d\r\n", b);
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

    zlib_setting_get_config(&user_config, sizeof(user_config_t));
    if(user_config.version != USER_CONFIG_VERSION)
    {
        char hwaddr[6];
        char strMac[16];
        //恢复默认设置
        wifi_get_macaddr(STATION_IF, hwaddr);
        os_sprintf(strMac, "%02x%02x%02x%02x%02x%02x", MAC2STR(hwaddr));
        os_sprintf(user_config.name, DEVICE_NAME, strMac + 8);
        os_sprintf(user_config.mqtt_ip, "");
        os_sprintf(user_config.mqtt_user, "");
        os_sprintf(user_config.mqtt_password, "");
        user_config.mqtt_port = 1883;
        user_config.version = USER_CONFIG_VERSION;
        os_printf("config version error.Restore default settings\n");
        zlib_setting_save_config(&user_config, sizeof(user_config_t));
    }

    zlib_wifi_init(false);

    zlib_web_config_init();
    user_mqtt_init();
    user_json_init();
    user_rtc_init();
//	user_io_init();

    zlib_udp_init(10182);
    zlib_tcp_init(10182);
    zlib_ota_set_result_callback(_ota_result_cb);


}
