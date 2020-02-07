#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "event_list.h"
#include "event_dispatcher.h"

#include "touch_screen.h"
#include "xpt2046.h"

#include "ili9341.h"

static void
touch_screen_irq_handler(uint32_t event)
{
  uint16_t      adc_x, adc_y,
                px, py;
  uint16_t      color = (0x1f << 11);

  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

  xpt2046_read(&adc_x, &adc_y);

  xpt2046_calc_point(adc_x, adc_y, &px, &py);

  ili9341_write_pixel(px, py, color);
}

void
touch_screen_init(void)
{
  event_register_handler(touch_screen_irq_handler, DISPATCH_EVENT_TOUCH_SCREEN);
  xpt2046_init(240, 320);
}
