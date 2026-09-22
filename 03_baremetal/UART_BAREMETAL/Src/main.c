#include <stdint.h>
#include "vara_uart.h"

int main(void) {
    usart1_init();

    while(1) {
        // Transmit "Hello" every cycle
        usart1_write_string("Hello\r\n");

        // Wait approximately 1 second while still polling for incoming bytes
        volatile uint32_t count = 1000000U;
        while (count-- != 0U) {
            if (usart1_data_available()) {
                char received_char = usart1_read_char();
                usart1_write_string("Received: ");
                usart1_write_char(received_char);
                usart1_write_string("\r\n");  // CR+LF
            }
        }
    }

    return 0;
}
