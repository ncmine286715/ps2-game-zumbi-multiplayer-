#include "input.h"
#include <libpad.h>
#include <string.h>

input_state_t g_input;
static char  s_pad_buf[256] __attribute__((aligned(64)));
static u32   s_prev_buttons;

void input_boot(void)
{
    padInit(0);
    padPortOpen(0, 0, s_pad_buf);
    padSetMainMode(0, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
}

/* Mapeia PS2 PAD bits para nossos BTN_*. */
static u32 map_buttons(u16 ps2)
{
    u32 r = 0;
    if (!(ps2 & PAD_UP))       r |= BTN_UP;
    if (!(ps2 & PAD_DOWN))     r |= BTN_DOWN;
    if (!(ps2 & PAD_LEFT))     r |= BTN_LEFT;
    if (!(ps2 & PAD_RIGHT))    r |= BTN_RIGHT;
    if (!(ps2 & PAD_CROSS))    r |= BTN_CROSS;
    if (!(ps2 & PAD_CIRCLE))   r |= BTN_CIRCLE;
    if (!(ps2 & PAD_SQUARE))   r |= BTN_SQUARE;
    if (!(ps2 & PAD_TRIANGLE)) r |= BTN_TRIANGLE;
    if (!(ps2 & PAD_L1))       r |= BTN_L1;
    if (!(ps2 & PAD_R1))       r |= BTN_R1;
    if (!(ps2 & PAD_L2))       r |= BTN_L2;
    if (!(ps2 & PAD_R2))       r |= BTN_R2;
    if (!(ps2 & PAD_SELECT))   r |= BTN_SELECT;
    if (!(ps2 & PAD_START))    r |= BTN_START;
    if (!(ps2 & PAD_L3))       r |= BTN_L3;
    if (!(ps2 & PAD_R3))       r |= BTN_R3;
    return r;
}

void input_tick(void)
{
    struct padButtonStatus st;
    int ready = padGetState(0, 0);
    if (ready != PAD_STATE_STABLE && ready != PAD_STATE_FINDCTP1) return;
    if (padRead(0, 0, &st) == 0) return;

    u32 b = map_buttons(st.btns);
    g_input.pressed  = b & ~s_prev_buttons;
    g_input.released = ~b & s_prev_buttons;
    g_input.buttons  = b;
    s_prev_buttons   = b;

    /* sticks: PS2 manda 0..255, centro = 128. Converte para -128..127. */
    g_input.lstick_x = (s8)((int)st.ljoy_h - 128);
    g_input.lstick_y = (s8)(128 - (int)st.ljoy_v);
    g_input.rstick_x = (s8)((int)st.rjoy_h - 128);
    g_input.rstick_y = (s8)(128 - (int)st.rjoy_v);

    /* Dead zone radial. */
    if (g_input.lstick_x < 24 && g_input.lstick_x > -24) g_input.lstick_x = 0;
    if (g_input.lstick_y < 24 && g_input.lstick_y > -24) g_input.lstick_y = 0;
    if (g_input.rstick_x < 16 && g_input.rstick_x > -16) g_input.rstick_x = 0;
    if (g_input.rstick_y < 16 && g_input.rstick_y > -16) g_input.rstick_y = 0;
}
