#include <stdint.h>
#include "vara_uart.h"

// ============================================================================
// 1. ADC & CLOCK REGISTER DEFINITIONS
// ============================================================================
#define RCC_BASE            0x40021000UL
#define GPIOA_BASE          0x40010800UL
#define ADC1_BASE           0x40012400UL

#define RCC_CFGR            (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define GPIOA_CRL           (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

#define ADC1_SR             (*(volatile uint32_t *)(ADC1_BASE + 0x00))
#define ADC1_CR2            (*(volatile uint32_t *)(ADC1_BASE + 0x08))
#define ADC1_SMPR2          (*(volatile uint32_t *)(ADC1_BASE + 0x10))
#define ADC1_SQR1           (*(volatile uint32_t *)(ADC1_BASE + 0x2C))
#define ADC1_SQR3           (*(volatile uint32_t *)(ADC1_BASE + 0x34))
#define ADC1_DR             (*(volatile uint32_t *)(ADC1_BASE + 0x4C))

// ============================================================================
// 2. HELPER & DELAY FUNCTIONS
// ============================================================================
void delay_ms(volatile uint32_t ms) {
    volatile uint32_t count;
    while (ms--) {
        for (count = 0; count < 8000; count++) {
            __asm("nop");
        }
    }
}

// Helper to print numbers using your usart1_write_char function
void usart1_write_num(uint32_t num) {
    char buf[10];
    int i = 0;
    if (num == 0) {
        usart1_write_char('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        usart1_write_char(buf[--i]);
    }
}

// ============================================================================
// 3. ADC DRIVER INITIALIZATION & READ
// ============================================================================
void adc1_init(void) {
    // 1. Enable clocks for GPIOA (Bit 2) and ADC1 (Bit 9) on APB2 bus
    RCC_APB2ENR |= (1 << 2) | (1 << 9);

    // 2. Set ADC Prescaler: PCLK2 / 6 = 12 MHz (ADCCLK must be <= 14 MHz)[cite: 2]
    RCC_CFGR &= ~(0x3U << 14);
    RCC_CFGR |=  (0x2U << 14);

    // 3. Configure PA0 as Analog Input (CNF0 = 00, MODE0 = 00)
    GPIOA_CRL &= ~(0xF << 0);

    // 4. Set Sampling Time for Channel 0 to 55.5 cycles (SMP0[2:0] = 101)
    ADC1_SMPR2 &= ~(0x7U << 0);
    ADC1_SMPR2 |=  (0x5U << 0);

    // 5. Set Regular Sequence Length L[3:0] = 0000 (1 conversion) on Channel 0
    ADC1_SQR1 &= ~(0xFU << 20);
    ADC1_SQR3 &= ~(0x1FU << 0);

    // 6. Configure Trigger Source to Software Trigger (EXTSEL = 111, EXTTRIG = 1)[cite: 1]
    ADC1_CR2 |= (7 << 17) | (1 << 20);

    // 7. Power ON ADC1 (Wake up from power-down mode)[cite: 1]
    ADC1_CR2 |= (1 << 0);
    delay_ms(1); // Stabilization delay (tSTAB)[cite: 1]

    // 8. Reset Calibration Registers[cite: 1]
    ADC1_CR2 |= (1 << 3);
    while (ADC1_CR2 & (1 << 3)); // Wait for RSTCAL to clear[cite: 1]

    // 9. Run Self-Calibration[cite: 1]
    ADC1_CR2 |= (1 << 2);
    while (ADC1_CR2 & (1 << 2)); // Wait for CAL to clear[cite: 1]
}

uint16_t adc1_read(void) {
    // 1. Start conversion via Software Trigger (SWSTART bit 22)
    ADC1_CR2 |= (1 << 22);

    // 2. Wait for End of Conversion flag (EOC, Bit 1 in ADC1_SR)
    while (!(ADC1_SR & (1 << 1)));

    // 3. Read 12-bit result (Reading ADC1_DR automatically clears EOC)
    return (uint16_t)(ADC1_DR & 0xFFF);
}

// ============================================================================
// 4. MAIN PROGRAM
// ============================================================================
int main(void) {
    // Initialize your custom UART driver
    usart1_init();

    // Initialize ADC1 on PA0
    adc1_init();

    usart1_write_string("\r\n==========================================\r\n");
    usart1_write_string(" STM32F103 ADC1 & Vara UART Ready         \r\n");
    usart1_write_string("==========================================\r\n\r\n");

    while (1) {
        uint16_t ldr_raw = adc1_read();

        usart1_write_string("ADC Raw Value (0-4095): ");
        usart1_write_num(ldr_raw);
        usart1_write_string("\r\n");

        delay_ms(50); // 500 ms sampling interval
    }

    return 0;
}
