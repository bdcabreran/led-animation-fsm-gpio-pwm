#ifndef HOST_COMM_TX_FSM_H
#define HOST_COMM_TX_FSM_H

#include <stdbool.h>
#include <stdint.h>
#include "time_event.h"
#include "host_comm_tx_fsm.h"
#include <string.h>

#define HEADER_BYTES_TIMEOUT_MS     (30)
#define POSTAMBLE_BYTES_TIMEOUT_MS  (5)

/*
 * Enum of states names in the statechart.
 */
typedef enum
{
    st_comm_rx_invalid,
    st_comm_rx_preamble_proc,
    st_comm_rx_header_proc,
    st_comm_rx_payload_proc,
    st_comm_rx_crc_and_postamble_proc,
    st_comm_rx_packet_ready,
    st_comm_rx_last,
} host_comm_rx_states_t;

/*
 * Enum of internal event names in the statechart.
 */
typedef enum
{
    ev_int_comm_rx_invalid,
    ev_int_preamble_ok,
    ev_int_header_ok,
    ev_int_header_error,
    ev_int_payload_ok,
    ev_int_crc_and_postamble_ok,
    ev_int_postamble_error,
    ev_int_crc_error,
    ev_int_req_packet_ready,
    ev_int_comm_rx_last,

} host_comm_rx_internal_events_t;

/*
 * Enum of external event names in the statechart.
 */

typedef enum
{
    ev_ext_comm_rx_invalid,
    ev_ext_comm_rx_packet_proccessed,
    ev_ext_comm_rx_last,

}host_comm_rx_external_events_t;

/*
 * Type definition of the data structure for time events
*/
typedef struct 
{
    time_event_t header_timeout;
    time_event_t payload_timeout;
    time_event_t crc_and_postamble_timeout;
}host_comm_rx_time_events_t;


typedef struct
{
    host_comm_rx_internal_events_t internal;
    host_comm_rx_external_events_t external;
    host_comm_rx_time_events_t     time;
}host_comm_rx_event_t;

typedef struct
{
    packet_data_t packet;
}host_comm_rx_iface_t;

/*! 
 * Type definition of the data structure for the communication state machine.
 * This data structure has to be allocated by the client code. 
 */
typedef struct
{
	host_comm_rx_states_t      state;	
    host_comm_rx_event_t       event;
    host_comm_rx_iface_t       iface;
} host_comm_rx_fsm_t;

extern host_comm_rx_fsm_t host_comm_rx_handle;

/**@Exported Functions*/
void host_comm_rx_fsm_init(host_comm_rx_fsm_t* handle);
void host_comm_rx_fsm_run(host_comm_rx_fsm_t* handle);

void host_comm_rx_fsm_time_event_update(host_comm_rx_fsm_t *handle);
bool host_comm_rx_fsm_is_active(const host_comm_rx_fsm_t* handle);
bool host_comm_rx_fsm_is_state_active(const host_comm_rx_fsm_t* handle, host_comm_rx_states_t state);
uint8_t host_comm_rx_fsm_set_ext_event(host_comm_rx_fsm_t* handle, host_comm_rx_external_events_t event);

#endif
