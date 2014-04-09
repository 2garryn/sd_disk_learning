#include "pl_uart_debug.h"


#define Pin5 ((uint16_t)0x0020)
#define Pin5Source 0x05
#define USART2_AF 0x07

void init_debug_usart_lib() {
  GPIO_InitTypeDef gpio;
  USART_InitTypeDef usart;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  

  GPIO_StructInit(&gpio);
  
  gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_Pin = GPIO_Pin_5;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOD, &gpio);

  GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);


  
  USART_StructInit(&usart);
  usart.USART_Mode = USART_Mode_Tx;
  usart.USART_BaudRate = 9600;	
  usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart.USART_WordLength = USART_WordLength_8b;
  usart.USART_StopBits = USART_StopBits_1;
  usart.USART_Parity = USART_Parity_No;
  
  USART_Init(USART2, &usart);	
  
  USART2->BRR = 0x0000016d;

  USART_Cmd(USART2, ENABLE);
}
 



void init_debug_usart(void) {

  //enable usart2 clock
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
  //enable GPIOD clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  // enable alternate function for PIn 5
  GPIOD->MODER |= GPIO_MODER_MODER5_1;
  // set speed
  GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;
  //set push pull mode
  GPIOD->OTYPER  &= ~GPIO_OTYPER_OT_5;
  //pull_up/pull_down = none
  GPIOD->PUPDR   &= ~GPIO_PUPDR_PUPDR5;
  //enable alternate func. for pin
  GPIOD->AFR[0] |= ((uint32_t) USART2_AF) << (Pin5Source * 4);
  //calculated. 42 Mhz bus freq. 115200 - baudrate
  USART2->BRR = 0x0000016d;
  //USART enabled, transmission enabled. Parity disabled by default, Word length 1 start bit, 8 data bits. 1 stopbit
  USART2->CR1 |= USART_CR1_UE | USART_CR1_TE;  
  
}

void usart_put_string( char *s){
	while(*s){
	  usart_put_word((uint16_t) (*s));
	  ++s;
	}
}


void usart_put_word( uint16_t word ) {
  while( !(USART2->SR & USART_SR_TC) );
  USART2->DR = word;
  while( !(USART2->SR & USART_SR_TC) );
};


#define STRING_LENGTH 6

void usart_put_int( uint16_t word ) {
  char str[STRING_LENGTH];
  str[STRING_LENGTH - 1] = 0;
  int8_t i;
  for (i = STRING_LENGTH - 2; i >= 0; i--) {
    str[i] = (word % 10) + 48;
    word /= 10;
  };
  usart_put_string(str);
}
