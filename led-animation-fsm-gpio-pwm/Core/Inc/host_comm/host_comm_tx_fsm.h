/**
 * @file host_comm_tx_fsm.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief This state machine is in charge of the tx communication between Host and test Jig using protocol.h
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef HOST_COMM_TX_FSM
#define HOST_COMM_TX_FSM

#include "uart_driver.h"
#include "time_event.h"
#include "protocol.h"
#include "host_comm_tx_queue.h"

#define MAX_NUM_OF_TRANSFER_RETRIES (2)
#define MAX_ACK_TIMEOUT_MS          (50)
#define DBG_MSG_BUFF_SIZE           (200)


/**
 * @brief Enumeration list of states for tx comm state machine
 * 
 */
typedef enum
{
    st_comm_tx_invalid,
    st_comm_tx_poll_pending_transfer,
    st_comm_tx_transmit_packet,
    st_comm_tx_last
}host_comm_tx_states_t;

/**
 * @brief Enumeration list for internal events 
 * 
 */
typedef enum 
{
    ev_int_comm_tx_invalid,
    ev_int_comm_tx_pending_packet,
    ev_int_comm_tx_no_ack_expected,
    ev_int_comm_tx_last,
}host_comm_tx_internal_events_t;

/**
 * @brief Enumeration list for external events 
 * 
 */
typedef enum 
{
    ev_ext_comm_tx_invalid,
    ev_ext_comm_tx_ack_received,
    ev_ext_comm_tx_nack_received,
    ev_ext_comm_tx_last,
}host_comm_tx_external_events_t;


/**
 * @brief Enumeration list for time related events
 * 
 */
typedef struct 
{
    time_event_t ack_timeout;
}host_comm_tx_time_events_t;


/**
 * @brief Host communication events struct definition
 * 
 */
typedef struct
{
    host_comm_tx_internal_events_t internal;
    host_comm_tx_external_events_t external;
    host_comm_tx_time_events_t     time;
}host_comm_tx_events_t;

/**
 * @brief Control Variables and Functions necessary to perform actions in the state machine
 * 
 */
typedef struct
{
    uint8_t retry_cnt;          /* counter for number of transmission retry */
    tx_request_t request;       /* tx request with the data to be transmitted */
} host_comm_tx_iface_t;

/**
 * @brief Struct definition for host tx communication state machine
 * 
 */
typedef struct
{
	host_comm_tx_states_t    state;	
    host_comm_tx_events_t    event;
    host_comm_tx_iface_t     iface;
} host_comm_tx_fsm_t;

extern host_comm_tx_fsm_t host_comm_tx_handle;

/**@Exported Functions*/
void host_comm_tx_fsm_init(host_comm_tx_fsm_t* handle);
void host_comm_tx_fsm_run(host_comm_tx_fsm_t* handle);
void host_comm_tx_fsm_time_event_update(host_comm_tx_fsm_t *handle);
void host_comm_tx_fsm_set_ext_event(host_comm_tx_fsm_t* handle, host_comm_tx_external_events_t event);

/**@Miscellaneous */
void crc32_accumulate(uint32_t *buff, size_t len, uint32_t *crc_value);
uint8_t host_comm_tx_fsm_write_dbg_msg(host_comm_tx_fsm_t *handle, char *dbg_msg, bool ack_expected);
uint8_t host_comm_tx_fsm_send_packet_no_payload(host_comm_tx_fsm_t *handle, uint8_t type, bool ack_expected);


/**
 * @}
 */

#define host_comm_printf(format, ...)                                       \
    do                                                                      \
    {                                                                       \
        char buff[DBG_MSG_BUFF_SIZE];                                       \
        size_t len = sprintf(buff, format, ##__VA_ARGS__);                  \
        buff[len] = '\0';                                                   \
        host_comm_tx_fsm_write_dbg_msg(&host_comm_tx_handle, buff);         \
    } while (0)

#endif
