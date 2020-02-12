#ifndef __TOUCH_SCREEN_DEF_H__
#define __TOUCH_SCREEN_DEF_H__

typedef enum
{
  touch_screen_calibration_step_ul = 0,
  touch_screen_calibration_step_ur,
  touch_screen_calibration_step_bl,
  touch_screen_calibration_step_br,
  touch_screen_calibration_step_done,
} touch_screen_calibration_step_t;

typedef void (*touch_screen_calibration_callback)(touch_screen_calibration_step_t step, void* arg);

extern void touch_screen_init(void);
extern void touch_screen_start_calibration(touch_screen_calibration_callback cb, void* cb_arg);

#endif /* !__TOUCH_SCREEN_DEF_H__ */
