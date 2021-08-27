#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "circular_buffer.h"
#include "stm32f4xx_hal.h"

#define MAX_DATA_CHUNK_SIZE     (100) 
#define RX_DATA_BUFF_SIZE       (512)
#define TX_DATA_BUFF_SIZE       (512)

uint8_t uart_init(void);
uint8_t uart_get_rx_data_len(void);
uint8_t uart_read_rx_data(uint8_t *data, uint8_t len);
uint8_t uart_fetch_rx_data(uint8_t *data, uint8_t len);
uint8_t uart_clear_rx_data(void);
uint8_t uart_transmit(uint8_t *data, uint8_t len);
uint8_t uart_transmit_it(uint8_t *data, uint8_t len);
uint8_t uart_write_rx_data(uint8_t *data, uint8_t len);

#endif