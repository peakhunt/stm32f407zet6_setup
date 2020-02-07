#include "app_common.h"
#include "lcd_driver.h"
#include "mainloop_timer.h"
#include "ili9341.h"

#define REFRESH_INTERVAL         30      // 30 FPS is target

static SoftTimerElem    _fps_timer;


static void
lcd_test_by_fill(SoftTimerElem* te)
{
  uint16_t col, row;
  static uint16_t r = 0,
                  g = 0,
                  b = 0;
  uint16_t color = 0x0;

  // mainloop_timer_schedule(&_fps_timer, REFRESH_INTERVAL);

  color = (r << 11) | (g << 5) | b;

  for(row = 0; row < 320; row++)
  {
    for(col = 0; col < 240; col++)
    {
      //
      // 5-6-5 format
      // 5 : 0x1F
      // 6 : 0x3F
      //
      ili9341_write_pixel(col, row, color);
    }
  }

  if(r < 0x1f)
  {
    r++;
    return;
  }

  if(g < 0x3f)
  {
    g++;
    return;
  }

  if(b < 0x1f)
  {
    b++;
    return;
  }

  r = g = b = 0;
}

void
lcd_driver_init(void)
{
  uint16_t color = 0x3f << 5;
  uint16_t i;

  ili9341_init_lcd();

  soft_timer_init_elem(&_fps_timer);
  _fps_timer.cb    = lcd_test_by_fill;
  // mainloop_timer_schedule(&_fps_timer, REFRESH_INTERVAL);
  lcd_test_by_fill(NULL);
  for(i = 10; i < (240 -10); i++)
  {
    ili9341_write_pixel(i, 10, color);
    ili9341_write_pixel(i, 320 - 10, color);
  }

  for(i = 10; i < (320 - 10); i++)
  {
    ili9341_write_pixel(240 - 10, i, color);
    ili9341_write_pixel(10, i, color);
  }
}
