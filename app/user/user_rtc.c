#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "zlib_rtc.h"

void _user_rtc_fun(const struct_time_t time)
{
    LOGI("%04d/%02d/%02d %d %02d:%02d:%02d\n", time.year, time.month, time.day, time.week, time.hour, time.minute,
            time.second);

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

