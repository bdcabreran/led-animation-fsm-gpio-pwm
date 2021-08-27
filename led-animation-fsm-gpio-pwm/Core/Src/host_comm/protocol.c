#include "protocol.h"

const byte_t protocol_preamble = {.byte = PREAMBLE};
const byte_t protocol_postamble = {.byte = POSTAMBLE};

void print_buff_hex(uint8_t *buff, size_t len)
{
    printf("buffer hex format : [ ");
    for (size_t i = 0; i < len; i++)
    {
        printf(" 0x%.2X", buff[i]);
    }
    printf(" ]\r\n");
}

void print_buff_ascii(uint8_t *buff, size_t len)
{
    printf("buffer ascii format : [ ");
    for (size_t i = 0; i < len; i++)
    {
        printf(" %c", buff[i]);
    }
    printf("\r\n");
}

    union
    {
        uint8_t cmd;
        uint8_t res;
        uint8_t evt;
    }type;

    packet_dir_t  dir;
    uint16_t payload_len;

uint8_t protocol_check_valid_header(packet_data_t *packet)
{
    if (IS_HOST_TO_TARGET_CMD(packet->header.type.cmd) ||
        IS_HOST_TO_TARGET_EVT(packet->header.type.evt) ||
        IS_HOST_TO_TARGET_RES(packet->header.type.res))
    {
        /*check payload len */
        if(packet->header.payload_len < MAX_PAYLOAD_SIZE)
        {
            if(packet->header.dir == HOST_TO_TARGET_DIR)
            {
                return 1;
            }
        }
    }

    return 0;
}
