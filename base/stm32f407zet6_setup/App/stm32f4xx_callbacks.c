#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "app_common.h"
#include "event_dispatcher.h"
#include "event_list.h"

volatile uint32_t     __uptime = 0;

void HAL_SYSTICK_Callback(void)
{
  static uint16_t   count = 0;

  count++;
  if(count >= 1000)
  {
    __uptime++;
    count = 0;
  }

  event_set(1 << DISPATCH_EVENT_TIMER_TICK);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  event_set(1 << DISPATCH_EVENT_TOUCH_SCREEN);
}
