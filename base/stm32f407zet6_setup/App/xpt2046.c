#include "xpt2046.h"
#include "gpio.h"

#include "dwt_stm32_delay.h"

#define XPT2046_SPI_DELAY(x)         HAL_Delay(x)
#define XPT2046_MAX_SAMPLES         2

#define CTRL_LO_DFR     (0b0000)
#define CTRL_LO_DFR_ID  (0b0011)
#define CTRL_LO_SER     (0b0100)
#define CTRL_HI_X       (0b1001  << 4)
#define CTRL_HI_Y       (0b1101  << 4)

#define CTRL_LO_READ    CTRL_LO_DFR_ID

#define XPT2046_PWR_DOWN()      \
  (void)xpt2046_read_write(CTRL_HI_X | CTRL_LO_SER);\
  (void)xpt2046_read_write(0);\
  (void)xpt2046_read_write(0);

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

static inline uint8_t
xpt2046_read_write(uint8_t data)
{
  uint8_t ret = 0;

  // first 8 command bits
  for(uint8_t bit = 0x80; bit > 0; bit >>= 1)
  {
    // latch output
    HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, (data & bit) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // toggle clock
    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);

    // read input
    if(HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin) == GPIO_PIN_SET)
    {
      ret |= bit;
    }
  }

  return ret;
}

void
xpt2046_power_down()
{
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);

  XPT2046_PWR_DOWN();

  // CS HIGH
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);
}

void
xpt2046_read(uint16_t* x, uint16_t* y)
{
  uint16_t    prev_x = 0xffff, cur_x = 0xffff,
              prev_y = 0xffff, cur_y = 0xffff;
  uint8_t i = 0;

  // CS LOW
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);

  // initiate read X
  (void)xpt2046_read_write(CTRL_HI_X | CTRL_LO_READ);

  do
  {
    prev_x = cur_x;
    prev_y = cur_y;

    // read X, initiate Y read
    cur_x = xpt2046_read_write(0);
    cur_x = (cur_x << 4) | (xpt2046_read_write(CTRL_HI_Y | CTRL_LO_READ) >> 4);

    // read Y, initiate X read
    cur_y = xpt2046_read_write(0);
    cur_y = (cur_y << 4) | (xpt2046_read_write(CTRL_HI_X | CTRL_LO_READ) >> 4);
  //} while(((prev_x != cur_x) || (prev_y != cur_y)) && (++i < XPT2046_MAX_SAMPLES));
  } while((++i < XPT2046_MAX_SAMPLES));

  // read first dummy
  (void)xpt2046_read_write(0);

  // read second dummy, initiate power down
  XPT2046_PWR_DOWN();

  // CS HIGH
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);

  *x = cur_x;
  *y = cur_y;
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

  // just to be sure
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);

  // power down for IRQ
  // I don't know why but this powerdown 
  // causes weird IRQ behavior.
  // xpt2046_power_down();
}
