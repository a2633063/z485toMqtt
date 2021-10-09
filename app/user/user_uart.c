#include <ctype.h>
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "user_uart.h"

#define UART_REC_DELAY   80 //多长时间内接收到的数据为一包,单位ms
#define UART_REC_BUFF_MAX 200
static uint8_t re_data[UART_REC_BUFF_MAX];
static uint16_t re_num = 0;

/**
 * 函  数  名: _uart_timer_fun
 * 函数说明: uart定時器回调函数,超时后将收到的数据发出去
 * 参        数: 无
 * 返        回: 无
 */

static os_timer_t _timer_uart;
static void _uart_timer_fun(void *arg)
{
    uint16_t i;
    char send_str[UART_REC_BUFF_MAX * 2] = { 0 };
    char *p = send_str;
    for (i = 0; i < re_num; i++)
    {
        os_sprintf(p, "%02X", re_data[i]);
        p += 2;
    }
    *p = 0;
    if(!zlib_mqtt_send_byte(user_mqtt_get_state_topic(), send_str, os_strlen(send_str), 1, 0))
    {
        zlib_udp_reply(NULL, send_str, os_strlen(send_str));
    }
//    if(!zlib_mqtt_send_byte(user_mqtt_get_state_topic(), re_data, re_num, 1, 0))
//    {
//        zlib_udp_reply(NULL,re_data,re_num);
//    }
    re_num = 0;
}
void ICACHE_FLASH_ATTR user_uart_receive(uint8_t dat)
{
    os_timer_disarm(&_timer_uart);

    re_data[re_num] = dat;
    re_num++;
    if(re_num >= UART_REC_BUFF_MAX)
    {
        _uart_timer_fun(NULL);
    }

    os_timer_setfn(&_timer_uart, (os_timer_func_t *) _uart_timer_fun, NULL);
    os_timer_arm(&_timer_uart, UART_REC_DELAY, false); //100毫秒后发送
}
