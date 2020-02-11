#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "app_common.h"
#include "app.h"
#include "event_dispatcher.h"
#include "mainloop_timer.h"

#include "lcd_driver.h"
#include "touch_screen.h"
#include "blinky.h"

#include "dwt_stm32_delay.h"

void
app_init_f(void)
{
  DWT_Delay_Init();
  event_dispatcher_init();
  mainloop_timer_init();
}

void
app_init_r(void)
{
  lcd_driver_init();
  touch_screen_init();
  blinky_init();
}

void
app_mainloop(void)
{
  while(1)
  {
    event_dispatcher_dispatch();
  }
}
