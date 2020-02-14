#include "stm32f4xx_hal.h"
#include "dma.h"
#include "app_common.h"
#include "lcd_driver.h"
#include "mainloop_timer.h"
#include "event_dispatcher.h"
#include "event_list.h"
#include "ili9341.h"

#define REFRESH_INTERVAL         41      // 24 FPS is target

#define LCD_WIDTH     ILI9341_LCD_PIXEL_WIDTH
#define LCD_HEIGHT    ILI9341_LCD_PIXEL_HEIGHT

typedef struct
{
  volatile uint16_t         buffer[LCD_HEIGHT*LCD_WIDTH];
} lcd_frame_t;

static SoftTimerElem    _fps_timer;

static volatile lcd_frame_t*     _frame_mem_r = (lcd_frame_t*)(0x68000000U + sizeof(lcd_frame_t) * 0);
static volatile lcd_frame_t*     _frame_mem_g = (lcd_frame_t*)(0x68000000U + sizeof(lcd_frame_t) * 1);
static volatile lcd_frame_t*     _frame_mem_b = (lcd_frame_t*)(0x68000000U + sizeof(lcd_frame_t) * 2);
static volatile lcd_frame_t*     _frame_mem;

/////////////// for DMA ///////////////
static DMA_HandleTypeDef* _dma_handle = &hdma_memtomem_dma2_stream7;
static uint8_t  _dma_in_prog = false;
static uint8_t  _dma_step = 0;
///////////////////////////////////////

static void lcd_dma_test(void);

//
// XXX
// runs in IRQ context
//
static void lcd_frame_dma_complete_irq(DMA_HandleTypeDef *DmaHandle)
{
  event_set(1 << DISPATCH_EVENT_LCD_DMA_COMPLETE);
}

static void
lcd_frame_do_dma(void)
{
  uint32_t offset;
  uint32_t len;

  len = LCD_WIDTH * LCD_HEIGHT / 2;

  if(_dma_step == 0)
  {
    offset = 0;
    ili9341_prepare_frame_update(0, 0);
  }
  else
  {
    offset = LCD_WIDTH * LCD_HEIGHT / 2;
    ili9341_prepare_frame_update(0, LCD_HEIGHT / 2);
  }

  if(HAL_DMA_Start_IT(_dma_handle, (uint32_t)&_frame_mem->buffer[offset], (uint32_t)LCD_DAT, len) != HAL_OK)
  {
    /* Transfer Error */
    while(1)
      ;
  }
}

static void
lcd_frame_dma_complete(uint32_t event)
{
  if(_dma_step == 0)
  {
    _dma_step++;
    lcd_frame_do_dma();
  }
  else
  {
    _dma_in_prog = false;
    _dma_step = 0;
  }
}

static void
lcd_frame_dma_init(void)
{
  HAL_DMA_RegisterCallback(_dma_handle, HAL_DMA_XFER_CPLT_CB_ID, lcd_frame_dma_complete_irq);
  event_register_handler(lcd_frame_dma_complete, DISPATCH_EVENT_LCD_DMA_COMPLETE);
}

static void
lcd_frame_dma_start(void)
{
  _dma_in_prog = true;
  _dma_step = 0;

  lcd_frame_do_dma();
}

static void
lcd_dma_test(void)
{
  static uint8_t step = 0;

  switch(step)
  {
  case 0:
    _frame_mem = _frame_mem_r;
    step = 1;
    break;
  case 1:
    _frame_mem = _frame_mem_g;
    step = 2;
    break;
  case 2:
    _frame_mem = _frame_mem_b;
    step = 0;
    break;
  }

  lcd_frame_dma_start();
}

static void
lcd_test_by_fill(SoftTimerElem* te)
{
  if(_dma_in_prog == false)
  {
    lcd_dma_test();
  }
  mainloop_timer_schedule(&_fps_timer, REFRESH_INTERVAL);
}

static void
lcd_frame_memory_init(void)
{
  uint16_t    x, y;
  uint32_t    ndx;
  const uint16_t    r = 0x001f << 11 | 0x0000 << 5 | 0x0000,
                    g = 0x0000 << 11 | 0x003f << 5 | 0x0000,
                    b = 0x0000 << 11 | 0x0000 << 5 | 0x001f;

  for(y = 0; y < LCD_HEIGHT; y++)
  {
    for(x = 0; x < LCD_WIDTH; x++)
    {
      ndx = y * LCD_WIDTH + x;
      _frame_mem_r->buffer[ndx] = r;
      _frame_mem_g->buffer[ndx] = g;
      _frame_mem_b->buffer[ndx] = b;
    }
  }

  for(y = 0; y < LCD_HEIGHT; y++)
  {
    for(x = 0; x < LCD_WIDTH; x++)
    {
      ndx = y * LCD_WIDTH + x;

      if(_frame_mem_r->buffer[ndx] != r) while(1);
      if(_frame_mem_g->buffer[ndx] != g) while(1);
      if(_frame_mem_b->buffer[ndx] != b) while(1);
    }
  }
}

void
lcd_driver_init(void)
{
  ili9341_init_lcd();

  lcd_frame_memory_init();
  lcd_frame_dma_init();

  soft_timer_init_elem(&_fps_timer);
  _fps_timer.cb    = lcd_test_by_fill;

  mainloop_timer_schedule(&_fps_timer, REFRESH_INTERVAL);
}
