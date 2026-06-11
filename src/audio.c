/* audio.c — fila para o IOP via SIF RPC.
 *
 * O EE so registra "tocar sfx X a partir do mundo (wx, wz)". O modulo
 * IOP (iop/audio_iop.c) mantem buffers PCM em SPU2 RAM e faz panning
 * + atenuacao por distancia em CPU IOP, liberando o EE. */

#include "audio.h"
#include <string.h>

typedef struct {
    u8  sfx;
    u8  _pad;
    s16 wx, wz;
} QALIGN audio_msg_t;

static fx_t s_lx, s_lz;
static u16  s_lyaw;

void audio_boot(void)
{
    /* TODO: SifLoadModule("rom0:LIBSD") + RPC bind do modulo customizado. */
}

void audio_play(sfx_id_t id, fx_t wx, fx_t wz)
{
    audio_msg_t m;
    m.sfx = (u8)id;
    m.wx  = (s16)FX_TO_INT(wx);
    m.wz  = (s16)FX_TO_INT(wz);
    /* TODO: SifCallRpc(audio_rpc_handle, FN_PLAY, ..., &m, sizeof m, ...) */
    (void)m;
}

void audio_set_listener(fx_t x, fx_t z, u16 yaw)
{
    s_lx = x; s_lz = z; s_lyaw = yaw;
}
