#include "world.h"
#include "render.h"
#include "spatial.h"
#include <string.h>

world_t g_world;

/* PRNG deterministico simples (xorshift32) — mesmo seed produz o
 * mesmo terreno em todas as maquinas, eliminando sync de geometria. */
static u32 prng_state;
static u32 prng(void)
{
    u32 x = prng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    prng_state = x; return x;
}

void world_generate_forest(u32 seed)
{
    memset(&g_world, 0, sizeof g_world);
    g_world.seed       = seed;
    prng_state         = seed ? seed : 1;
    g_world.world_min_x = FX_FROM_INT(-128);
    g_world.world_min_z = FX_FROM_INT(-128);
    g_world.world_max_x = FX_FROM_INT( 128);
    g_world.world_max_z = FX_FROM_INT( 128);

    for (u16 i = 0; i < MAX_TREES; ++i) {
        tree_t *t = &g_world.trees[i];
        u32 r1 = prng(), r2 = prng(), r3 = prng();
        t->pos.x   = FX_FROM_INT(-128) + (fx_t)(r1 % 256 * FX_ONE);
        t->pos.z   = FX_FROM_INT(-128) + (fx_t)(r2 % 256 * FX_ONE);
        t->pos.y   = 0;
        t->rot_y   = (u16)(r3 & 0xFFFF);
        t->species = (r3 >> 16) % 3;        /* PINE/OAK/BUSH */
        t->hp      = (t->species == MDL_TREE_BUSH) ? 20 :
                     (t->species == MDL_TREE_OAK)  ? 120 : 80;
    }
    g_world.tree_count = MAX_TREES;
}

void world_tick(u32 dt_ms)
{
    /* Re-insere arvores no grid (arvores nao se movem, mas o reset
     * acontece a cada tick para que players/zumbis tenham vizinhos).
     * Otimizacao: poderiamos manter um snapshot fixo de arvores num
     * grid separado para evitar re-insert toda hora. */
    (void)dt_ms;
    for (u16 i = 0; i < g_world.tree_count; ++i) {
        if (!g_world.trees[i].hp) continue;
        spatial_insert(ENT_TREE, i,
                       g_world.trees[i].pos.x, g_world.trees[i].pos.z);
    }
}

void world_render(void)
{
    extern player_t g_players[MAX_PLAYERS];
    extern u8       g_local_player_idx;
    (void)g_players; (void)g_local_player_idx;

    /* TODO: usar o grid + posicao do player para iterar so quem esta
     * proximo. Por ora submetemos todos — frustum cull filtra. */
    draw_cmd_t cmd;
    for (u16 i = 0; i < g_world.tree_count; ++i) {
        const tree_t *t = &g_world.trees[i];
        if (!t->hp) continue;
        cmd.model = (model_id_t)(MDL_TREE_PINE + t->species);
        cmd.pos   = t->pos;
        cmd.rot_y = t->rot_y;
        cmd.lod   = 0;
        cmd.flags = 0;
        render_submit(&cmd);
    }
}

void world_damage_tree(u16 tree_id, u8 dmg, u8 by_player)
{
    if (tree_id >= g_world.tree_count) return;
    tree_t *t = &g_world.trees[tree_id];
    if (!t->hp) return;
    if (t->hp <= dmg) {
        t->hp = 0;
        /* Drop ate o jogador (network event PKT_EVENT). */
        (void)by_player;
    } else {
        t->hp -= dmg;
    }
}
