//logger uart implementation

#include "logger.h"
#include "pl_uart_debug.h"


void logger_initialize() {
  init_debug_usart();
};

void logger_print(uint8_t str_size, char * str) {
  usart_put_string(str);
};
