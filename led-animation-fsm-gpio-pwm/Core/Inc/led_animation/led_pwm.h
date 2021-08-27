#ifndef LED_PWM_H
#define LED_PWM_H

#include "stm32f4xx_hal.h"

#define LED_MAX_BRIGHTNESS          (100)   
#define LED_MIN_BRIGHTNESS          (0)

typedef struct
{
    TIM_HandleTypeDef *tim;
    uint32_t ch;
} led_pwm_t;

void led_pwm_init(void);
uint8_t led_pwm_set_brightness(led_pwm_t *pwm, uint8_t level);
void led_pwm_on(led_pwm_t *pwm);
void led_pwm_off(led_pwm_t *pwm);

#endif