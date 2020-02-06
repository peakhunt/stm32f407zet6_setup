#ifndef __XPT2046_DEF_H__
#define __XPT2046_DEF_H__

#include <stdint.h>

#define XPT2046_ADC_MAX       0xfffU
#define XPT2046_CAL_OFFSET    20

#define XPT2046_DF_CH_Y       0x10
#define XPT2046_DF_CH_X       0x50

// 12-bit, DFR, Power Down enabled
#define XPT2046_DFR_CMD(ch)   (0x80 | ch)

// 12-bit, DFR, Power Down disabled
//#define XPT2046_DFR_CMD(ch)   (0x80 | ch | 0x01)


extern void xpt2046_calc_point(uint16_t adc_x, uint16_t adc_y, uint16_t* px, uint16_t* py);
extern void xpt2046_set_calibration(uint16_t adc_x0,uint16_t adc_y0, uint16_t adc_x1, uint16_t adc_y1);
extern void xpt2046_read(uint8_t cmd, uint16_t* d);
extern void xpt2046_init(uint16_t width, uint16_t height);

#endif /* !__XPT2046_DEF_H__ */
