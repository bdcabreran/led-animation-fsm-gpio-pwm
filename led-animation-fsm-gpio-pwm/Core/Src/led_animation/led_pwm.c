#include "led_pwm.h"

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void Error_Handler(void);
TIM_HandleTypeDef htim2;


void led_pwm_init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /*  PWM signal with 10KHz and variable duty cycle from 0-100
    F = Timer CLK / ( Prescaler + 1 )( Period +1 )
    F = 84MHz / (84 + 1) (99 + 1) = 10KHz
  */
  #define PRESCALER (84)
  #define PERIOD    (99)

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = PRESCALER;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = PERIOD;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /*Led Configuration : 
    Configure PA5 -> LED 2 -> TIM2 PWM_CH1
  */ 

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);
}

uint8_t led_pwm_set_brightness(led_pwm_t *pwm, uint8_t level)
{
    if (level <= LED_MAX_BRIGHTNESS)
    {
    	__HAL_TIM_SET_COMPARE(pwm->tim, pwm->ch, level);
        return 1;
    }
    return 0;
}

void led_pwm_on(led_pwm_t *pwm)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void led_pwm_off(led_pwm_t *pwm)
{
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}
