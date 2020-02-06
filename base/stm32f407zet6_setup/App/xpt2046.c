#include "xpt2046.h"

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
xp2046_pt_to_adc(uint16_t l, uint16_t pt)
{
  return pt * (XP2046_ADC_MAX / (float)l);
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
  uint16_t  x0 = XP2046_CAL_OFFSET,
            x1 = _xpt2046.width - XP2046_CAL_OFFSET,
            y0 = XP2046_CAL_OFFSET,
            y1 = _xpt2046.height - XP2046_CAL_OFFSET;

  _xpt2046.xa = (float)(x1 - x0) / (float)(adc_x1 - adc_x0);
  _xpt2046.xb = x0 - adc_x0 * _xpt2046.xa;

  _xpt2046.ya = (float)(y1 - y0) / (float)(adc_y1 - adc_y0);
  _xpt2046.yb = y0 - adc_y0 * _xpt2046.ya;
}

void
xpt2046_init(uint16_t width, uint16_t height)
{
  _xpt2046.width  = width;
  _xpt2046.height = height;

  xpt2046_set_calibration(
      xp2046_pt_to_adc(width, XP2046_CAL_OFFSET),           // adc x0
      xp2046_pt_to_adc(height, XP2046_CAL_OFFSET),          // adc y0
      xp2046_pt_to_adc(width, width - XP2046_CAL_OFFSET),   // adc x1
      xp2046_pt_to_adc(height, height - XP2046_CAL_OFFSET)  // adc y1
  );
}
