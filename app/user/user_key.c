#include <ctype.h>
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "zlib.h"
#include "key.h"
#include "user_key.h"

static struct keys_param keys;
static struct single_key_param *single_key[USER_KEY_NUM];


LOCAL void ICACHE_FLASH_ATTR user_key_long_press(void)
{
    zlib_wifi_AP();
}


void ICACHE_FLASH_ATTR user_key_init(void)
{
    single_key[0] = key_init_single(USER_KEY_0_IO_NUM, USER_KEY_0_IO_MUX, USER_KEY_0_IO_FUNC,
                                        user_key_long_press, NULL);

        keys.key_num = USER_KEY_NUM;
        keys.single_key = single_key;

        key_init(&keys);
}
