#ifndef ZUMBI_INPUT_H
#define ZUMBI_INPUT_H

#include "types.h"

#define BTN_UP        (1 << 0)
#define BTN_DOWN      (1 << 1)
#define BTN_LEFT      (1 << 2)
#define BTN_RIGHT     (1 << 3)
#define BTN_CROSS     (1 << 4)
#define BTN_CIRCLE    (1 << 5)
#define BTN_SQUARE    (1 << 6)
#define BTN_TRIANGLE  (1 << 7)
#define BTN_L1        (1 << 8)
#define BTN_R1        (1 << 9)
#define BTN_L2        (1 << 10)
#define BTN_R2        (1 << 11)
#define BTN_SELECT    (1 << 12)
#define BTN_START     (1 << 13)
#define BTN_L3        (1 << 14)
#define BTN_R3        (1 << 15)

typedef struct {
    u32  buttons;       /* held now */
    u32  pressed;       /* edge: pressionado este frame */
    u32  released;
    s8   lstick_x, lstick_y;
    s8   rstick_x, rstick_y;
} input_state_t;

extern input_state_t g_input;

void input_boot(void);
void input_tick(void);

#endif
