#include "zombie.h"
#include "player.h"
#include "render.h"
#include "spatial.h"
#include "audio.h"
#include <string.h>
#include <math.h>

zombie_t g_zombies[MAX_ZOMBIES];

#define FF_DIM   32
static s8 s_flowfield_x[FF_DIM][FF_DIM] QALIGN;
static s8 s_flowfield_z[FF_DIM][FF_DIM] QALIGN;
static u8 s_ff_dirty = 1;

static int s_next_zid = 0;

void zombie_boot(void)
{
    memset(g_zombies, 0, sizeof g_zombies);
    memset(s_flowfield_x, 0, sizeof s_flowfield_x);
    memset(s_flowfield_z, 0, sizeof s_flowfield_z);
    s_next_zid = 0;
}

void zombie_spawn(fx_t x, fx_t z)
{
    for (int i = 0; i < MAX_ZOMBIES; ++i) {
        if (g_zombies[i].active) continue;
        zombie_t *z0 = &g_zombies[i];
        memset(z0, 0, sizeof *z0);
        z0->active        = 1;
        z0->state         = ZSTATE_WANDER;
        z0->target_player = 0xFF;
        z0->pos.x         = x;
        z0->pos.z         = z;
        z0->hp            = 60;
        z0->sense_radius  = FX_FROM_INT(20);
        z0->attack_radius = FX_FROM_INT(2);
        s_next_zid++;
        return;
    }
}

/* Flowfield BFS-like a partir dos players. Atualizado a cada 4 ticks. */
void zombie_flowfield_update(void)
{
    /* Reseta — direcoes 0 significa parado/idle. */
    memset(s_flowfield_x, 0, sizeof s_flowfield_x);
    memset(s_flowfield_z, 0, sizeof s_flowfield_z);

    /* Para cada player, marca direcao do gradiente "ate ele" nas
     * celulas em volta (raio de 12). Simplificacao: campo radial
     * aproximado, suficiente para floresta aberta. */
    extern player_t g_players[MAX_PLAYERS];
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (!g_players[p].active) continue;
        int pcx = (FX_TO_INT(g_players[p].pos.x) + FF_DIM * 8 / 2) >> 3;
        int pcz = (FX_TO_INT(g_players[p].pos.z) + FF_DIM * 8 / 2) >> 3;
        const int R = 12;
        for (int dz = -R; dz <= R; ++dz)
        for (int dx = -R; dx <= R; ++dx) {
            int gx = pcx + dx, gz = pcz + dz;
            if (gx < 0 || gx >= FF_DIM || gz < 0 || gz >= FF_DIM) continue;
            int d2 = dx*dx + dz*dz;
            if (!d2 || d2 > R*R) continue;
            float len = sqrtf((float)d2);
            s_flowfield_x[gz][gx] = (s8)(-dx * 127 / (int)len);
            s_flowfield_z[gz][gx] = (s8)(-dz * 127 / (int)len);
        }
    }
    s_ff_dirty = 0;
}

static int s_tick_count = 0;

void zombie_tick(u32 dt_ms)
{
    if ((s_tick_count++ & 3) == 0 || s_ff_dirty)
        zombie_flowfield_update();

    extern player_t g_players[MAX_PLAYERS];

    for (int i = 0; i < MAX_ZOMBIES; ++i) {
        zombie_t *z = &g_zombies[i];
        if (!z->active) continue;

        /* Acha player mais proximo dentro do sense radius. */
        int    best = -1;
        fx_t   best_d2 = z->sense_radius;
        best_d2 = FX_MUL(best_d2, best_d2);
        for (int pi = 0; pi < MAX_PLAYERS; ++pi) {
            const player_t *p = &g_players[pi];
            if (!p->active || p->state == PSTATE_DEAD) continue;
            fx_t dx = p->pos.x - z->pos.x;
            fx_t dz = p->pos.z - z->pos.z;
            fx_t d2 = FX_MUL(dx, dx) + FX_MUL(dz, dz);
            if (d2 < best_d2) { best_d2 = d2; best = pi; }
        }
        z->target_player = (best < 0) ? 0xFF : (u8)best;

        switch (z->state) {
        case ZSTATE_WANDER:
            if (best >= 0) z->state = ZSTATE_CHASE;
            else {
                /* drift lento */
                z->vel.x = (i & 1) ? FX_FROM_FLOAT( 0.02f) : FX_FROM_FLOAT(-0.02f);
                z->vel.z = (i & 2) ? FX_FROM_FLOAT( 0.02f) : FX_FROM_FLOAT(-0.02f);
            }
            break;
        case ZSTATE_CHASE: {
            if (best < 0) { z->state = ZSTATE_WANDER; break; }
            /* Direcao do flowfield. */
            int cx = (FX_TO_INT(z->pos.x) + FF_DIM*8/2) >> 3;
            int cz = (FX_TO_INT(z->pos.z) + FF_DIM*8/2) >> 3;
            if (cx >= 0 && cx < FF_DIM && cz >= 0 && cz < FF_DIM) {
                fx_t vx = FX_FROM_FLOAT((float)s_flowfield_x[cz][cx] / 127.0f);
                fx_t vz = FX_FROM_FLOAT((float)s_flowfield_z[cz][cx] / 127.0f);
                z->vel.x = FX_MUL(vx, FX_FROM_FLOAT(0.07f));
                z->vel.z = FX_MUL(vz, FX_FROM_FLOAT(0.07f));
            }
            fx_t ar2 = FX_MUL(z->attack_radius, z->attack_radius);
            if (best_d2 < ar2) z->state = ZSTATE_ATTACK;
        } break;
        case ZSTATE_ATTACK: {
            z->vel.x = z->vel.z = 0;
            /* Aplica dano periodico no player alvo. */
            if (best >= 0) {
                static u32 atk_cd = 0;
                atk_cd += dt_ms;
                if (atk_cd >= 800) {
                    g_players[best].hp -= 8;
                    audio_play(SFX_ZOMBIE_GROAN, z->pos.x, z->pos.z);
                    atk_cd = 0;
                }
                fx_t ar2 = FX_MUL(z->attack_radius, z->attack_radius);
                if (best_d2 > FX_MUL(ar2, FX_FROM_FLOAT(1.5f)))
                    z->state = ZSTATE_CHASE;
            } else z->state = ZSTATE_WANDER;
        } break;
        default: break;
        }

        z->pos.x += z->vel.x;
        z->pos.z += z->vel.z;

        /* Update yaw para olhar na direcao do movimento. */
        if (z->vel.x || z->vel.z) {
            float a = atan2f(FX_TO_FLOAT(z->vel.x), FX_TO_FLOAT(z->vel.z));
            z->yaw = (u16)(a * 65536.0f / (2.0f * PI_F));
        }

        spatial_insert(ENT_ZOMBIE, (u16)i, z->pos.x, z->pos.z);
    }
}

void zombie_render(void)
{
    draw_cmd_t c = {0};
    for (int i = 0; i < MAX_ZOMBIES; ++i) {
        const zombie_t *z = &g_zombies[i];
        if (!z->active) continue;
        c.model = MDL_ZOMBIE;
        c.pos   = z->pos;
        c.rot_y = z->yaw;
        c.flags = z->anim_frame;
        render_submit(&c);
    }
}
