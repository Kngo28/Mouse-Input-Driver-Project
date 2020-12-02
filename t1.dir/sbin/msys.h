#ifndef _MSYS_H_
#define _MSYS_H_

#include "stdint.h"

/* mouse behaviors */

extern void middle_click_on(void);

extern void middle_click_off(void);

extern void right_click_on(void);

extern void right_click_off(void);

extern void left_click_on(void);

extern void left_click_off(void);

extern void move_mouse(int xm, int ym);

/*
 * used for checking the state of mouse
 * basically the same as mouse packet:
 * byte 0 represents yo xo ys xs ao bm br bl (does not need yo, xo, ao)
 * byte 1 represents xm
 * byte 2 represents ym
 * byte 3 is garbage
 */
extern uint32_t show_mouse(void);

#endif
