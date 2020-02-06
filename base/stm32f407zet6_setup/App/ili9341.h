#ifndef __ILI9341_DEF_H__
#define __ILI9341_DEF_H__

#include <stdint.h>

#define ILI9341_LCD_PIXEL_WIDTH     320
#define ILI9341_LCD_PIXEL_HEIGHT    240

#define ILI9341_NOP            0x00
#define ILI9341_SWRESET        0x01

#define ILI9341_RDDID          0x04
#define ILI9341_RDDST          0x09
#define ILI9341_RDMODE         0x0A
#define ILI9341_RDMADCTL       0x0B
#define ILI9341_RDPIXFMT       0x0C
#define ILI9341_RDIMGFMT       0x0D
#define ILI9341_RDSELFDIAG     0x0F

#define ILI9341_SLPIN          0x10
#define ILI9341_SLPOUT         0x11
#define ILI9341_PTLON          0x12
#define ILI9341_NORON          0x13

#define ILI9341_INVOFF         0x20
#define ILI9341_INVON          0x21
#define ILI9341_GAMMASET       0x26
#define ILI9341_DISPOFF        0x28
#define ILI9341_DISPON         0x29

#define ILI9341_CASET          0x2A
#define ILI9341_PASET          0x2B
#define ILI9341_RAMWR          0x2C
#define ILI9341_RAMRD          0x2E

#define ILI9341_PTLAR          0x30
#define ILI9341_MADCTL         0x36
#define ILI9341_VSCRSADD       0x37     /* Vertical Scrolling Start Address */
#define ILI9341_PIXFMT         0x3A     /* COLMOD: Pixel Format Set */

#define ILI9341_RGB_INTERFACE  0xB0     /* RGB Interface Signal Control */
#define ILI9341_FRMCTR1        0xB1
#define ILI9341_FRMCTR2        0xB2
#define ILI9341_FRMCTR3        0xB3
#define ILI9341_INVCTR         0xB4
#define ILI9341_DFUNCTR        0xB6     /* Display Function Control */

#define ILI9341_PWCTR1         0xC0
#define ILI9341_PWCTR2         0xC1
#define ILI9341_PWCTR3         0xC2
#define ILI9341_PWCTR4         0xC3
#define ILI9341_PWCTR5         0xC4
#define ILI9341_VMCTR1         0xC5
#define ILI9341_VMCTR2         0xC7

#define ILI9341_RDID1          0xDA
#define ILI9341_RDID2          0xDB
#define ILI9341_RDID3          0xDC
#define ILI9341_RDID4          0xDD

#define ILI9341_GMCTRP1        0xE0
#define ILI9341_GMCTRN1        0xE1

#define ILI9341_PWCTR6         0xFC
#define ILI9341_INTERFACE      0xF6   /* Interface control register */

/* Extend register commands */
#define ILI9341_POWERA         0xCB   /* Power control A register */
#define ILI9341_POWERB         0xCF   /* Power control B register */
#define ILI9341_DTCA           0xE8   /* Driver timing control A */
#define ILI9341_DTCB           0xEA   /* Driver timing control B */
#define ILI9341_POWER_SEQ      0xED   /* Power on sequence register */
#define ILI9341_3GAMMA_EN      0xF2   /* 3 Gamma enable register */
#define ILI9341_PRC            0xF7   /* Pump ratio control register */

//-----------------------------------------------------------------------------
#define ILI9341_MAD_RGB        0x08
#define ILI9341_MAD_BGR        0x00

#define ILI9341_MAD_VERTICAL   0x20
#define ILI9341_MAD_X_LEFT     0x00
#define ILI9341_MAD_X_RIGHT    0x40
#define ILI9341_MAD_Y_UP       0x80
#define ILI9341_MAD_Y_DOWN     0x00

#define ILI9341_MAD_COLORMODE  ILI9341_MAD_RGB

#define ILI9341_SIZE_X                     ILI9341_LCD_PIXEL_WIDTH
#define ILI9341_SIZE_Y                     ILI9341_LCD_PIXEL_HEIGHT
#define ILI9341_MAD_DATA_RIGHT_THEN_UP     ILI9341_MAD_COLORMODE | ILI9341_MAD_X_RIGHT | ILI9341_MAD_Y_UP
#define ILI9341_MAD_DATA_RIGHT_THEN_DOWN   ILI9341_MAD_COLORMODE | ILI9341_MAD_X_RIGHT | ILI9341_MAD_Y_DOWN
#define ILI9341_MAD_DATA_RGBMODE           ILI9341_MAD_COLORMODE | ILI9341_MAD_X_LEFT | ILI9341_MAD_Y_DOWN
#define XPOS                               Xpos
#define YPOS                               Ypos
#define XSIZE                              Xsize
#define YSIZE                              Ysize
#define XSTEP                              1
#define YSTEP                              ILI9341_LCD_PIXEL_WIDTH


extern void ili9341_init_lcd(void);
extern void ili9341_write_pixel(uint16_t x, uint16_t y, uint16_t rgb);

#endif /* !__ILI9341_DEF_H__ */
