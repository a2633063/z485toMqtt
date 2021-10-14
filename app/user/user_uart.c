#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "user_uart.h"
#include "user_mqtt.h"
#include "user_function.h"

#define UART_REC_DELAY   80 //多长时间内接收到的数据为一包,单位ms
#define UART_REC_BUFF_MAX 200
static uint8_t re_data[UART_REC_BUFF_MAX];  //当前接收到的数据
static uint16_t re_num = 0;

static uint8_t re_data_last[UART_REC_BUFF_MAX]; //记录上一次收到的一帧数据
static uint16_t re_num_last = 0;

/**
 * 函  数  名: user_uart_get_data
 * 函数说明: uart定時器回调函数,超时后将收到的数据发出去
 * 参        数: length:获取到的长度
 * 返        回: 无
 */
uint8_t * ICACHE_FLASH_ATTR user_uart_get_data(uint16_t *length)
{
    *length = re_num_last;
    return re_data_last;
}
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
    uint8_t x, j;
    char send_str[UART_REC_BUFF_MAX * 2] = { 0 };
    char *p = send_str;

    os_memcpy(re_data_last, re_data, re_num);
    re_num_last = re_num;
    re_num = 0;

    os_printf("uart rec length:%d\n", re_num_last);
    for (x = 0; x < TASK_NUM; x++)
    {
        user_config_task_t *task = &user_config.task[x];

        if(task->on == 0) continue;   //自动化任务未开,忽略
        if(task->type != TASK_TYPE_MQTT && task->type != TASK_TYPE_WOL && task->type != TASK_TYPE_UART
                && task->type != TASK_TYPE_HTTP) continue; //非串口触发任务,忽略
        if(task->dat_length < 1 || task->dat_length > TASK_DATA_REC_BUF_LENGTH) continue;   //无有效数据
        if(task->dat_length > re_num_last) continue; //自动化任务触发条件比接收到的数据长,忽略

        //开始对比是否是 需要接收的数据
        uint8_t re_cmp[TASK_DATA_REC_BUF_LENGTH];
        os_memcpy(re_cmp, re_data_last, task->dat_length);
        if(task->reserved > 0 && task->reserved <= TASK_DATA_REC_BUF_LENGTH)
        {   //处理需要忽略的字节
            re_cmp[task->reserved - 1] = task->condition_dat[task->reserved - 1];
        }
        if(os_memcmp(re_cmp, task->condition_dat, task->dat_length) != 0) continue; //不相同,忽略

        os_printf("task uart length:%d\n", task->dat_length);
        //符合自动化条件 ,开始执行自动化
        os_printf("uart task[%d] trigger\n", x);
        user_fun_task_deal(task);

        if(task->mqtt_send)
            break;
        else
            goto EXIT;
    }
    os_printf("uart rec x:%d\n", x);

    for (i = 0; i < re_num_last; i++)
    {
        os_sprintf(p, "%02X", re_data_last[i]);
        p += 2;
    }
    *p = 0;
    os_printf("uart rec:%s", send_str);
    if(!zlib_mqtt_send_byte(user_mqtt_get_state_topic(), send_str, os_strlen(send_str), 1, 0))
    {
        zlib_udp_reply(NULL, send_str, os_strlen(send_str));
    }
//    if(!zlib_mqtt_send_byte(user_mqtt_get_state_topic(), re_data_last, re_num_last, 1, 0))
//    {
//        zlib_udp_reply(NULL,re_data_last,re_num_last);
//    }
    EXIT: return;
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
    os_timer_arm(&_timer_uart, UART_REC_DELAY, false); //80毫秒后发送
}
