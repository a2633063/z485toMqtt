#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"

#include "user_config.h"
#include "zlib.h"

user_config_t user_config;


void user_init(void)
{
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
//	user_io_init();
//	user_setting_init();
//	user_wifi_init();
//	user_sntp_init();
//	user_os_timer_init();
	//UDP��ʼ��,�����˿�10182,�����յ��ض��ַ���ʱ,���ر��豸IP��MAC��ַ
//	user_devicefind_init(10182);
//	user_webserver_init(80);
//	TCP��ʼ��,�����˿�10191
//	user_tcp_init(10191);
	//TCP��ʼ�� 80�˿� webserver
	//user_webserver_init(80);

}
