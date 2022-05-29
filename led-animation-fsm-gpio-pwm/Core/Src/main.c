/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "peripherals_init.h"
#include "host_comm_tx_fsm.h"
#include "host_comm_rx_fsm.h"
#include "led_animation.h"
#include "stdio.h"
#include "tdd.h" 
#include "uart_driver.h"
#include "led_pwm.h"

#define HEARTBEAT_PERIOD_MS (200)
void heartbeat_handler(void);


void print_startup_message(void)
{
	printf("**************************************\r\n");
	printf("Brief:\t LED animation GPIO FSM\r\n");
	printf("Author:\t Bayron Cabrera \r\n");
	printf("Board:\t Nucleo F411RE \r\n");
	printf("Date:\t %s\r\n", __DATE__);
	printf("**************************************\r\n");
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU configuration */
  peripherals_init();
  print_startup_message();

  /* init host tx fsm*/
  host_comm_tx_fsm_init(&host_comm_tx_handle);
  host_comm_rx_fsm_init(&host_comm_rx_handle);

  /*example */
  led_pwm_t led2 =
      {
          .ch = TIM_CHANNEL_1,
          .tim = &htim2};

  led_animation_t blink =
      {
          .brightness = 50,
          .execution_time = LED_ENDLESS_EXEC_TIME,
          .period = 200,
          .time_on = 100};

  led_animation_init(&led_animation, &led2);
  led_animation_start(&led_animation, &blink);

  /* Infinite loop */
  while (1)
  {
    host_comm_tx_fsm_run(&host_comm_tx_handle);
    host_comm_rx_fsm_run(&host_comm_rx_handle);
    
    led_animation_run(&led_animation);
  }
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
