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
#define TOUCH_SCREEN_MIN_SAMPLES    6

static uint32_t _irq_count = 0;

static uint32_t _sample_count = 0;
static uint16_t _x_samples[TOUCH_SCREEN_MAX_SAMPLES];
static uint16_t _y_samples[TOUCH_SCREEN_MAX_SAMPLES];

static SoftTimerElem _touch_timer;

static uint32_t _touch_timer_interval = 1;

//////////// calibration ////////////
#define CALIBRATION_MAX_SAMPLES           16

static touch_screen_calibration_callback  _cal_cb = NULL;
static void*                              _cal_cb_arg = NULL;
static touch_screen_calibration_step_t    _cal_step = touch_screen_calibration_step_done;
static uint16_t                           _cal_x[4];
static uint16_t                           _cal_y[4];
static uint16_t                           _cal_x_buffer[CALIBRATION_MAX_SAMPLES];
static uint16_t                           _cal_y_buffer[CALIBRATION_MAX_SAMPLES];
static uint16_t                           _cal_sample_count = 0;
static uint8_t                            _cal_progress = false;

static SoftTimerElem _step_delay;
/////////////////////////////////////

static void
touch_screen_cal_step_callback(SoftTimerElem* te)
{
  _cal_step++;

  _cal_cb(_cal_step, true, _cal_cb_arg);    // notify start

  if(_cal_step == touch_screen_calibration_step_done)
  {
    uint16_t adc_x0, adc_y0, adc_x1, adc_y1;

    adc_x0 = (_cal_x[0] + _cal_x[2]) / 2;
    adc_y0 = (_cal_y[0] + _cal_y[1]) / 2;
    adc_x1 = (_cal_x[1] + _cal_x[3]) / 2;
    adc_y1 = (_cal_y[2] + _cal_y[3]) / 2;

    xpt2046_set_calibration(adc_x0, adc_y0, adc_x1, adc_y1);

    _cal_progress     = false;
    _touch_timer_interval = 1;

    _cal_cb(_cal_step, false, _cal_cb_arg); // notify finish
  }
}

static void
touch_screen_callback(uint16_t x, uint16_t y, uint16_t adc_x, uint16_t adc_y)
{
  uint16_t      color = (0x1f << 11);

  if(_cal_progress)
  {
    _cal_x_buffer[_cal_sample_count] = adc_x;
    _cal_y_buffer[_cal_sample_count] = adc_y;

    _cal_sample_count++;

    if(_cal_sample_count >= CALIBRATION_MAX_SAMPLES)
    {
      uint32_t    sum_x = 0, sum_y = 0;

      _cal_sample_count = 0;
      for(uint8_t i = 0; i < CALIBRATION_MAX_SAMPLES; i++)
      {
        sum_x += _cal_x_buffer[i];
        sum_y += _cal_y_buffer[i];
      }

      _cal_x[_cal_step] = sum_x / CALIBRATION_MAX_SAMPLES;
      _cal_y[_cal_step] = sum_y / CALIBRATION_MAX_SAMPLES;

      _cal_cb(_cal_step, false, _cal_cb_arg);   // notify end
      mainloop_timer_schedule(&_step_delay, 1000);
    }
  }
  else
  {
    ili9341_write_pixel(x, y, color);
  }
}

static void
touch_timer_callback(SoftTimerElem* te)
{
  uint32_t      sum_x = 0, sum_y = 0;
  uint16_t      adc_x, adc_y,
                px, py;

  if(_sample_count < TOUCH_SCREEN_MIN_SAMPLES)
  {
    _sample_count = 0;
    return;
  }

  for(uint32_t i = 0; i < _sample_count; i++)
  {
    sum_x += _x_samples[i];
    sum_y += _y_samples[i];
  }

  adc_x = (uint16_t)(sum_x / _sample_count);
  adc_y = (uint16_t)(sum_y / _sample_count);

  xpt2046_calc_point(adc_x, adc_y, &px, &py);
  _sample_count = 0;

  touch_screen_callback(px, py, adc_x, adc_y);
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
    mainloop_timer_schedule(&_touch_timer, _touch_timer_interval);
    return;
  }
}

void
touch_screen_init(void)
{
  event_register_handler(touch_screen_irq_handler, DISPATCH_EVENT_TOUCH_SCREEN);

  soft_timer_init_elem(&_touch_timer);
  _touch_timer.cb    = touch_timer_callback;

  soft_timer_init_elem(&_step_delay);
  _step_delay.cb = touch_screen_cal_step_callback;

  NVIC_DisableIRQ(T_PEN_EXTI_IRQn);
  __DSB();
  __ISB();

  xpt2046_init(240, 320);

  NVIC_EnableIRQ(T_PEN_EXTI_IRQn);
}

void
touch_screen_start_calibration(touch_screen_calibration_callback cb, void* cb_arg)
{
  _cal_step         = touch_screen_calibration_step_ul;
  _cal_progress     = true;
  _cal_cb           = cb;
  _cal_cb_arg       = cb_arg;
  _cal_sample_count = 0;

  for(uint8_t i = 0; i < 4; i++)
  {
    _cal_x[i] = 0;
    _cal_y[i] = 0;
  }
  _touch_timer_interval = 100;

  _cal_cb(_cal_step, true, _cal_cb_arg); // notify start
}
