/**
 * @file circular_buffer.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief  	Circular buffer implementation
 * @version 0.1
 * @date 2019-07-30
 */

#include "circular_buffer.h"

/**
 * @brief  Circular buffer data struct
 * @note   The definition of our circular buffer structure is hidden from the user
 * @struct circular_buff_t
 * 
 */
struct circular_buff_t
{
    uint8_t *buffer;
    size_t head;
    size_t tail;
    size_t length;
    uint8_t full;
};

/**
 * @defgroup Circular_Buffer_Private_Functions
 * @{
 */

/**
 * @brief Advance head pointer by 1 position
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 */
static void head_ptr_advance(c_buff_handle_t c_buff)
{
    assert(c_buff);

    if (c_buff->full)
    {
        c_buff->tail = (c_buff->tail + 1) % c_buff->length;
    }

    // We mark full because we will advance tail on the next time around
    c_buff->head = (c_buff->head + 1) % c_buff->length;
    c_buff->full = (c_buff->head == c_buff->tail);
}

/**
 * @brief Retreat tail pointer by 1 position
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 */
static void tail_ptr_retreat(c_buff_handle_t c_buff)
{
    assert(c_buff);

    c_buff->full = 0;
    c_buff->tail = (c_buff->tail + 1) % c_buff->length;
}

/**@} */

/**
 * @defgroup Circular_Buffer_Public_Functions
 * @{
 */

/**
 * @brief Check if circular buffer is empty or not
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @return uint8_t return 1 if circular buffer is empty, return 0 otherwise.
 */
uint8_t circular_buff_empty(c_buff_handle_t c_buff)
{
    assert(c_buff);

    return (!c_buff->full && (c_buff->tail == c_buff->head));
}

/**
 * @brief Check if circular buffer is full or not
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @return uint8_t returns 1 if circular buffer is full, return 0 otherwise.
 */
uint8_t circular_buff_full(c_buff_handle_t c_buff)
{
    assert(c_buff);

    return c_buff->full;
}

/**
 * @brief Initialize Circular buffer and get the handler associated.
 * 
 * @param buffer  pointer to a buffer reserved in memory by the user that is going to be register in circular buffer
 * @param size    size of the buffer to be register.
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the initialized circular buffer.
 */
c_buff_handle_t circular_buff_init(uint8_t *buffer, size_t size)
{
    assert(buffer && size);

    c_buff_handle_t c_buff = malloc(sizeof(circular_buff_t));
    assert(c_buff);

    c_buff->buffer = buffer;
    c_buff->length = size;
    circular_buff_reset(c_buff);

    assert(circular_buff_empty(c_buff));

    return c_buff;
}

/**
 * @brief Deallocate an specified circular buffer handler 
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 */
void circular_buff_free(c_buff_handle_t c_buff)
{
    assert(c_buff);
    free(c_buff);
}

/**
 * @brief Reset Circular buffer to default configuration
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 */
void circular_buff_reset(c_buff_handle_t c_buff)
{
    assert(c_buff);
    c_buff->head = 0;
    c_buff->tail = 0;
    c_buff->full = 0;
}

/**
 * @brief Return the data available in circular buffer
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @return size_t return number of bytes in buffer.
 */
size_t circular_buff_get_data_len(c_buff_handle_t c_buff)
{
    assert(c_buff);

    size_t size = c_buff->length;

    if (!c_buff->full)
    {
        if (c_buff->head >= c_buff->tail)
        {
            size = (c_buff->head - c_buff->tail);
        }
        else
        {
            size = (c_buff->length + c_buff->head - c_buff->tail);
        }
    }

    return size;
}

/**
 * @brief Return the capacity of the circular buffer 
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @return size_t return length of the buffer registered in circular buffer 
 */
size_t circular_buff_capacity(c_buff_handle_t c_buff)
{
    assert(c_buff);
    return c_buff->length;
}

/**
 * @brief Return the free space available in circular buffer 
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @return size_t return the number of bytes available in circular buffer 
 */
size_t circular_buff_get_free_space(c_buff_handle_t c_buff)
{
    assert(c_buff);
    return (c_buff->length - circular_buff_get_data_len(c_buff));
}

/**
 * @brief Put byte in circular buffer
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @param data byte to be written in buffer.
 */
void circular_buff_put(c_buff_handle_t c_buff, uint8_t data)
{
    assert(c_buff && c_buff->buffer);

    c_buff->buffer[c_buff->head] = data;

    head_ptr_advance(c_buff);
}

/**
 * @brief Get byte from circular buffer 
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @param data   pointer to a variable to be fill whit the data in buffer.
 * @return uint8_t  return 0 if there is not data available to be read, return 1 otherwise.
 */
uint8_t circular_buff_get(c_buff_handle_t c_buff, uint8_t *data)
{
    assert(c_buff && data && c_buff->buffer);

    int r = 0;

    if (!circular_buff_empty(c_buff))
    {
        *data = c_buff->buffer[c_buff->tail];
        tail_ptr_retreat(c_buff);

        r = 1;
    }

    return r;
}

/**
 * @brief Write data in circular buffer
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @param data   pointer to a buffer that contains the data to be written in buffer
 * @param data_len number of bytes of data to be written in buffer
 * @return circular_buff_st_t  return status of buffer.
 */
circular_buff_st_t circular_buff_write(c_buff_handle_t c_buff, uint8_t *data, uint8_t data_len)
{
    assert(c_buff && c_buff->buffer);

    if (c_buff->full)
    {
        return CIRCULAR_BUFF_FULL;
    }

    if (circular_buff_get_free_space(c_buff) < data_len)
    {
        return CIRCUILAR_BUFF_NOT_ENOUGH_SPACE;
    }
    else
    {
        size_t data_counter = 0;

        while (data_counter < data_len)
        {
            circular_buff_put(c_buff, data[data_counter++]);
        }

        return CIRCULAR_BUFF_OK;
    }
}

/**
 * @brief Read data from circular buffer
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @param data pointer to a buffer to be filled.
 * @param data_len  number of bytes to be read in circular buffer.
 * @return uint8_t  return 1 if number of bytes requested to be read is correct, return 0 otherwise.
 */
uint8_t circular_buff_read(c_buff_handle_t c_buff, uint8_t *data, size_t data_len)
{
    assert(c_buff && c_buff->buffer && data);

    size_t data_counter = 0;

    while (data_counter < data_len)
    {
        if (!circular_buff_get(c_buff, &data[data_counter++]))
        {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Fetch data in ring buffer 
 * 
 * @param c_buff variable of type circular_buff_t* which contains the struct associated to the circular buffer
 * @param data   buffer to be filled with the fetch data in circular buffer.
 * @param data_len number of bytes to be fetch.
 * @return uint8_t  return 1 if number of bytes requested to be fetch is correct, return 0 otherwise.
 */
uint8_t circular_buff_fetch(c_buff_handle_t c_buff, uint8_t *data, size_t data_len)
{
    assert(c_buff && c_buff->buffer && data);

    size_t data_counter = 0;
    size_t tail_idx = c_buff->tail;

    assert(c_buff && c_buff->buffer);

    if (data_len > circular_buff_get_data_len(c_buff))
    {
        return 0;
    }
    else
    {
        while (data_counter < data_len)
        {
            data[data_counter++] = c_buff->buffer[tail_idx++];
            tail_idx = (tail_idx) % c_buff->length;
        }
    }

    return 1;
}

/**@} */
