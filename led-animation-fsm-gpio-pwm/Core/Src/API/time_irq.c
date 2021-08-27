
#include "time_event.h"
#include "stm32f4xx_hal.h"
#include "host_comm_tx_fsm.h"
#include "host_comm_rx_fsm.h"
#include "led_animation.h"

/**
 * @brief Systick Callback Function 
 * @note  This callback is executed every ms
 */
void HAL_SYSTICK_Callback(void)
{
    /* update FSM time events*/
    host_comm_tx_fsm_time_event_update(&host_comm_tx_handle);
    host_comm_rx_fsm_time_event_update(&host_comm_rx_handle);
    led_animation_time_update(&led_animation);

}
