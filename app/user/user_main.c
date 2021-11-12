#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "user_mqtt.h"
#include "zlib_rtc.h"
#include "user_json.h"
#include "user_key.h"
#include "user_setting.h"
#include "user_rtc.h"

user_config_t user_config;

void ICACHE_FLASH_ATTR _ota_result_cb(state_ota_result_state_t b)
{
    os_printf("_ota_result_cb: %d\r\n", b);
}

//void ICACHE_FLASH_ATTR system_init_done(void){}

void user_init(void)
{
    uint8_t i;
    uart_init(115200, 115200);
    //os_delay_us(60000);
    os_printf(" \n \nStart user%d.bin\n", system_upgrade_userbin_check() + 1);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    os_printf("FW version:%s\n", VERSION);
    //os_printf("Set UART0 Port to IO13(Rx) IO15(Tx)\n");
    os_printf("Set Print Port to USART1 IO2\n");
    //system_uart_swap();
    UART_SetPrintPort(1);// 调试串口改为串口1

    os_printf(" \n \nStart user%d.bin\n", system_upgrade_userbin_check() + 1);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    os_printf("FW version:%s\n", VERSION);
    //os_printf("Set UART0 Port to IO13(Rx) IO15(Tx)\n");
    os_printf("Set Print Port to USART1 IO2\n");

    os_printf("system_get_free_heap_size:%d\n", system_get_free_heap_size());
    user_setting_init();


    zlib_wifi_init(false);
    zlib_web_config_init();

    user_mqtt_init();
    user_json_init();
    user_rtc_init();
//	user_io_init();
    user_key_init();
    zlib_udp_init(10182);
    zlib_tcp_init(10182);
    zlib_ota_set_result_callback(_ota_result_cb);


    //system_init_done_cb(system_init_done);

}
