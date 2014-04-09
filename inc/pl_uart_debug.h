#ifndef _pl_uart_debug_h_
#define _pl_uart_debug_h_

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"

void init_debug_usart(void);
void init_debug_usart_lib(void);
void usart_put_string( char *s);
void usart_put_int( uint16_t word );
void usart_put_word( uint16_t word );

#endif//_pl_uart_debug_h_
