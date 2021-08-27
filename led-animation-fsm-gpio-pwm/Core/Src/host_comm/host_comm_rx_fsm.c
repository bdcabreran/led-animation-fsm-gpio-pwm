#include "host_comm_rx_fsm.h"

/**@brief Enable/Disable debug messages */
#define HOST_RX_DEBUG 0
#define HOST_RX_TAG "host rx comm : "

/**@brief uart debug function for server comm operations  */
#if HOST_RX_DEBUG
#define host_comm_rx_dbg(format, ...) printf(HOST_RX_TAG format, ##__VA_ARGS__)
#else
#define host_comm_rx_dbg(format, ...) \
	do                                       \
	{ /* Do nothing */                       \
	} while (0)
#endif


/**@ Extern handle for server communication state machine */
host_comm_rx_fsm_t host_comm_rx_handle;

/**@ 'Preamble process' state related functions */
static void enter_seq_preamble_proc(host_comm_rx_fsm_t *handle);
static bool preamble_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition);
static uint8_t during_action_preamble_proc(host_comm_rx_fsm_t *handle);

/**@ 'Header Process' state related functions */
static void enter_seq_header_proc(host_comm_rx_fsm_t *handle);
static void entry_action_header_proc(host_comm_rx_fsm_t *handle);
static void exit_action_header_proc(host_comm_rx_fsm_t *handle);
static void during_action_header_proc(host_comm_rx_fsm_t *handle);
static bool header_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition);

/**@ 'Payload Process' state related functions */
static void enter_seq_payload_proc(host_comm_rx_fsm_t *handle);
static void entry_action_payload_proc(host_comm_rx_fsm_t *handle);
static void during_action_payload_proc(host_comm_rx_fsm_t *handle);
static void exit_action_payload_proc(host_comm_rx_fsm_t *handle);
static bool payload_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition);

/**@ 'Check crc_and_postamble' state related functions */
static void enter_seq_crc_and_postamble_proc(host_comm_rx_fsm_t *handle);
static void entry_action_crc_and_postamble_proc(host_comm_rx_fsm_t *handle);
static void during_action_crc_and_postamble_proc(host_comm_rx_fsm_t *handle);
static void exit_action_crc_and_postamble_proc(host_comm_rx_fsm_t *handle);
static bool crc_and_postamble_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition);

/**@ 'gateway request packet' state related functions */
static void enter_seq_packet_ready(host_comm_rx_fsm_t *handle);
static void entry_action_packet_ready(host_comm_rx_fsm_t *handle);
static bool packet_ready_on_react(host_comm_rx_fsm_t *handle, const bool try_transition);

/* Entry action for state machine */
void host_comm_rx_fsm_enter(host_comm_rx_fsm_t *handle)
{
	/* Default react sequence for initial entry  */
	enter_seq_preamble_proc(handle);
}

static void host_comm_rx_fsm_set_next_state(host_comm_rx_fsm_t *handle, host_comm_rx_states_t next_state)
{
	handle->state = next_state;
	handle->event.internal = ev_int_comm_rx_invalid;
	handle->event.external = ev_ext_comm_rx_invalid;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@ 'preamble process' state related functions */

static void enter_seq_preamble_proc(host_comm_rx_fsm_t *handle)
{
	host_comm_rx_dbg("enter seq \t[ check_preamble_st ]\r\n");
	host_comm_rx_fsm_set_next_state(handle, st_comm_rx_preamble_proc);
}

static uint8_t during_action_preamble_proc(host_comm_rx_fsm_t *handle)
{
	if (uart_get_rx_data_len() >= PREAMBLE_SIZE_BYTES)
	{
		uint8_t preamble;
		/* read byte by byte to prevent data lost */
		uart_read_rx_data((uint8_t *)&preamble, 1);
		if (preamble != protocol_preamble.bit[0])
			return 0;
		uart_read_rx_data((uint8_t *)&preamble, 1);
		if (preamble != protocol_preamble.bit[1])
			return 0;
		uart_read_rx_data((uint8_t *)&preamble, 1);
		if (preamble != protocol_preamble.bit[2])
			return 0;
		uart_read_rx_data((uint8_t *)&preamble, 1);
		if (preamble != protocol_preamble.bit[3])
			return 0;

		host_comm_rx_dbg("ev_internal \t[ preamble_ok ]\r\n");
		handle->event.internal = ev_int_preamble_ok;
		return 1;
	}
	return 0;
}

static bool preamble_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.internal == ev_int_preamble_ok)
		{
			/*Enter sequence */
			enter_seq_header_proc(handle);
		}
		else
			did_transition = false;
	}
	if ((did_transition) == (false))
	{
		/*during action*/
		during_action_preamble_proc(handle);
	}
	
	return did_transition;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*'default enter sequence for state 'header process '*/
static void enter_seq_header_proc(host_comm_rx_fsm_t *handle)
{
	host_comm_rx_dbg("enter seq \t[ check_header_st ]\r\n");
	host_comm_rx_fsm_set_next_state(handle, st_comm_rx_header_proc);
	entry_action_header_proc(handle);
}

static void entry_action_header_proc(host_comm_rx_fsm_t *handle)
{
	time_event_start(&handle->event.time.header_timeout, (HEADER_BYTES_TIMEOUT_MS));
}

static void exit_action_header_proc(host_comm_rx_fsm_t *handle)
{
	/*Stop time out events*/
	time_event_stop(&handle->event.time.header_timeout);
}

static void during_action_header_proc(host_comm_rx_fsm_t *handle)
{
	if (uart_get_rx_data_len() >= HEADER_SIZE_BYTES)
	{
		/* 1. Read Header from server buffer */
		uart_read_rx_data((uint8_t *)&handle->iface.packet.header, HEADER_SIZE_BYTES);

		if (protocol_check_valid_header(&handle->iface.packet))
		{
			host_comm_rx_dbg("ev_internal \t[ header_ok ]\r\n");
			handle->event.internal = ev_int_header_ok;
		}
		else
		{
			host_comm_rx_dbg("ev_internal \t[ header_error ]\r\n");
			handle->event.internal = ev_int_header_error;
		}
	}
}

static bool header_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.internal == ev_int_header_ok)
		{
			/*Exit Action */
			exit_action_header_proc(handle);

			/*Choice Enter sequence */
			if (handle->iface.packet.header.payload_len > 0)
			{
				host_comm_rx_dbg("guard \t[ payload len > 0 ]\r\n");
				enter_seq_payload_proc(handle);
			}
			else
			{
				host_comm_rx_dbg("guard \t[ payload = 0 ]\r\n");
				enter_seq_crc_and_postamble_proc(handle);
			}
		}

		else if (time_event_is_raised(&handle->event.time.header_timeout) == true ||
				 handle->event.internal == ev_int_header_error)
		{

			/*Transition Action*/
			if (time_event_is_raised(&handle->event.time.header_timeout) == true)
			{
				uart_clear_rx_data();
				host_comm_rx_dbg("ev_internal \t[ header timeout ]\r\n");
			}

			host_comm_tx_fsm_send_packet_no_payload(&host_comm_tx_handle, TARGET_TO_HOST_RES_NACK, false);

			/*Exit Action*/
			exit_action_header_proc(handle);

			/*Enter Sequence*/
			enter_seq_preamble_proc(handle);
		}
		else
		{
			did_transition = false;
		}
	}
	if ((did_transition) == (false))
	{
		/*during action*/
		during_action_header_proc(handle);
	}
	return did_transition;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@ 'Check payload' state related functions */

/*'default enter sequence for state 'check payload'*/
static void enter_seq_payload_proc(host_comm_rx_fsm_t *handle)
{
	/*Entry Action*/
	host_comm_rx_dbg("enter seq \t[ check_payload_st ]\r\n");
	host_comm_rx_fsm_set_next_state(handle, st_comm_rx_payload_proc);
	entry_action_payload_proc(handle);
}

static void entry_action_payload_proc(host_comm_rx_fsm_t *handle)
{
	uint16_t time_ms = handle->iface.packet.header.payload_len * 1;
	time_event_start(&handle->event.time.payload_timeout, time_ms);
}

static void exit_action_payload_proc(host_comm_rx_fsm_t *handle)
{
	time_event_stop(&handle->event.time.payload_timeout);
}

static void during_action_payload_proc(host_comm_rx_fsm_t *handle)
{
	uint8_t exp_data_len = handle->iface.packet.header.payload_len;

	if (uart_get_rx_data_len() >= exp_data_len)
	{
		host_comm_rx_dbg("ev_internal \t[ payload_ok ]\r\n");
		handle->event.internal = ev_int_payload_ok;
		uart_read_rx_data((uint8_t *)&handle->iface.packet.payload,
								   handle->iface.packet.header.payload_len);
	}
}

static bool payload_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.internal == ev_int_payload_ok)
		{
			/*Exit Action */
			exit_action_payload_proc(handle);

			/*Enter sequence */
			enter_seq_crc_and_postamble_proc(handle);
		}

		else if (time_event_is_raised(&handle->event.time.payload_timeout) == true)
		{
			/*Exit Action*/
			exit_action_payload_proc(handle);

			/*Transition Action*/
			host_comm_rx_dbg("ev_internal \t[ timeout payload ] \r\n");
			uart_clear_rx_data();
			host_comm_tx_fsm_send_packet_no_payload(&host_comm_tx_handle, TARGET_TO_HOST_RES_NACK, false);

			/*Enter Sequence*/
			enter_seq_preamble_proc(handle);
		}
		else
		{
			did_transition = false;
		}
	}
	if ((did_transition) == (false))
	{
		/*during action*/
		during_action_payload_proc(handle);
	}
	return did_transition;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@ 'Check crc and postamble' state related functions */
static void enter_seq_crc_and_postamble_proc(host_comm_rx_fsm_t *handle)
{
	/*Entry Action*/
	host_comm_rx_dbg("enter seq \t[ check_crc_and_postamble_st ]\r\n");
	host_comm_rx_fsm_set_next_state(handle, st_comm_rx_crc_and_postamble_proc);
	entry_action_crc_and_postamble_proc(handle);
}

static void entry_action_crc_and_postamble_proc(host_comm_rx_fsm_t *handle)
{
	time_event_start(&handle->event.time.crc_and_postamble_timeout, POSTAMBLE_BYTES_TIMEOUT_MS);
}

static void exit_action_crc_and_postamble_proc(host_comm_rx_fsm_t *handle)
{
	time_event_stop(&handle->event.time.crc_and_postamble_timeout);
}

static void during_action_crc_and_postamble_proc(host_comm_rx_fsm_t *handle)
{
	uint8_t exp_data_len = CRC_SIZE_BYTES + POSTAMBLE_SIZE_BYTES;

	if (uart_get_rx_data_len() >= exp_data_len)
	{
		uint32_t recv_crc;
		uint32_t postamble;

		uart_read_rx_data((uint8_t*)&recv_crc, CRC_SIZE_BYTES);
		uart_read_rx_data((uint8_t*)&postamble, POSTAMBLE_SIZE_BYTES);

		size_t packet_len = HEADER_SIZE_BYTES + handle->iface.packet.header.payload_len;
		uint32_t crc = 0;

		crc32_accumulate((uint8_t *)&handle->iface.packet.header, packet_len, &crc);

		if (crc != recv_crc)
		{
			host_comm_rx_dbg("ev_internal \t[ crc error ]\r\n");
			host_comm_rx_dbg("expected crc \t[0x%.8X] != recv [0x%.8X]\r\n", crc, recv_crc);
			handle->event.internal = ev_int_crc_error;
		}
		else
		{
			if (postamble != POSTAMBLE)
			{
				host_comm_rx_dbg("ev_internal \t[ postamble error ] \r\n");
				handle->event.internal = ev_int_postamble_error;
			}
			host_comm_rx_dbg("ev_internal \t[ crc and postamble ok ]\r\n");
			handle->event.internal = ev_int_crc_and_postamble_ok;
		}
	}
}

static bool crc_and_postamble_proc_on_react(host_comm_rx_fsm_t *handle, const bool try_transition)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.internal == ev_int_crc_and_postamble_ok)
		{
			/*Exit Action */
			exit_action_crc_and_postamble_proc(handle);

			/*Transition Action*/
			host_comm_tx_fsm_send_packet_no_payload(&host_comm_tx_handle, TARGET_TO_HOST_RES_ACK, false);

			/*Enter sequence */
			enter_seq_packet_ready(handle);
		}

		else if (time_event_is_raised(&handle->event.time.crc_and_postamble_timeout) == true ||
				 handle->event.internal == ev_int_crc_error || handle->event.internal == ev_int_postamble_error)
		{

			/*Exit Action*/
			exit_action_crc_and_postamble_proc(handle);

			/*Transition Action*/
			if (time_event_is_raised(&handle->event.time.crc_and_postamble_timeout) == true)
			{
				host_comm_rx_dbg("ev_internal \t[ timeout crc and postamble] \r\n");
				uart_clear_rx_data();
			}

			host_comm_tx_fsm_send_packet_no_payload(&host_comm_tx_handle, TARGET_TO_HOST_RES_NACK, false);

			/*Enter Sequence*/
			enter_seq_preamble_proc(handle);
		}
		else
		{
			did_transition = false;
		}
	}
	if ((did_transition) == (false))
	{
		/*during action*/
		during_action_crc_and_postamble_proc(handle);
	}
	return did_transition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void enter_seq_packet_ready(host_comm_rx_fsm_t *handle)
{
	/*Entry Action*/
	host_comm_rx_dbg("enter seq \t[ packet ready ]\r\n");
	host_comm_rx_fsm_set_next_state(handle, st_comm_rx_packet_ready);
	entry_action_packet_ready(handle);
}

static void entry_action_packet_ready(host_comm_rx_fsm_t *handle)
{
	/*Notify or enqueue data for other fsm process*/

}

static bool packet_ready_on_react(host_comm_rx_fsm_t *handle, const bool try_transition)
{
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.external == ev_ext_comm_rx_packet_proccessed)
			enter_seq_preamble_proc(handle);

		else
			did_transition = false;
	}

	return did_transition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void clear_time_events(host_comm_rx_fsm_t *handle)
{
	/*reset raised flags*/
	time_event_stop(&handle->event.time.crc_and_postamble_timeout);
	time_event_stop(&handle->event.time.header_timeout);
	time_event_stop(&handle->event.time.payload_timeout);
}

void host_comm_rx_fsm_init(host_comm_rx_fsm_t *handle)
{
	/*Init Interface*/
	memset((uint8_t *)&handle->iface.packet, 0, sizeof(packet_data_t));

	/*Clear events*/
	clear_time_events(handle);

	/*Default Enter Sequence*/
	host_comm_rx_fsm_enter(handle);
}


bool host_comm_rx_fsm_is_active(const host_comm_rx_fsm_t *handle)
{
	bool result = (handle->state > st_comm_rx_invalid && handle->state < st_comm_rx_last)? true : false;
	return result;
}

void host_comm_rx_fsm_time_event_update(host_comm_rx_fsm_t *handle)
{
	time_event_t *time_event = (time_event_t *)&handle->event.time;
	for (int tev_idx = 0; tev_idx < sizeof(handle->event.time) / sizeof(time_event_t); tev_idx++)
	{
		time_event_update(time_event);
		time_event++;
	}
}

void host_comm_rx_fsm_run(host_comm_rx_fsm_t *handle)
{
	switch (handle->state)
	{
	case st_comm_rx_preamble_proc:          preamble_proc_on_react(handle, true);          break;
	case st_comm_rx_header_proc:            header_proc_on_react(handle, true);            break;
	case st_comm_rx_payload_proc:           payload_proc_on_react(handle, true);           break;
	case st_comm_rx_crc_and_postamble_proc: crc_and_postamble_proc_on_react(handle, true); break;
	case st_comm_rx_packet_ready:           packet_ready_on_react(handle, true);           break;

	default:
		break;
	}
}

bool host_comm_rx_fsm_is_state_active(const host_comm_rx_fsm_t *handle, host_comm_rx_states_t state)
{
	bool result = (handle->state == state) ? true : false;
	return result;
}

uint8_t host_comm_rx_fsm_set_ext_event(host_comm_rx_fsm_t *handle, host_comm_rx_external_events_t event)
{
	handle->event.external = event;
	return 1;
}
