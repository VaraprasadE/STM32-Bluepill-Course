/*
 * vara_uart.c
 *
 *  Created on: 18-Sept-2026
 *      Author: varap
 */

#include <stdint.h>
#include<vara_uart.h>

#define RCC_APB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define GPIOA_CRH           (*(volatile uint32_t *)(GPIOA_BASE + 0x04))

#define USART1_SR           (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR           (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR          (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1          (*(volatile uint32_t *)(USART1_BASE + 0x0C))


void usart1_init(void) {
    // 1. Enable Clocks for GPIOA and USART1
    RCC_APB2ENR |= (1 << 2) | (1 << 14); // Set IOPAEN (bit 2) and USART1EN (bit 14)

    // 2. Configure PA9 (TX) as Alternate Function Push-Pull (50MHz) -> 0xB
    //    Configure PA10 (RX) as Floating Input -> 0x4
    GPIOA_CRH &= ~(0xFFU << 4);         // Clear configuration for PA9 and PA10
    GPIOA_CRH |=  (0x4BU << 4);         // Write 0x4 (PA10) and 0xB (PA9) -> 0x4B shifted

    // 3. Set Baud Rate to 9600 assuming a default 8 MHz HSI clock
    //    USARTDIV = 8,000,000 / (16 * 9600) = 52.0833
    //    Mantissa = 52 (0x34), Fraction = 1 (0x1) -> BRR = 0x0341
    USART1_BRR = 0x0341;

    // 4. Configure Control Register 1: Enable TX, RX, and USART
    USART1_CR1 |= (1 << 3) | (1 << 2) | (1 << 13); // Set TE (bit 3), RE (bit 2), and UE (bit 13)
}


void usart1_write_char(char c) {
    // Wait until Transmit Data Register is Empty (TXE, Bit 7)
    while (!(USART1_SR & (1 << 7)));

    // Write character to Data Register
    USART1_DR = (c & 0xFF);
}


void usart1_write_string(const char *str) {
    while (*str) {
        usart1_write_char(*str++);
    }
}


char usart1_read_char(void) {
    // Wait until Read Data Register is Not Empty (RXNE, Bit 5)
    while (!(USART1_SR & (1 << 5)));

    // Read and return the character
    return (char)(USART1_DR & 0xFF);
}


int usart1_data_available(void) {
    // Check if Read Data Register Not Empty (RXNE, Bit 5) is set
    return (USART1_SR & (1 << 5)) != 0;
}
