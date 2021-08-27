/**
 * @file host_comm_tx_fsm.c
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief 
 * @version 0.1
 * @date 2020-06-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "host_comm_tx_fsm.h"
#include "string.h"

/*@brief Tx comm finite state machine object */
host_comm_tx_fsm_t host_comm_tx_handle;


/**@brief Enable/Disable debug messages */
#define HOST_TX_FSM_DEBUG 1
#define HOST_TX_FSM_TAG "host tx comm: "

/**@brief uart debug function for server comm operations  */
#if HOST_TX_FSM_DEBUG
#define host_comm_tx_dbg(format, ...) printf(HOST_TX_FSM_TAG format, ##__VA_ARGS__)
#else
#define host_comm_tx_dbg(format, ...) \
    do                                    \
    { /* Do nothing */                    \
    } while (0)
#endif


/*Static functions for state poll pending transfers */
static void enter_seq_poll_pending_transfers(host_comm_tx_fsm_t *handle);
static void exit_action_poll_pending_transfers(host_comm_tx_fsm_t *handle);
static void during_action_poll_pending_transfers(host_comm_tx_fsm_t *handle);
static bool poll_pending_transfers_on_react(host_comm_tx_fsm_t *handle, const bool try_transition);

/*Static functions for state transmit packet*/
static void enter_seq_transmit_packet(host_comm_tx_fsm_t *handle);
static void entry_action_transmit_packet(host_comm_tx_fsm_t *handle);
static void exit_action_transmit_packet(host_comm_tx_fsm_t *handle);
static bool transmit_packet_on_react(host_comm_tx_fsm_t *handle, const bool try_transition);

/*Static methods of the finite state machine*/
static uint8_t tx_send_packet(host_comm_tx_fsm_t *handle);

static void clear_events(host_comm_tx_fsm_t* handle)
{
    handle->event.internal = ev_int_comm_tx_invalid;
    handle->event.external = ev_ext_comm_tx_invalid;
    time_event_stop(&handle->event.time.ack_timeout);
}


static void host_comm_tx_fsm_set_next_state(host_comm_tx_fsm_t *handle, host_comm_tx_states_t next_state)
{
	handle->state = next_state;
    clear_events(handle);
}

void host_comm_tx_fsm_init(host_comm_tx_fsm_t* handle)
{
    /*Init interface*/
    host_comm_tx_queue_init();
    memset((uint8_t*)&handle->iface.request.packet, 0, sizeof(packet_data_t));

    /*Clear events */
    clear_events(handle);

    /*defaut enter sequence */
    enter_seq_poll_pending_transfers(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void enter_seq_poll_pending_transfers(host_comm_tx_fsm_t *handle)
{
	host_comm_tx_dbg("enter seq \t[ poll_pending_transfers ]\n");
	host_comm_tx_fsm_set_next_state(handle, st_comm_tx_poll_pending_transfer);
    handle->iface.retry_cnt = 0;
}


static void during_action_poll_pending_transfers(host_comm_tx_fsm_t *handle)
{
    if(host_comm_tx_queue_get_pending_transfers())
    {
        handle->event.internal = ev_int_comm_tx_pending_packet;
        host_comm_tx_dbg("int event \t[ pending_packet ]\n");
    }
}


static void exit_action_poll_pending_transfers(host_comm_tx_fsm_t *handle)
{
    /*Read packet to transfer */
    host_comm_tx_queue_read_request(&handle->iface.request);
}


static bool poll_pending_transfers_on_react(host_comm_tx_fsm_t *handle, const bool try_transition)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.internal == ev_int_comm_tx_pending_packet)
		{
            /*Exit action */
            exit_action_poll_pending_transfers(handle);
			/*Enter sequence */
			enter_seq_transmit_packet(handle);
		}
		else
			did_transition = false;
	}
	if ((did_transition) == (false))
	{
		/*during action*/
		during_action_poll_pending_transfers(handle);
	}
	return did_transition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void enter_seq_transmit_packet(host_comm_tx_fsm_t *handle)
{
	host_comm_tx_dbg("enter seq \t[ transmit_packet ]\n");
	host_comm_tx_fsm_set_next_state(handle, st_comm_tx_transmit_packet);
    entry_action_transmit_packet(handle);
}

static void entry_action_transmit_packet(host_comm_tx_fsm_t *handle)
{
    if(handle->iface.request.ack_expected == true)
    {
        time_event_start(&handle->event.time.ack_timeout, MAX_ACK_TIMEOUT_MS);
        host_comm_tx_dbg("time event \t[ ack resp time start ]\n");
    }
    else
    {
        handle->event.internal = ev_int_comm_tx_no_ack_expected;
        host_comm_tx_dbg("int event \t[ ack no expected ]\n");
    }
    tx_send_packet(handle);
}

static void exit_action_transmit_packet(host_comm_tx_fsm_t *handle)
{
    time_event_stop(&handle->event.time.ack_timeout);
}


static bool transmit_packet_on_react(host_comm_tx_fsm_t *handle, const bool try_transition)
{
    /* The reactions of state 'check preamble' */
    bool did_transition = try_transition;

    if (try_transition == true)
    {
        if ((handle->event.external == ev_ext_comm_tx_ack_received) |
            (handle->event.internal == ev_int_comm_tx_no_ack_expected))
        {
            exit_action_transmit_packet(handle);
            enter_seq_poll_pending_transfers(handle);
        }

        else if(handle->event.external == ev_ext_comm_tx_nack_received)
        {
            exit_action_transmit_packet(handle);
            enter_seq_transmit_packet(handle);
        }

        else if (time_event_is_raised(&handle->event.time.ack_timeout) == true)
        {
            exit_action_transmit_packet(handle);
            host_comm_tx_dbg("time event \t[ ack resp timeout ]\n");
            host_comm_tx_dbg("tx retry\t : #%d\n",handle->iface.retry_cnt);

            /*Enter sequence */
            if (handle->iface.retry_cnt++ >= MAX_NUM_OF_TRANSFER_RETRIES)
            {
                host_comm_tx_dbg("guard \t[ max tx retries ->%d]\n", MAX_NUM_OF_TRANSFER_RETRIES);
                enter_seq_poll_pending_transfers(handle);
            }
            else
                enter_seq_transmit_packet(handle);
        }

        else
            did_transition = false;
    }
    if ((did_transition) == (false))
    {
        
    }
    return did_transition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void crc32_accumulate(uint32_t *buff, size_t len, uint32_t *crc_value)
{
    int i = 0;
    uint32_t crc = *crc_value ^ buff[i];
    for (i = 0; i < len - 1; i++)
        crc ^= buff[i + 1];

    *crc_value = crc;
}

/**
 * @brief 
 * 
 * @param host_comm 
 * @param packet 
 * @return uint8_t 
 */
static uint8_t tx_send_packet(host_comm_tx_fsm_t *handle)
{
   /* packet index to write bytes  */
    uint32_t crc = 0;

    packet_data_t *packet = &handle->iface.request.packet;

    /* Transmit preamble */
    if (!uart_transmit_it((uint8_t *)&protocol_preamble.bit, PREAMBLE_SIZE_BYTES))
        return 0;

    /* Start CRC calculation*/
    crc32_accumulate((uint8_t *)&packet->header, HEADER_SIZE_BYTES, &crc);

    /* Transmit Header */
    if (!uart_transmit_it((uint8_t *)&packet->header, HEADER_SIZE_BYTES))
        return 0;

    /* If Payload  */
    if (packet->header.payload_len)
    {
        /*update CRC*/
        crc32_accumulate((uint8_t *)&packet->payload, packet->header.payload_len, &crc);

        /*Transmit payload*/
        if (!uart_transmit_it((uint8_t *)&packet->payload, packet->header.payload_len))
            return 0;
    }

    /*Transmit CRC*/
    if (!uart_transmit_it((uint8_t *)&crc, CRC_SIZE_BYTES))
        return 0;

    /*Transmit Postamble*/
    if (!uart_transmit_it((uint8_t *)&protocol_postamble.bit, POSTAMBLE_SIZE_BYTES))
        return 0;

    return 1;
}



uint8_t host_comm_tx_fsm_write_dbg_msg(host_comm_tx_fsm_t *handle, char *dbg_msg, bool ack_expected)
{
	/* Check frame identifier */
	if (dbg_msg != NULL)
	{
		/*form header*/
		tx_request_t request;
        request.ack_expected = ack_expected;
        request.src = TX_SRC_FW_USER;
		request.packet.header.dir = TARGET_TO_HOST_DIR;
		request.packet.header.type.evt = TARGET_TO_HOST_EVT_PRINT_DBG_MSG;
		request.packet.header.payload_len = strlen(dbg_msg);

		/*copy dbg message to payload*/
        if((request.packet.header.payload_len > 0) && (request.packet.header.payload_len < MAX_PAYLOAD_SIZE))
		    memcpy((uint8_t*)&request.packet.payload, dbg_msg, request.packet.header.payload_len);
        else
            return 0;

		/*Write Data*/
        //print_buff_hex((uint8_t*)&request.packet, HEADER_SIZE_BYTES + request.packet.header.payload_len);
        return host_comm_tx_queue_write_request(&request);
	}

	return 0;
}

uint8_t host_comm_tx_fsm_send_packet_no_payload(host_comm_tx_fsm_t *handle, uint8_t type, bool ack_expected)
{
    /*form header*/
    tx_request_t request = 
    {
        .ack_expected = ack_expected,
        .src = TX_SRC_RX_FSM,
        .packet.header.dir = TARGET_TO_HOST_DIR,
        .packet.header.type.res = type,
        .packet.header.payload_len = 0,
    };

    /*Write Data*/
    return host_comm_tx_queue_write_request(&request);
}

void host_comm_tx_fsm_time_event_update(host_comm_tx_fsm_t *handle)
{
	time_event_t *time_event = (time_event_t *)&handle->event.time;
	for (int tev_idx = 0; tev_idx < sizeof(handle->event.time) / sizeof(time_event_t); tev_idx++)
	{
		time_event_update(time_event);
		time_event++;
	}
}

void host_comm_tx_fsm_run(host_comm_tx_fsm_t *handle)
{
    switch (handle->state)
    {
    case st_comm_tx_poll_pending_transfer: poll_pending_transfers_on_react(handle, true); break;
    case st_comm_tx_transmit_packet:       transmit_packet_on_react(handle, true);         break;
    default: break;
    }
}

void host_comm_tx_fsm_set_ext_event(host_comm_tx_fsm_t* handle, host_comm_tx_external_events_t event)
{
    handle->event.external = event;

    /*debug*/
    if(event == ev_ext_comm_tx_ack_received)
        host_comm_tx_dbg("ext event\t [ack received ]\r\n");
    else if (event == ev_ext_comm_tx_nack_received)
        host_comm_tx_dbg("ext event\t [nack received]\r\n");
}

