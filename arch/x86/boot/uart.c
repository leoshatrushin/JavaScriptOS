// Intel 8250 serial port (UART).
#include "types.h"
#include "uart.h"
#include "asm.h"
#include "console.h"
#include "ioapic.h"

static boolean uart_present;

void uart_write_reg(u8 offset, u8 value) {
    out8(COM1_PORT + offset, value);
}

u8 uart_read_reg(u8 offset) {
    return in8(COM1_PORT + offset);
}

static void uart_putc(char c) {
    if (!uart_present) return;
    while (!((line_status){ .raw = uart_read_reg(LINE_STATUS) }.transmitter_buffer_empty));
    uart_write_reg(DATA, c);
}

static int uart_getc(void) {
  if (!uart_present) return -1;
  if ((line_status){ .raw = uart_read_reg(LINE_STATUS) }.data_ready == 0) return -1;
  return uart_read_reg(DATA);
}

#define BAUD_RATE_MAX 115200
#define BAUD_RATE_DESIRED 9600

void init_uart() {
    // disable fifo buffers
    uart_write_reg(FIFO_CONTROL, (fifo_control){ .en = 0 }.raw);

    // set communication paramters
    uart_write_reg(LINE_CONTROL, (line_control){ .dlab = 1 }.raw);
    uart_write_reg(DIVISOR_LSB, BAUD_RATE_MAX / BAUD_RATE_DESIRED);
    uart_write_reg(DIVISOR_MSB, (BAUD_RATE_MAX / BAUD_RATE_DESIRED) >> 8);
    uart_write_reg(LINE_CONTROL, (line_control){
        .data_bits = DATA_BITS_8,
        .stop_bits = STOP_BITS_1,
        .parity = PARITY_NONE,
        .break_enable = 0,
        .dlab = 0,
    }.raw);

    // interrupt on data available
    uart_write_reg(INTERRUPT_ENABLE, (interrupt_enable){ .data_available = 1 }.raw);

    // perform loopback test
    uart_write_reg(MODEM_CONTROL,
            (modem_control){ .request_to_send = 1, .out1 = 1, .out2 = 1, .loopback_enable = 1 }.raw);
    uart_write_reg(DATA, 0xAE);
    if (uart_read_reg(DATA) != 0xAE) {
        debug_printf("Error: UART failed to initialize\n");
        return;
    }
    uart_present = true;
    uart_write_reg(MODEM_CONTROL, 0);

    ioapic_route(IRQ_COM1, 0, 0);

    for (char *s = "UART initialized\n"; *s; ++s) uart_putc(*s);
}
