/**
 * @file time_event.c
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief  time event file for time-driven events in FSM
 * @version 0.1
 * @date 2021-08-18
 */

#include "time_event.h"
#include <stdbool.h>
#include <assert.h>

bool volatile update_time_events_flag = false;

void time_event_start(time_event_t *time_event, const uint32_t time_ms)
{
	assert(time_event);
    time_event->millis_cnt = time_ms;
    time_event->active = true;
    time_event->raised = false;
}   

void time_event_stop(time_event_t *time_event)
{
    assert(time_event);
    time_event->active = false;
    time_event->raised = false;
    time_event->millis_cnt = 0;

}

bool time_event_is_active(time_event_t *time_event)
{
    assert(time_event);
    return time_event->active;
}

bool time_event_update(time_event_t *time_event)
{
    if (time_event->active == true)
    {
        if (time_event->millis_cnt > 0)
            time_event->millis_cnt--;
        else
            time_event->raised = true;
    }

    return true;
}

bool time_event_is_raised(time_event_t *time_event)
{
    return time_event->raised;
}

/*This function is called every ms*/
void time_event_set_update_flag(bool status)
{
    update_time_events_flag = status;
}

bool time_event_get_update_flag(void)
{
    return update_time_events_flag;
}

/**
 * @note The FSM must declare a time_event update method which will be updated 
 * every ms, for STM32 devices the Systick interruption can be used for this propose,
 * if many time events needs to be handled a polling update mode can be used by checking 
 * @time_event_get_update_flag method.
 * 
 * @example 
 * 

void HAL_SYSTICK_Callback(void)
{
    time_event_set_update_flag(true);
}

void time_events_poll_update(void)
{
    if(time_event_get_update_flag() == true)
    {
        Update Time Events of the state machines 
        host_comm_fsm_time_event_update(&host_tx_comm_handle);
        time_event_set_update_flag(false); 
    }
}
*/


