/* main.c — entry do EE.
 *
 * Fluxo de boot:
 *   1. memory_boot   : reserva zonas estatica/level/frame
 *   2. gs_boot       : inicializa GS + framebuffer duplo
 *   3. input_boot    : carrega modulos IOP do pad e inicia libpad
 *   4. audio_boot    : carrega libsd e modulos de som no IOP
 *   5. render_boot   : carrega microcode VU1, sobe atlas de texturas
 *   6. world_generate: gera floresta deterministica a partir do seed
 *   7. net_boot      : abre socket UDP via ps2ip
 *   8. loop principal — tick fixo de 50 Hz (PAL 1/50, NTSC arredondado)
 */

#include <kernel.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "memory.h"
#include "scratchpad.h"
#include "gs.h"
#include "render.h"
#include "world.h"
#include "player.h"
#include "zombie.h"
#include "spatial.h"
#include "network.h"
#include "input.h"
#include "audio.h"

#define TICK_HZ      50
#define TICK_MS      (1000 / TICK_HZ)

static int parse_args(int argc, char **argv, int *as_host, char **host_ip)
{
    *as_host = 1;
    *host_ip = "192.168.1.10";
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--client") && i + 1 < argc) {
            *as_host = 0;
            *host_ip = argv[++i];
        } else if (!strcmp(argv[i], "--host")) {
            *as_host = 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int  as_host;
    char *host_ip;

    parse_args(argc, argv, &as_host, &host_ip);

    printf("zumbi: boot (%s)\n", as_host ? "HOST" : "CLIENT");

    memory_boot();
    gs_boot(GS_INTERLACED);
    input_boot();
    audio_boot();
    render_boot();

    world_generate_forest(0xC0FFEE42);
    player_boot();
    zombie_boot();
    net_boot(as_host, host_ip);

    /* Spawn inicial: 12 zumbis distribuidos. */
    if (as_host) {
        for (int i = 0; i < 12; ++i) {
            fx_t x = FX_FROM_INT(-64 + (i * 17) % 128);
            fx_t z = FX_FROM_INT(-64 + (i * 23) % 128);
            zombie_spawn(x, z);
        }
    }

    u32 tick = 0;
    u32 acc  = 0;
    u32 last_ms = 0;   /* preenchido por timer real do ps2sdk */

    /* Loop principal — fixed-step com render variavel. */
    for (;;) {
        u32 now_ms = 0;          /* TODO: GetTimerTicks/Ticks2Microsec */
        u32 dt     = now_ms - last_ms;
        last_ms    = now_ms;
        acc       += dt;

        input_tick();

        while (acc >= TICK_MS) {
            spatial_reset();
            world_tick(TICK_MS);
            zombie_tick(TICK_MS);
            player_tick(TICK_MS);
            net_tick(tick);
            acc  -= TICK_MS;
            tick += 1;
        }

        /* Render usa o estado mais recente — interpola se quiser
         * suavidade entre ticks (TODO: lerp pos/yaw). */
        gs_frame_begin();
        {
            mat4_t view, proj;
            extern void player_camera(mat4_t *view, mat4_t *proj);
            player_camera(&view, &proj);
            render_begin(&view, &proj);
            world_render();
            zombie_render();
            player_render();
            render_flush();
        }
        gs_frame_end();
    }

    /* unreachable */
    return 0;
}
