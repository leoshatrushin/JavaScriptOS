#pragma once
#include "types.h"

#define COM1_PORT 0x3F8

// register offsets
#define DATA 0
#define INTERRUPT_ENABLE 1
#define DIVISOR_LSB 0 // DLAB = 1
#define DIVISOR_MSB 1 // DLAB = 1
#define INTERRUPT_IDENTIFICATION 2
#define FIFO_CONTROL 2
#define LINE_CONTROL 3
#define MODEM_CONTROL 4
#define LINE_STATUS 5
#define MODEM_STATUS 6
#define SCRATCH 7

typedef union {
    struct {
    u8 data_available : 1;
    u8 transmitter_empty : 1;
    u8 receiver_line_status : 1;
    u8 modem_status : 1;
    u8 rsrv0 : 4;
    };
    u8 raw;
} __attribute__((packed)) interrupt_enable;

enum uart_data_bits {
    DATA_BITS_5 = 0,
    DATA_BITS_6 = 1,
    DATA_BITS_7 = 2,
    DATA_BITS_8 = 3,
};

enum uart_stop_bits {
    STOP_BITS_1 = 0,
    STOP_BITS_2 = 1, // 1.5 if data_bits = 5
};

enum uart_parity {
    PARITY_NONE = 0,
    PARITY_ODD = 1, // including parity bit
    PARITY_EVEN = 3, // including parity bit
    PARITY_MARK = 5, // parity bit always 0
    PARITY_SPACE = 7, // parity bit always 1
};

typedef union {
    struct {
    enum uart_data_bits data_bits : 2; // for ASCII, probably only need 7, less is faster
    enum uart_stop_bits stop_bits : 1; // used by controller to verify devices are in phase
    enum uart_parity parity : 3; // 
    u8 break_enable : 1;
    u8 dlab : 1; // divisor latch access bit, set to send 16-bit divisor
    };
    u8 raw;
} __attribute__((packed)) line_control;

enum uart_fifo_trigger_level {
    _1B = 0,
    _4B = 1,
    _8B = 2,
    _14B = 3,
};

typedef union {
    struct {
    u8 en : 1;
    u8 clear_receive_fifo : 1; // auto-resets when complete
    u8 clear_transmit_fifo : 1; // auto-resets when complete
    u8 dma_mode_select : 1;
    u8 rsrv0 : 2;
    enum uart_fifo_trigger_level interrupt_trigger_level : 2;
    };
    u8 raw;
} __attribute__((packed)) fifo_control;

typedef union {
    struct {
    u8 data_terminal_ready : 1;
    u8 request_to_send : 1;
    u8 out1 : 1; // unused in PC implementations
    u8 out2 : 1; // enable IRQ in PC implementations
    u8 loopback_enable : 1; // diagnostic feature
    u8 ignored : 3;
    };
    u8 raw;
} __attribute__((packed)) modem_control;

typedef union {
    struct {
    u8 data_ready : 1;
    u8 overrun_error : 1; // data lost
    u8 parity_error : 1; // transmission error
    u8 framing_error : 1; // missing stop bit
    u8 break_indicator : 1; // break in data input
    u8 transmitter_buffer_empty : 1;
    u8 transmitter_empty : 1;
    u8 input_error : 1;
    };
    u8 raw;
} __attribute__((packed)) line_status;
