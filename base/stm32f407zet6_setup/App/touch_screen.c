#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "event_list.h"
#include "event_dispatcher.h"
#include "app_common.h"
#include "mainloop_timer.h"

#include "touch_screen.h"
#include "xpt2046.h"

#include "ili9341.h"

#define TOUCH_SCREEN_MAX_SAMPLES    32

static uint32_t _irq_count = 0;

static uint32_t _sample_count = 0;
static uint16_t _x_samples[TOUCH_SCREEN_MAX_SAMPLES];
static uint16_t _y_samples[TOUCH_SCREEN_MAX_SAMPLES];

static SoftTimerElem _touch_timer;

static void
touch_timer_callback(SoftTimerElem* te)
{
  uint32_t      sum_x = 0, sum_y = 0;
  uint16_t      adc_x, adc_y,
                px, py;
  uint16_t      color = (0x1f << 11);

  for(uint32_t i = 0; i < _sample_count; i++)
  {
    sum_x += _x_samples[i];
    sum_y += _y_samples[i];
  }

  adc_x = (uint16_t)(sum_x / _sample_count);
  adc_y = (uint16_t)(sum_y / _sample_count);

  xpt2046_calc_point(adc_x, adc_y, &px, &py);
  ili9341_write_pixel(px, py, color);

  _sample_count = 0;
}

static void
touch_screen_irq_handler(uint32_t event)
{
  uint16_t      adc_r_x, adc_r_y,
                adc_x, adc_y;

  _irq_count++;

  if(_irq_count > 1024)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    _irq_count = 0;
  }

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

  if(_sample_count >= TOUCH_SCREEN_MAX_SAMPLES)
  {
    return;
  }

  //
  // touch screen rotation handling
  //
  adc_x = XPT2046_ADC_MAX - adc_r_y;
  adc_y = adc_r_x;

  _x_samples[_sample_count] = adc_x;
  _y_samples[_sample_count] = adc_y;
  _sample_count++;

  if(!is_soft_timer_running(&_touch_timer))
  {
    mainloop_timer_schedule(&_touch_timer, 2);
    return;
  }
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
