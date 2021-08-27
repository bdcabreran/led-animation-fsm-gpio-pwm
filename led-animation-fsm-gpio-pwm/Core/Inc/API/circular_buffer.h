/**
 * @file circular_buffer.h
 */

#ifndef _CIRCULAR_BUFFER_H
#define _CIRCULAR_BUFFER_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stdint.h"
#include "assert.h"
#include "stdlib.h"

/**
 * @brief list enumeration for circular buffer state
 * @enum  circular_buff_st_t
 */
typedef enum
{
	CIRCULAR_BUFF_OK = 0x00,
	CIRCULAR_BUFF_FULL,
	CIRCUILAR_BUFF_NOT_ENOUGH_SPACE,

}circular_buff_st_t;

/**@defgroup Server_Communication_Exported_Types
 * @{
 */

/*@brief typedef definition for circular buffer struct  */
typedef struct circular_buff_t circular_buff_t;

/*@brief pointer typedef to circular buffer struct */
typedef circular_buff_t* c_buff_handle_t;

/**@} */


/**
 * @defgroup Circular_Buffer_Exported_Functions Circular Buffer Exported Functions 
 * @{
 */

/** Check if c_buffer is empty  */
uint8_t circular_buff_empty(c_buff_handle_t c_buff);

/** Check if c_buff is full */
uint8_t circular_buff_full(c_buff_handle_t c_buff);

/** Get an instance of circular buffer and initialize it */
c_buff_handle_t circular_buff_init(uint8_t *buffer, size_t size);

/** Deallocate specified c_buffer */
void circular_buff_free(c_buff_handle_t c_buff);

/** Reset c_buffer to default values */
void circular_buff_reset(c_buff_handle_t c_buff);

/** Get amount of bytes available to be written in c_buffer */
size_t circular_buff_get_free_space(c_buff_handle_t c_buff);

/** Get the capacity for an specified c_buff */
size_t circular_buff_capacity(c_buff_handle_t c_buff);

/** get amount of data available to be read in c_buff */
size_t circular_buff_get_data_len(c_buff_handle_t c_buff);

/** write byte in circular buffer */
void circular_buff_put(c_buff_handle_t c_buff, uint8_t data);

/** Read byte in circular buffer  */
uint8_t circular_buff_get(c_buff_handle_t c_buff, uint8_t *data);

/** Write amount of data in c_buff */
circular_buff_st_t circular_buff_write(c_buff_handle_t c_buff, uint8_t *data, uint8_t data_len);

/** Read amount of data in c_buff */
uint8_t circular_buff_read(c_buff_handle_t c_buff, uint8_t *data, size_t data_len);

/** Fetch amount of data in c_buff */
uint8_t circular_buff_fetch(c_buff_handle_t c_buff, uint8_t *data, size_t data_len);

/**@} */

#endif