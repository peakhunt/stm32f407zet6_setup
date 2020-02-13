#include "stm32f4xx_hal.h"
#include "dma.h"
#include "app_common.h"
#include "lcd_driver.h"
#include "mainloop_timer.h"
#include "event_dispatcher.h"
#include "event_list.h"
#include "ili9341.h"

#define REFRESH_INTERVAL         30      // 30 FPS is target

#define LCD_WIDTH     240
#define LCD_HEIGHT    320

typedef struct
{
  uint16_t      frame[LCD_WIDTH][LCD_HEIGHT];
} lcd_frame_t;

static SoftTimerElem    _fps_timer;
static lcd_frame_t*     _frame_mem = (lcd_frame_t*)(0x68000000);

/////////////// for DMA ///////////////
static DMA_HandleTypeDef* _dma_handle = &hdma_memtomem_dma2_stream0;
///////////////////////////////////////

//
// XXX
// runs in IRQ context
//
static void lcd_frame_dma_complete_irq(DMA_HandleTypeDef *DmaHandle)
{
  event_set(1 << DISPATCH_EVENT_LCD_DMA_COMPLETE);
}

static void
lcd_frame_dma_complete(uint32_t event)
{
  // FIXME
}

static void
lcd_frame_dma_init(void)
{
  HAL_DMA_RegisterCallback(_dma_handle, HAL_DMA_XFER_CPLT_CB_ID, lcd_frame_dma_complete_irq);
  event_register_handler(lcd_frame_dma_complete, DISPATCH_EVENT_LCD_DMA_COMPLETE);
}

void
lcd_frame_dma_start(void)
{
#if 0
  if(HAL_DMA_Start_IT(&DmaHandle, (uint32_t)&aSRC_Const_Buffer, (uint32_t)&aDST_Buffer, BUFFER_SIZE) != HAL_OK)
  {
    /* Transfer Error */
    Error_Handler();
  }
#endif
}

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

static void
lcd_frame_memory_init(void)
{
  uint16_t    x, y;

  for(y = 0; y < LCD_HEIGHT; y++)
  {
    for(x = 0; x < LCD_WIDTH; x++)
    {
      _frame_mem->frame[x][y] = 0x0;
    }
  }
}

void
lcd_driver_init(void)
{
  uint16_t color = 0x3f << 5;
  uint16_t i;

  ili9341_init_lcd();

  lcd_frame_memory_init();
  lcd_frame_dma_init();

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
