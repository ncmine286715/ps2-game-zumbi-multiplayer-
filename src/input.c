#include "input.h"
#include <libpad.h>
#include <string.h>

input_state_t g_input;                          /* alias do pad 0 */
input_state_t g_inputs[MAX_LOCAL_PLAYERS];

/* Mapeamento (porta, slot) por indice de jogador local. Com multitap,
 * uma unica porta expoe varios slots; sem multitap usamos as 2 portas
 * fisicas do console. */
static const u8 s_pad_port[MAX_LOCAL_PLAYERS] = { 0, 1, 0, 1 };
static const u8 s_pad_slot[MAX_LOCAL_PLAYERS] = { 0, 0, 1, 1 };

static char s_pad_buf[MAX_LOCAL_PLAYERS][256] __attribute__((aligned(64)));
static u32  s_prev_buttons[MAX_LOCAL_PLAYERS];
static u8   s_pad_ok[MAX_LOCAL_PLAYERS];

void input_boot(void)
{
    padInit(0);
    memset(g_inputs, 0, sizeof g_inputs);
    for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i) {
        int port = s_pad_port[i], slot = s_pad_slot[i];
        if (padPortOpen(port, slot, s_pad_buf[i]) == 1) {
            padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
            s_pad_ok[i] = 1;
        } else {
            s_pad_ok[i] = 0;
        }
    }
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

static void read_pad(int i)
{
    input_state_t *in = &g_inputs[i];
    if (!s_pad_ok[i]) { memset(in, 0, sizeof *in); return; }

    int port = s_pad_port[i], slot = s_pad_slot[i];
    struct padButtonStatus st;
    int ready = padGetState(port, slot);
    if (ready != PAD_STATE_STABLE && ready != PAD_STATE_FINDCTP1) return;
    if (padRead(port, slot, &st) == 0) return;

    u32 b = map_buttons(st.btns);
    in->pressed  = b & ~s_prev_buttons[i];
    in->released = ~b & s_prev_buttons[i];
    in->buttons  = b;
    s_prev_buttons[i] = b;

    in->lstick_x = (s8)((int)st.ljoy_h - 128);
    in->lstick_y = (s8)(128 - (int)st.ljoy_v);
    in->rstick_x = (s8)((int)st.rjoy_h - 128);
    in->rstick_y = (s8)(128 - (int)st.rjoy_v);

    if (in->lstick_x < 24 && in->lstick_x > -24) in->lstick_x = 0;
    if (in->lstick_y < 24 && in->lstick_y > -24) in->lstick_y = 0;
    if (in->rstick_x < 16 && in->rstick_x > -16) in->rstick_x = 0;
    if (in->rstick_y < 16 && in->rstick_y > -16) in->rstick_y = 0;
}

void input_tick(void)
{
    for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
        read_pad(i);
    g_input = g_inputs[0];     /* mantem o alias compativel */
}

const input_state_t *input_for_pad(u8 pad)
{
    if (pad >= MAX_LOCAL_PLAYERS) pad = 0;
    return &g_inputs[pad];
}

int input_pad_connected(u8 pad)
{
    return (pad < MAX_LOCAL_PLAYERS) && s_pad_ok[pad];
}
