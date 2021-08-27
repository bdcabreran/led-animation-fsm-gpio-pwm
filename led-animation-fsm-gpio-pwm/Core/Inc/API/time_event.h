
/**
 * @brief time event header file used to control time - related events in FSM
 * 
 */

#ifndef TIME_EVENT_H
#define TIME_EVENT_H

#include <stdint.h>
#include <stdbool.h>


typedef struct
{
    uint32_t millis_cnt;
    bool active;
    bool raised;
}time_event_t;

void time_event_start(time_event_t *time_event, const uint32_t time_ms);
void time_event_stop(time_event_t *time_event);
bool time_event_is_active(time_event_t *time_event);
void time_event_set_update_flag(bool status);
bool time_event_get_update_flag(void);
bool time_event_update(time_event_t *time_event);
bool time_event_is_raised(time_event_t *time_event);
extern void time_events_poll_update(void);

#endif
