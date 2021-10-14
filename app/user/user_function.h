#ifndef __USER_FUNCTION_H__
#define __USER_FUNCTION_H__

int16_t ICACHE_FLASH_ATTR char2nibble(char c);
extern void user_fun_send_wol(uint8_t mac[6], uint8_t ip[4], uint16_t port, uint8_t secure[6]);
extern void user_fun_task_deal(user_config_task_t *task);

#endif

