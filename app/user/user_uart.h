#ifndef __USER_UART_H__
#define __USER_UART_H__

extern void user_uart_receive(uint8_t dat);
extern uint8_t * user_uart_get_data(uint16_t *length);
#endif

