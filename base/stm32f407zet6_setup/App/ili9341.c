#include "stm32f4xx_hal.h"
#include "ili9341.h"

#define LCD_BASE_ADDRESS        0x6C000000
#define LCD_ADDRESS_6           (1 << (6 + 1))
// BANK-1 CS4 with FMC_ADDR6 high
#define LCD_DATA_ADDRESS        (LCD_BASE_ADDRESS | LCD_ADDRESS_6)
// BANK-1 CS4 with FMC_ADDR6 low
#define LCD_COMMAND_ADDRESS     (LCD_BASE_ADDRESS)

#define LCD_REG     ((volatile uint16_t*)LCD_COMMAND_ADDRESS)
#define LCD_DAT     ((volatile uint16_t*)LCD_DATA_ADDRESS)

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

#define LCD_IO_WriteData16_to_2x8(dt)    {lcd_write_data8((dt) >> 8); lcd_write_data8(dt); }

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

  // Power Control 1 (Vreg1out, Verg2out)
  lcd_write_cmd_multiple(ILI9341_PWCTR1, (uint8_t *)"\x23", 1);

  // Power Control 2 (VGH,VGL)
  lcd_write_cmd_multiple(ILI9341_PWCTR2, (uint8_t *)"\x10", 1);

  // Power Control 3 (Vcom)
  lcd_write_cmd_multiple(ILI9341_VMCTR1, (uint8_t *)"\x3E\x28", 2);

  // Power Control 3 (Vcom)
  lcd_write_cmd_multiple(ILI9341_VMCTR2, (uint8_t *)"\x86", 1);

  // Vertical scroll zero
  lcd_write_cmd_multiple(ILI9341_VSCRSADD, (uint8_t *)"\x00", 1);
  lcd_write_cmd_multiple(ILI9341_PIXFMT, (uint8_t *)"\x55", 1);

  // lcd_write_cmd_multiple(0xF6, (uint8_t *)"\x01\x00\x06", 3);

  lcd_write_cmd_multiple(ILI9341_FRMCTR1, (uint8_t *)"\x00\x18", 2);
  lcd_write_cmd_multiple(ILI9341_DFUNCTR, (uint8_t *)"\x08\x82\x27", 3);  // Display Function Control
  lcd_write_cmd_multiple(0xF2, (uint8_t *)"\x00", 1);            // 3Gamma Function Disable
  lcd_write_cmd_multiple(ILI9341_GAMMASET, (uint8_t *)"\x01", 1);// Gamma curve selected

  // positive gamma control
  lcd_write_cmd_multiple(ILI9341_GMCTRP1, (uint8_t *)"\x0F\x31\x2B\x0C\x0E\x08\x4E\xF1\x37\x07\x10\x03\x0E\x09\x00", 15);

  // negative gamma control
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
