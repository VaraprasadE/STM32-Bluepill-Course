/*
 * vara_uart.h
 *
 *  Created on: 18-Sept-2026
 *      Author: varap
 */

#ifndef VARA_UART_H_
#define VARA_UART_H_

// Register Address Definitions
#define RCC_BASE            0x40021000UL
#define GPIOA_BASE          0x40010800UL
#define USART1_BASE         0x40013800UL

void usart1_init(void);
void usart1_write_char(char c);
void usart1_write_string(const char *str);
char usart1_read_char(void);
int usart1_data_available(void);

#endif /* VARA_UART_H_ */
