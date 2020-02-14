#include "stm32f4xx_hal.h"
#include "ili9341.h"

static void
lcd_write_cmd8(uint8_t cmd)
{
  *LCD_REG = cmd;
}

static void
lcd_write_data8(uint8_t data)
{
  *LCD_DAT = data;
}

static void
lcd_write_data16(uint16_t data)
{
  *LCD_DAT = data;
}

static void
lcd_write_cmd_multiple(uint8_t cmd, uint8_t* data, uint32_t size)
{
  *LCD_REG = cmd;
  while(size--)
  {
    *LCD_DAT = *data;
    data++;
  }
}

#define LCD_IO_WriteData16_to_2x8(dt)     \
  lcd_write_data8((uint8_t)((dt) >> 8));  \
  lcd_write_data8((uint8_t)(dt));

void
ili9341_init_lcd(void)
{
  lcd_write_cmd8(ILI9341_SWRESET);
  HAL_Delay(10);

  lcd_write_cmd_multiple(0xEF, (uint8_t *)"\x03\x80\x02", 3);
  lcd_write_cmd_multiple(0xCF, (uint8_t *)"\x00\xC1\x30", 3);
  lcd_write_cmd_multiple(0xED, (uint8_t *)"\x64\x03\x12\x81", 4);
  lcd_write_cmd_multiple(0xE8, (uint8_t *)"\x85\x00\x78", 3);
  lcd_write_cmd_multiple(0xCB, (uint8_t *)"\x39\x2C\x00\x34\x02", 5);
  lcd_write_cmd_multiple(0xF7, (uint8_t *)"\x20", 1);
  lcd_write_cmd_multiple(0xEA, (uint8_t *)"\x00\x00", 2);

  lcd_write_cmd_multiple(ILI9341_PWCTR1, (uint8_t *)"\x23", 1);
  lcd_write_cmd_multiple(ILI9341_PWCTR2, (uint8_t *)"\x10", 1);
  lcd_write_cmd_multiple(ILI9341_VMCTR1, (uint8_t *)"\x3E\x28", 2);
  lcd_write_cmd_multiple(ILI9341_VMCTR2, (uint8_t *)"\x86", 1);

  // Vertical scroll zero
  // XXX ILI9341_MADCTL
  lcd_write_cmd_multiple(ILI9341_VSCRSADD, (uint8_t *)"\x00", 1);
  lcd_write_cmd_multiple(ILI9341_PIXFMT, (uint8_t *)"\x55", 1);

  // lcd_write_cmd_multiple(0xF6, (uint8_t *)"\x01\x00\x06", 3);

  lcd_write_cmd_multiple(ILI9341_FRMCTR1, (uint8_t *)"\x00\x18", 2);
  lcd_write_cmd_multiple(ILI9341_DFUNCTR, (uint8_t *)"\x08\x82\x27", 3); 
  lcd_write_cmd_multiple(0xF2, (uint8_t *)"\x00", 1);                     // 3Gamma Function Disable
  lcd_write_cmd_multiple(ILI9341_GAMMASET, (uint8_t *)"\x01", 1);

  lcd_write_cmd_multiple(ILI9341_GMCTRP1, (uint8_t *)"\x0F\x31\x2B\x0C\x0E\x08\x4E\xF1\x37\x07\x10\x03\x0E\x09\x00", 15);
  lcd_write_cmd_multiple(ILI9341_GMCTRN1, (uint8_t *)"\x00\x0E\x14\x03\x11\x07\x31\xC1\x48\x08\x0F\x0C\x31\x36\x0F", 15);

  lcd_write_cmd8(ILI9341_MADCTL);
  lcd_write_data8(ILI9341_MAD_DATA_RIGHT_THEN_DOWN);

  lcd_write_cmd8(ILI9341_SLPOUT);    // Exit Sleep
  HAL_Delay(10);
  lcd_write_cmd8(ILI9341_DISPON);    // Display on
  HAL_Delay(10);
}

void
ili9341_write_pixel(uint16_t x, uint16_t y, uint16_t rgb)
{
  // set cursor
  lcd_write_cmd8(ILI9341_CASET);
  LCD_IO_WriteData16_to_2x8(x);
  LCD_IO_WriteData16_to_2x8(x);

  lcd_write_cmd8(ILI9341_PASET);
  LCD_IO_WriteData16_to_2x8(y);
  LCD_IO_WriteData16_to_2x8(y);

  // set pixel
  lcd_write_cmd8(ILI9341_RAMWR);
  lcd_write_data16(rgb);
}

void
ili9341_write_pixel_multi(uint16_t x, uint16_t y, uint16_t* colors, uint32_t len)
{
#if 1
  // uint16_t color = 0x001f << 11 | 0x0000 << 5 | 0x0000;

  // set cursor
  lcd_write_cmd8(ILI9341_CASET);
  LCD_IO_WriteData16_to_2x8(0);
  LCD_IO_WriteData16_to_2x8(ILI9341_LCD_PIXEL_WIDTH - 1);

  lcd_write_cmd8(ILI9341_PASET);
  LCD_IO_WriteData16_to_2x8(0);
  LCD_IO_WriteData16_to_2x8(ILI9341_LCD_PIXEL_HEIGHT - 1);

  lcd_write_cmd8(ILI9341_RAMWR);

#if 0
  // FSMC timing conflict test
  for(uint32_t i = 0; i < len; i++)
  {
    if(colors[i] != color)
    {
      while(1);
    }
  }
#endif

  for(uint32_t i = 0; i < len; i++)
  {
    *LCD_DAT = colors[i];
  }

#else
  for(uint32_t y = 0; y < ILI9341_LCD_PIXEL_HEIGHT; y++)
  {
    for(uint32_t x = 0; x < ILI9341_LCD_PIXEL_WIDTH; x++)
    {
      ili9341_write_pixel(x, y, colors[x + y * ILI9341_LCD_PIXEL_WIDTH]);
    }
  }
#endif
}

void
ili9341_prepare_frame_update(uint16_t x, uint16_t y)
{
  // set cursor
  lcd_write_cmd8(ILI9341_CASET);
  LCD_IO_WriteData16_to_2x8(x);
  LCD_IO_WriteData16_to_2x8(ILI9341_LCD_PIXEL_WIDTH);

  lcd_write_cmd8(ILI9341_PASET);
  LCD_IO_WriteData16_to_2x8(y);
  LCD_IO_WriteData16_to_2x8(ILI9341_LCD_PIXEL_HEIGHT);

  lcd_write_cmd8(ILI9341_RAMWR);
}

void
ili9341_prepare_frame_continue(void)
{
  lcd_write_cmd8(0x3c);
}
