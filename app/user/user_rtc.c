#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "zlib_rtc.h"
#include "user_function.h"

void _user_rtc_fun(const struct_time_t time)
{
    uint8_t i;
    static uint8_t minute_last = 80;
    if(minute_last != time.minute)
    {
        minute_last = time.minute;
        LOGI("%04d/%02d/%02d %d %02d:%02d:%02d\n", time.year, time.month, time.day, time.week, time.hour, time.minute,
                time.second);

        for (i = 0; i < TASK_NUM; i++)
        {
            user_config_task_t *task = &user_config.task[i];
            if(task->on == 0) continue;   //自动化任务未开,忽略
            if(task->type != TASK_TYPE_TIME_MQTT && task->type != TASK_TYPE_TIME_UART) continue; //非定时触发任务,忽略

            //开始对比时间/重复是否符合要求
            if(task->hour != time.hour || task->minute != time.minute || (task->repeat & (1 << (time.week - 1))) == 0)
                continue; //未到定时触发时间,忽略

            //符合自动化条件 ,开始执行自动化
            user_fun_task_deal(task);

        }
    }

    static uint32_t size_last;
    uint32_t size;
    size = system_get_free_heap_size();
    if(size_last != size)
    {
        size_last = size;
        os_printf("system_get_free_heap_size:%d\n", size);
    }
}
/**
 * 函  数  名: user_rtc_init
 * 函数说明: rtc初始化
 * 参        数: 无
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR user_rtc_init(void)
{
    zlib_rtc_set_timezone(8);
    zlib_rtc_init();
    zlib_rtc_set_recall_callback(_user_rtc_fun);
    os_printf("user rtc init\n");
}

