#include "tdd.h"

void rx_comm_test_0(void)
{
    /*
    * Identify packet structure and size
    */

    printf("TDD Test #0 -> [packet structure]\r\n");
    printf(" **** Packet frame structure: ***** \r\n");
    printf(" **** Preamble size : %d \r\n", PREAMBLE_SIZE_BYTES);
    printf(" **** Header size : %d \r\n", HEADER_SIZE_BYTES);
    printf(" **** Max Payload size : %d \r\n", MAX_PAYLOAD_SIZE);
    printf(" **** CRC size : %d \r\n", CRC_SIZE_BYTES);
    printf(" **** Postamble size : %d \r\n", POSTAMBLE_SIZE_BYTES);
    printf(" ********************************** \r\n");

    /* Print packet size */
    uint16_t frame_size_1 = PREAMBLE_SIZE_BYTES + HEADER_SIZE_BYTES +
                            MAX_PAYLOAD_SIZE + CRC_SIZE_BYTES + POSTAMBLE_SIZE_BYTES;

    uint16_t frame_size_2 = PREAMBLE_SIZE_BYTES + sizeof(packet_data_t) + CRC_SIZE_BYTES + POSTAMBLE_SIZE_BYTES;

    printf("packet frame size [sum of its components ] -> [%d]\r\n", frame_size_1);
    printf("packet frame size [struct ] -> [%d]\r\n", frame_size_2);

    /*Simulate packet struct in host*/
    uint32_t preamble_src = PREAMBLE;
    uint32_t postamble_src = POSTAMBLE;
    uint32_t crc_src = 0xAABBCCDD;

    packet_data_t host_cmd =
        {
            .header.dir = HOST_TO_TARGET,
            .header.payload_len = 5,
            .payload = "demo0",
            .header.type.cmd = HOST_TO_TARGET_CMD_GET_FW_VERSION,
        };

    /*create temporal buffer and store packet */
    uint8_t frame[100];
    uint8_t frame_idx = 0;
    memcpy(frame, (uint8_t *)&preamble_src, PREAMBLE_SIZE_BYTES);
    frame_idx = PREAMBLE_SIZE_BYTES;
    memcpy(frame + frame_idx, (uint8_t *)&host_cmd.header, HEADER_SIZE_BYTES);
    frame_idx += HEADER_SIZE_BYTES;
    memcpy(frame + frame_idx, (uint8_t *)&host_cmd.payload, host_cmd.header.payload_len);
    frame_idx += host_cmd.header.payload_len;
    memcpy(frame + frame_idx, (uint8_t *)&crc_src, CRC_SIZE_BYTES);
    frame_idx += CRC_SIZE_BYTES;
    memcpy(frame + frame_idx, (uint8_t *)&postamble_src, POSTAMBLE_SIZE_BYTES);
    frame_idx += POSTAMBLE_SIZE_BYTES;

    printf("frame : len [%d]\t ", frame_idx);
    print_buff_hex(frame, frame_idx);

    uint16_t host_cmd_size = PREAMBLE_SIZE_BYTES + HEADER_SIZE_BYTES +
                             host_cmd.header.payload_len + POSTAMBLE_SIZE_BYTES + CRC_SIZE_BYTES;

    printf("host cmd frame size [struct ] -> [%d]\r\n", host_cmd_size);

    printf(" ********************************** \r\n");
    printf(" **** Printing Rx packet [host side]: ***** \r\n");
    printf("preamble:\t 0x%.8X\r\n", preamble_src);
    printf(" **** Header:\t ");
    print_buff_hex((uint8_t *)&host_cmd.header, HEADER_SIZE_BYTES);
    printf(" **** Payload len:\t %d \r\n", host_cmd.header.payload_len);
    print_buff_hex((uint8_t *)&host_cmd.payload.buffer, host_cmd.header.payload_len);
    printf("postamble:\t 0x%.8X\r\n", postamble_src);
    printf("crc:\t 0x%.8X\r\n", crc_src);
    printf(" ********************************** \r\n");

    /*Copy packet format to secondary packet [emulate copying data into rx struct]*/
    uint32_t preamble_des;
    uint32_t postamble_des;
    uint32_t crc_des;

    packet_data_t host_req;
    frame_idx = 0;

    memcpy((uint8_t *)&preamble_des, frame, PREAMBLE_SIZE_BYTES);
    frame_idx += PREAMBLE_SIZE_BYTES;
    memcpy((uint8_t *)&host_req.header, frame + frame_idx, HEADER_SIZE_BYTES);
    frame_idx += HEADER_SIZE_BYTES;
    memcpy((uint8_t *)&host_req.payload, frame + frame_idx, host_req.header.payload_len);
    frame_idx += host_req.header.payload_len;
    memcpy((uint8_t *)&crc_des, frame + frame_idx, POSTAMBLE_SIZE_BYTES);
    frame_idx += POSTAMBLE_SIZE_BYTES;
    memcpy((uint8_t *)&postamble_des, frame + frame_idx, CRC_SIZE_BYTES);

    /*print every component */
    printf(" **** Printing Rx packet [Target side]: ***** \r\n");
    printf("preamble:\t 0x%.8X\r\n", preamble_src);
    printf(" **** Header:\t ");
    print_buff_hex((uint8_t *)&host_req.header, HEADER_SIZE_BYTES);
    printf(" **** Payload len:\t %d \r\n", host_req.header.payload_len);
    print_buff_hex((uint8_t *)&host_req.payload.buffer, host_req.header.payload_len);
    printf("postamble:\t 0x%.8X\r\n", postamble_des);
    printf("crc:\t 0x%.8X\r\n", crc_des);
    printf(" ********************************** \r\n");
}

void rx_comm_test_1(void)
{
    /*
    * Simulate packet reception in the rx fsm by writing a packet in the uart rx queue 
    */

       /**
     * @brief packet
         packet_data_t host_cmd =
        {
            .header.dir = HOST_TO_TARGET,
            .header.payload_len = 5,
            .payload = "demo0",
            .header.type.cmd = HOST_TO_TARGET_CMD_GET_FW_VERSION,
        };
     */


   uint8_t buff3[] = {0x55, 0xAA, 0x55, 0xAA, 0x03, TARGET_TO_HOST_DIR, 0x05, 0x00, 0x64, 0x65, 0x6D,
                     0x6F, 0x30, 0x57, 0xDE, 0x68, 0x6F, 0x55, 0xBB, 0x55, 0xBB};
   uart_write_rx_data(buff3, 21); /*<! Header error expected */

   uint8_t buff1[] = {0x55, 0xAA, 0x55, 0xAA, 0x03, HOST_TO_TARGET_DIR, 0x05, 0x00, 0x64, 0x65, 0x6D,
                     0x6F, 0x30, 0xDD, 0xCC, 0xBB, 0xAA, 0x55, 0xBB, 0x55, 0xBB};

   uart_write_rx_data(buff1, 21); /*<! CRC error expected */

  uint8_t buff2[] = {0x55, 0xAA, 0x55, 0xAA, 0x03, HOST_TO_TARGET_DIR, 0x05, 0x00, 0x64, 0x65, 0x6D,
                    0x6F, 0x30, 0x57, 0xDE, 0x68, 0x6F, 0x55, 0xBB, 0x55, 0xBB};

  uart_write_rx_data(buff2, 21); /*<! Packet ready expected */

  
  uint8_t buff4[] = {0x32, 0xAA, 0x12, 0x54, 0x23, 0xFF, 0x01, 0x30, 0x64, 0x65, 0x6D,
                    0x6F, 0x30, 0x57, 0xDE, 0x15, 0x6F, 0x55, 0x20, 0x55, 0xBB};

  uart_write_rx_data(buff4, 21); /*<! Noise, Nothing expected*/

}

void tx_comm_test_0(void)
{
    /*Send Debug message with ACK response expected */
    host_comm_tx_fsm_write_dbg_msg(&host_comm_tx_handle, "This is a debug message #1, ACK expected\r\n", true);

    /*Send Debug message with no ACK response expected */
    host_comm_tx_fsm_write_dbg_msg(&host_comm_tx_handle, "This is a debug message #2, no ACK expected\r\n", false);

}
