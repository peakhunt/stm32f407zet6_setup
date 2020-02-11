#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "event_list.h"
#include "event_dispatcher.h"
#include "app_common.h"
#include "mainloop_timer.h"

#include "touch_screen.h"
#include "xpt2046.h"

#include "ili9341.h"

static uint32_t _irq_count = 0;
static uint32_t _loop_count = 0;

static SoftTimerElem _touch_timer;

static void
touch_timer_callback(SoftTimerElem* te)
{
  uint16_t      adc_r_x, adc_r_y,
                adc_x, adc_y,
                px, py;
  uint16_t      color = (0x1f << 11);

  while(_loop_count > 0)
  {
    _loop_count--;

    if(HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin) == GPIO_PIN_SET)
    {
      _loop_count = 0;
      return;
    }

    NVIC_DisableIRQ(T_PEN_EXTI_IRQn);
    __DSB();
    __ISB();

    xpt2046_read(&adc_r_x, &adc_r_y);

    NVIC_EnableIRQ(T_PEN_EXTI_IRQn);

    if (adc_r_x == 0xffff || adc_r_y == 0xffff ||
        adc_r_x == 0 || adc_r_y == 0)
    {
      continue;
    }

    //
    // touch screen rotation handling
    //
    adc_x = XPT2046_ADC_MAX - adc_r_y;
    adc_y = adc_r_x;

    xpt2046_calc_point(adc_x, adc_y, &px, &py);

    ili9341_write_pixel(px, py, color);
  }
}

static void
touch_screen_irq_handler(uint32_t event)
{
#if 0
  uint16_t      adc_r_x, adc_r_y,
                adc_x, adc_y,
                px, py;
  uint16_t      color = (0x1f << 11);
#endif

  _irq_count++;
  _loop_count++;

  if(_irq_count > 64)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    _irq_count = 0;
  }


  if(is_soft_timer_running(&_touch_timer))
  {
    return;
  }

  mainloop_timer_schedule(&_touch_timer, 2);

#if 0
  if(HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin) == GPIO_PIN_SET)
  {
    return;
  }

  NVIC_DisableIRQ(T_PEN_EXTI_IRQn);
  __DSB();
  __ISB();

  xpt2046_read(&adc_r_x, &adc_r_y);

  NVIC_EnableIRQ(T_PEN_EXTI_IRQn);

  if (adc_r_x == 0xffff || adc_r_y == 0xffff ||
      adc_r_x == 0 || adc_r_y == 0)
  {
    return;
  }

  //
  // touch screen rotation handling
  //
  adc_x = XPT2046_ADC_MAX - adc_r_y;
  adc_y = adc_r_x;

  xpt2046_calc_point(adc_x, adc_y, &px, &py);

  ili9341_write_pixel(px, py, color);
#endif
}

void
touch_screen_init(void)
{
  event_register_handler(touch_screen_irq_handler, DISPATCH_EVENT_TOUCH_SCREEN);

  soft_timer_init_elem(&_touch_timer);
  _touch_timer.cb    = touch_timer_callback;

  NVIC_DisableIRQ(T_PEN_EXTI_IRQn);
  __DSB();
  __ISB();

  xpt2046_init(240, 320);

  NVIC_EnableIRQ(T_PEN_EXTI_IRQn);
}
