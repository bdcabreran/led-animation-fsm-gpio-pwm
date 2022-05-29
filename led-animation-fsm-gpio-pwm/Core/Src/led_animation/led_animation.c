/**
 * @file led_animation.c
 * @author Bayron Cabrera (bayron.cabrera@titima.com)
 * @brief  Led Animation Functions
 * @version 0.1
 * @date 2021-01-20
 * @copyright Copyright (c) 2021
 * 
 */
#include "led_animation.h"

led_animation_fsm_t led_animation;

/**@brief Enable/Disable debug messages */
#define LED_ANIMATION_FSM_DBG 1
#define LED_ANIMATION_TAG "led animation : "

/**@brief uart debug function for server comm operations  */
#if LED_ANIMATION_FSM_DBG
#define led_animation_dbg(format, ...) printf(LED_ANIMATION_TAG format, ##__VA_ARGS__)
#else
#define led_animation_dbg(format, ...) \
    do                                    \
    { /* Do nothing */                    \
    } while (0)
#endif


/*Functions reltaed to led animation EXEC */
static void enter_seq_exec_proc(led_animation_fsm_t *handle);
static void entry_action_exec_proc(led_animation_fsm_t *handle);
static void exit_action_exec_proc(led_animation_fsm_t *handle);
static bool exec_proc_on_react(led_animation_fsm_t *handle);

/*Functions reltaed to led animation IDLE */
static void enter_seq_idle_proc(led_animation_fsm_t *handle);
static bool idle_proc_on_react(led_animation_fsm_t *handle);
static void entry_action_idle_proc(led_animation_fsm_t *handle);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void led_animation_set_next_state(led_animation_fsm_t *handle, led_animation_state_t state)
{
    handle->state = state;
    handle->event.internal = ev_int_led_animation_invalid;
    handle->event.external = ev_ext_led_animation_invalid;
}

void led_animation_init(led_animation_fsm_t *handle, led_pwm_t *led)
{
    /*init mcu gpio .. */
    handle->iface.led.ch = led->ch;
    handle->iface.led.tim = led->tim;

    /*enter idle state */
    enter_seq_idle_proc(handle);
}

void led_animation_run(led_animation_fsm_t *handle)
{
    switch (handle->state)
    {
    case st_led_animation_idle: idle_proc_on_react(handle); break;
    case st_led_animation_exec: exec_proc_on_react(handle); break;
    default:
        break;
    }
}

void led_animation_time_update(led_animation_fsm_t *handle)
{
    time_event_t *time_event = (time_event_t *)&handle->event.time;
	for (int tev_idx = 0; tev_idx < sizeof(handle->event.time) / sizeof(time_event_t); tev_idx++)
	{
		time_event_update(time_event);
		time_event++;
	}
}

bool is_led_animation_ongoing(led_animation_fsm_t *handle)
{
    return (handle->state == st_led_animation_idle);
}

/**
 * @brief Set Animation to be executed 
 */
uint8_t led_animation_start(led_animation_fsm_t *handle, led_animation_t *animation)
{
    if(led_set_brightness(handle, animation->brightness))
    {
        memcpy((uint8_t*)&handle->iface.animation, (uint8_t*)animation, sizeof(led_animation_t));
        handle->event.external = ev_ext_led_animation_start;
        return 1;
    }
    return 0;
}

uint8_t led_set_brightness(led_animation_fsm_t *handle, uint8_t brightness)
{
    led_animation_dbg("func \t[ set brightness -> [%d] ]\n", brightness);
    return led_pwm_set_brightness(&handle->iface.led, brightness);
}


void led_animation_stop(led_animation_fsm_t *handle)
{
    led_animation_dbg("func \t[ animation stop ]\n");
    handle->event.external = ev_ext_led_animation_stop;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void enter_seq_idle_proc(led_animation_fsm_t *handle)
{
    led_animation_dbg("enter seq \t[ idle proc ]\n");
    led_animation_set_next_state(handle, st_led_animation_idle);
    led_pwm_off(&handle->iface.led);
}

static bool idle_proc_on_react(led_animation_fsm_t *handle)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = true;

    if (handle->event.external == ev_ext_led_animation_start)
    {
        /*New Led service ready to start*/
        enter_seq_exec_proc(handle);
    }
    else
        did_transition = false;

    return did_transition;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static void enter_seq_exec_proc(led_animation_fsm_t *handle)
{
    led_animation_dbg("enter seq \t[ execution proc ]\n");
    led_animation_set_next_state(handle, st_led_animation_exec);
    entry_action_exec_proc(handle);
}

static void entry_action_exec_proc(led_animation_fsm_t *handle)
{
    led_animation_dbg("entry act \t[ execution proc -> start timers, led on ]\n");

    time_event_start(&handle->event.time.exec_time_expired, handle->iface.animation.execution_time);
    time_event_start(&handle->event.time.period_expired, handle->iface.animation.period);
    time_event_start(&handle->event.time.time_on_expired, handle->iface.animation.time_on);
    
    led_pwm_on(&handle->iface.led);
}

static void exit_action_exec_proc(led_animation_fsm_t *handle)
{
    led_animation_dbg("exit act \t[ execution proc -> stop timers, led off ]\n");

    /*stop timers*/
    time_event_stop(&handle->event.time.exec_time_expired);
    time_event_stop(&handle->event.time.period_expired);
    time_event_stop(&handle->event.time.time_on_expired);

    /*start led off*/
    led_pwm_off(&handle->iface.led);
}

static void during_action_exec_proc(led_animation_fsm_t *handle)
{
    /*Update Period/Time on ms 
    
    ____|▔▔▔▔▔|________|▔▔▔▔▔|________
        |------- T --------|
        |---Ton---|
    */
    if(time_event_is_raised(&handle->event.time.period_expired) == true)
    {
        led_pwm_on(&handle->iface.led);
        time_event_start(&handle->event.time.time_on_expired, handle->iface.animation.time_on);
        time_event_start(&handle->event.time.period_expired, handle->iface.animation.period);
    }

    else if(time_event_is_raised(&handle->event.time.time_on_expired) == true)
    {
        led_pwm_off(&handle->iface.led);
    }

    /*Check if pattern execution time expired */
    if(time_event_is_raised(&handle->event.time.exec_time_expired) == true)
    {
        handle->event.internal = ev_int_led_animation_completed;
    }
}

static bool exec_proc_on_react(led_animation_fsm_t *handle)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = true;

    if (handle->event.external == ev_ext_led_animation_start)
    {
        /*new animation ready to launch*/
        exit_action_exec_proc(handle);
        enter_seq_exec_proc(handle);
    }
    
    else if(handle->event.external == ev_ext_led_animation_stop)
    {
        /*enter sequence to idle*/
        exit_action_exec_proc(handle);
        enter_seq_idle_proc(handle);
    }

    else if(handle->event.internal = ev_int_led_animation_completed)
    {
        if(handle->iface.animation.execution_time == LED_ENDLESS_EXEC_TIME)
        {
            /*run animation infinitely*/
            exit_action_exec_proc(handle);
            enter_seq_exec_proc(handle);
        }
        else
        {
        	exit_action_exec_proc(handle);
        	enter_seq_idle_proc(handle);
        }
    }
    else
        did_transition = false;

    //---------------- during action ------------------//
   if (did_transition == false)
   {
       during_action_exec_proc(handle);
   }

    return did_transition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
