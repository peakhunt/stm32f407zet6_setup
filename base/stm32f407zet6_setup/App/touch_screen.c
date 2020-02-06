#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "event_list.h"
#include "event_dispatcher.h"

#include "touch_screen.h"

static void
touch_screen_irq_handler(uint32_t event)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

void
touch_screen_init(void)
{
  event_register_handler(touch_screen_irq_handler, DISPATCH_EVENT_TOUCH_SCREEN);
}
