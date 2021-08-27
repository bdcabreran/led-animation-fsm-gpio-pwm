/**
 * @file uart.c
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "uart_driver.h"

extern void Error_Handler(void);

/**@brief Enable/Disable debug messages */
#define UART_DRIVER_DEBUG 0
#define UART_DRIVER_TAG "uart driver : "

/**@brief uart debug function for server comm operations  */
#if UART_DRIVER_DEBUG
#define uart_driver_dbg(format, ...) printf(UART_DRIVER_TAG format, ##__VA_ARGS__)
#else
#define uart_driver_dbg(format, ...) \
    do                                    \
    { /* Do nothing */                    \
    } while (0)
#endif


/* Handle driver type used for host communication*/
UART_HandleTypeDef huart2;

typedef struct
{
    struct
    {
        uint8_t buffer[RX_DATA_BUFF_SIZE]; /* Received Data over UART are stored in this buffer */
        c_buff_handle_t cb;                /* pointer typedef to circular buffer struct */
        uint8_t byte;                      /* used to active RX reception interrupt mode */ 
    } rx;

    struct
    {
        uint8_t buffer[TX_DATA_BUFF_SIZE]; /* Data to be transmitted via UART are stored in this buffer */
        c_buff_handle_t cb;                /* pointer typedef to circular buffer struct */
    } tx;


}uart_data_t;

/**/
static uart_data_t uart_data; 

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief Init host comm peripheral interface
 * 
 * @param rx_buff buffer in stack reserved for data reception 
 * @param tx_buff buffer in stack reserved for data transmission
 * @return uint8_t 
 */
uint8_t uart_init(void)
{
    /*Init Uart device*/
    MX_USART2_UART_Init();

    /*Init Circular Buffer*/
    uart_data.tx.cb = circular_buff_init(uart_data.tx.buffer, TX_DATA_BUFF_SIZE);
    uart_data.rx.cb = circular_buff_init(uart_data.rx.buffer, RX_DATA_BUFF_SIZE);

    /*Start Reception of data*/
    HAL_UART_Receive_IT(&huart2, &uart_data.rx.byte, 1);

    uart_driver_dbg("comm driver info : uart2 initialized\r\n");

    return 1;
}

uint8_t uart_get_rx_data_len(void)
{
    return circular_buff_get_data_len(uart_data.rx.cb);
}


uint8_t uart_read_rx_data(uint8_t *data, uint8_t len)
{
    return circular_buff_read(uart_data.rx.cb, data, len);
}


uint8_t uart_fetch_rx_data(uint8_t *data, uint8_t len)
{
    return circular_buff_fetch(uart_data.rx.cb, data, len);
}


uint8_t uart_clear_rx_data(void)
{
    circular_buff_reset(uart_data.rx.cb);
    return 1;
}

uint8_t uart_transmit(uint8_t *data, uint8_t len)
{
    return HAL_UART_Transmit(&huart2, data, len, HAL_MAX_DELAY);
}

uint8_t uart_transmit_it(uint8_t *data, uint8_t len)
{
    /* Write data to circular buffer */
    if (circular_buff_write(uart_data.tx.cb, data, len) == CIRCULAR_BUFF_OK)
    {
        if (huart2.gState == HAL_UART_STATE_READY)
        {
            uint8_t byte;
            circular_buff_get(uart_data.tx.cb, &byte);
            HAL_UART_Transmit_IT(&huart2, &byte, 1);
        }
        else
        {
            uart_driver_dbg("comm driver warning:\t uart busy\r\n");
        }

        return 1;
    }

    uart_driver_dbg("comm driver error:\t circular buffer cannot write request\r\n");
	return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  { 
    /*check for pendings transfers */
    static uint8_t data_chunk[MAX_DATA_CHUNK_SIZE];
    uint16_t data_len = circular_buff_get_data_len(uart_data.tx.cb);

    if(data_len)
    {
        data_len = (data_len >= MAX_DATA_CHUNK_SIZE ) ? (MAX_DATA_CHUNK_SIZE - 1) : data_len;
        circular_buff_read(uart_data.tx.cb, data_chunk, data_len);
        HAL_UART_Transmit_IT(&huart2, data_chunk, data_len);
    }

    uart_driver_dbg("comm driver info:\t irq uart tx complete\r\n");
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        /*Set Uart Data reception for next byte*/
        HAL_UART_Receive_IT(&huart2, &uart_data.rx.byte, 1);

        if(circular_buff_write(uart_data.rx.cb, &uart_data.rx.byte, 1) !=  CIRCULAR_BUFF_OK)
        {
            /*Reinit ring buffer*/
            circular_buff_reset(uart_data.rx.cb);
        }
    }
}

/* only for dbg*/
uint8_t uart_write_rx_data(uint8_t *data, uint8_t len)
{
	circular_buff_st_t status = circular_buff_write(uart_data.rx.cb, data, len);
	if(status != CIRCULAR_BUFF_OK)
	{
	    uart_driver_dbg("comm driver error:\t circular buffer cannot write request\r\n");
	}
    return status;
}



