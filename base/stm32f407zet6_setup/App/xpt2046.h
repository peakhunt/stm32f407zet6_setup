#ifndef __XPT2046_DEF_H__
#define __XPT2046_DEF_H__

#include <stdint.h>

#define XP2046_ADC_MAX        0xfffU
#define XP2046_CAL_OFFSET     20

extern void xpt2046_calc_point(uint16_t adc_x, uint16_t adc_y, uint16_t* px, uint16_t* py);
extern void xpt2046_set_calibration(uint16_t adc_x0,uint16_t adc_y0, uint16_t adc_x1, uint16_t adc_y1);
extern void xpt2046_init(uint16_t width, uint16_t height);

#endif /* !__XPT2046_DEF_H__ */
