#include "xpt2046.h"
#include "gpio.h"

typedef struct
{
  uint16_t    width;
  uint16_t    height;

  // y = ax + b equation
  float       xa;
  float       xb;
  float       ya;
  float       yb;
} xpt2046_t;

static xpt2046_t _xpt2046;

static inline uint16_t
xpt2046_pt_to_adc(uint16_t l, uint16_t pt)
{
  return pt * (XPT2046_ADC_MAX / (float)l);
}

void
xpt2046_calc_point(uint16_t adc_x, uint16_t adc_y,
    uint16_t* px, uint16_t* py)
{
  uint16_t x, y;

  x = (uint16_t)(_xpt2046.xa * adc_x + _xpt2046.xb);
  y = (uint16_t)(_xpt2046.ya * adc_y + _xpt2046.yb);

  if(x >= _xpt2046.width)
  {
    x = _xpt2046.width;
  }

  if(y >= _xpt2046.height)
  {
    y = _xpt2046.height;
  }

  *px = x;
  *py = y;
}

void
xpt2046_set_calibration(uint16_t adc_x0,
                        uint16_t adc_y0,
                        uint16_t adc_x1,
                        uint16_t adc_y1)
{
  uint16_t  x0 = XPT2046_CAL_OFFSET,
            x1 = _xpt2046.width - XPT2046_CAL_OFFSET,
            y0 = XPT2046_CAL_OFFSET,
            y1 = _xpt2046.height - XPT2046_CAL_OFFSET;

  _xpt2046.xa = (float)(x1 - x0) / (float)(adc_x1 - adc_x0);
  _xpt2046.xb = x0 - adc_x0 * _xpt2046.xa;

  _xpt2046.ya = (float)(y1 - y0) / (float)(adc_y1 - adc_y0);
  _xpt2046.yb = y0 - adc_y0 * _xpt2046.ya;
}

void
xpt2046_read(uint8_t cmd, uint16_t* d)
{
  uint8_t   dh, dl;

  dh = dl = 0;

  // CS LOW
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);

  // first 8 command bits
  for(uint8_t bit = 0x80; bit > 0; bit >>= 1)
  {
    // latch data
    HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, (cmd & bit) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // toggle clock
    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET);
    // XXX delay?
    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
  }

  //
  // check busy signal ???
  //

  // data high
  for(uint8_t bit = 0x80; bit > 0; bit >>= 1)
  {
    // toggle clock
    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET);

    // XXX delay?

    // read data
    if(HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin) == GPIO_PIN_SET)
    {
      dh |= bit;
    }

    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
  }

  // data low
  for(uint8_t bit = 0x80; bit > 0; bit >>= 1)
  {
    // toggle clock
    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET);

    // XXX delay?

    // read data
    if(HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin) == GPIO_PIN_SET)
    {
      dl |= bit;
    }

    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
  }

  // CS HIGH
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);

  *d = (((uint16_t)dl << 4) | ((uint16_t)dl >> 4)); 
}

void
xpt2046_init(uint16_t width, uint16_t height)
{
  _xpt2046.width  = width;
  _xpt2046.height = height;

  xpt2046_set_calibration(
      xpt2046_pt_to_adc(width, XPT2046_CAL_OFFSET),           // adc x0
      xpt2046_pt_to_adc(height, XPT2046_CAL_OFFSET),          // adc y0
      xpt2046_pt_to_adc(width, width - XPT2046_CAL_OFFSET),   // adc x1
      xpt2046_pt_to_adc(height, height - XPT2046_CAL_OFFSET)  // adc y1
  );
}
