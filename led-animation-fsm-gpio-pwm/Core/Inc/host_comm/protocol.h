/**
 * @file protocol.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief Protocol Scheme for Host-Target communication
 * @version 0.1
 * @date 2021-08-17
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "stdint.h"
#include "stdio.h"

#define MAX_PAYLOAD_SIZE	(256)

/* 1 byte = 256 possible cmd/res/evt */
#define CMD_START   (0x00)
#define CMD_END     (0x55)
#define EVT_START   (0x56)
#define EVT_END     (0xAB)
#define RES_START   (0xAC)
#define RES_END     (0xFF)

/* Host / Target IDs */
#define TARGET_TO_HOST_DIR   (0xAA)
#define HOST_TO_TARGET_DIR   (0xBB)

/* Packet Format Sizes */
#define PREAMBLE_SIZE_BYTES     sizeof(uint32_t)
#define POSTAMBLE_SIZE_BYTES    sizeof(uint32_t)
#define HEADER_SIZE_BYTES       sizeof(packet_header_t)
#define CRC_SIZE_BYTES          sizeof(uint32_t)

/* Packet structure 
    ------------------------------------------------------------------------------
   | PREAMBLE : 2B | HEADER : 4B | PAYLOAD : [0 - 256]B | CRC : 4B | POSTAMBLE :4B| 
    ------------------------------------------------------------------------------
*/

/* Preamble / Postamble bytes */
#define PREAMBLE              (0xAA55AA55)
#define POSTAMBLE             (0xBB55BB55)

typedef union
{
    uint32_t byte; 
    uint8_t  bit[4];
}byte_t;


typedef enum
{
    HOST_TO_TARGET = HOST_TO_TARGET_DIR,
    TARGET_TO_HOST = TARGET_TO_HOST_DIR,
}packet_dir_t;

typedef struct
{
    union
    {
        uint8_t cmd;
        uint8_t res;
        uint8_t evt;
    }type;

    packet_dir_t  dir;
    uint16_t payload_len;

}packet_header_t;

typedef struct
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];

}packet_payload_t;

typedef struct
{
    packet_header_t  header;
    packet_payload_t payload;

}packet_data_t;

/*##################################################################################################*/

/* Host Header Types */
typedef enum
{
    HOST_TO_TARGET_CMD_START = CMD_START,
    HOST_TO_TARGET_CMD_TURN_ON_LED,
    HOST_TO_TARGET_CMD_TURN_OFF_LED,
    HOST_TO_TARGET_CMD_GET_FW_VERSION,
    HOST_TO_TARGET_CMD_END = CMD_END
}host_to_target_cmd_t;
#define IS_HOST_TO_TARGET_CMD(cmd) ((cmd > HOST_TO_TARGET_CMD_START) && (cmd < HOST_TO_TARGET_CMD_END))

typedef enum
{
    HOST_TO_TARGET_EVT_START = EVT_START,
    HOST_TO_TARGET_EVT_END = EVT_END
}host_to_target_evt_t;
#define IS_HOST_TO_TARGET_EVT(evt) ((evt > HOST_TO_TARGET_EVT_START) && (evt < HOST_TO_TARGET_EVT_END))


typedef enum
{
    HOST_TO_TARGET_RES_START = RES_START,
    HOST_TO_TARGET_RES_ACK,
    HOST_TO_TARGET_RES_NACK,
    HOST_TO_TARGET_RES_END = RES_END
}host_to_target_resp_t;
#define IS_HOST_TO_TARGET_RES(res) ((res > HOST_TO_TARGET_RES_START) && (res < HOST_TO_TARGET_RES_END))


/*##################################################################################################*/

/* Target Header Types*/
typedef enum
{
    TARGET_TO_HOST_CMD_START = CMD_START,
    TARGET_TO_HOST_CMD_END = CMD_END
}target_to_host_cmd_t;
#define IS_TARGET_TO_HOST_CMD(cmd) ((cmd > TARGET_TO_HOST_CMD_START) && (cmd < TARGET_TO_HOST_CMD_END))


typedef enum
{
    TARGET_TO_HOST_EVT_START = EVT_START,
    TARGET_TO_HOST_EVT_HANDLER_ERROR,
    TARGET_TO_HOST_EVT_PRINT_DBG_MSG,
    TARGET_TO_HOST_EVT_END = EVT_END
}target_to_host_evt_t;
#define IS_TARGET_TO_HOST_EVT(evt) ((evt > TARGET_TO_HOST_EVT_START) && (evt < TARGET_TO_HOST_EVT_END))


typedef enum
{
    TARGET_TO_HOST_RES_START = RES_START,
    TARGET_TO_HOST_RES_ACK,
    TARGET_TO_HOST_RES_NACK,
    TARGET_TO_HOST_RES_LED_ON,
    TARGET_TO_HOST_RES_LED_OFF,
    TARGET_TO_HOST_RES_FW_VERSION,
    TARGET_TO_HOST_RES_END = RES_END
}target_to_host_resp_t;
#define IS_TARGET_TO_HOST_RES(res) ((res > TARGET_TO_HOST_RES_START) && (res < TARGET_TO_HOST_RES_END))


/*##################################################################################################*/

extern const byte_t protocol_preamble;
extern const byte_t protocol_postamble;

void print_buff_ascii(uint8_t *buff, size_t len);
void print_buff_hex(uint8_t *buff, size_t len);
uint8_t protocol_check_valid_header(packet_data_t *packet);


#endif
