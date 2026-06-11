/* render.c — pipeline EE -> VU1 -> GS.
 *
 * Estrategia:
 *  - EE faz cull por bounding sphere e LOD por distancia.
 *  - Comandos sao agrupados em "lotes" por (model_id, lod).
 *  - Para cada lote, montamos uma matriz por instancia e enviamos
 *    via DMA chain um pacote VIF que:
 *      (a) carrega o microprograma VU1 (apenas no primeiro frame)
 *      (b) UNPACK matrizes para VU_MEM
 *      (c) UNPACK vertices do modelo
 *      (d) MSCAL endereco do entry point — VU1 transforma e emite
 *          um GIF packet direto para o GS.
 *  - Z buffer no GS resolve oclusao entre lotes.
 *
 * Tudo escrito a frio aqui — em producao se usaria libpacket/libdma do
 * ps2sdk para construir as listas. */

#include "render.h"
#include "memory.h"
#include "scratchpad.h"
#include "gs.h"
#include <string.h>
#include <stdio.h>

render_stats_t g_rstats;

#define BATCH_MAX     64
#define CMD_MAX       512

static draw_cmd_t   s_cmds[CMD_MAX];
static u32          s_cmd_count;
static mat4_t       s_view, s_proj, s_view_proj;
static frustum_t    s_frustum;

/* Microcode VU1 referenciado por simbolo (linkado pelo Makefile). */
extern u32 VU1Transform_CodeStart __attribute__((section(".vu1code")));
extern u32 VU1Transform_CodeEnd   __attribute__((section(".vu1code")));

static int s_vu1_loaded = 0;

static void vu1_upload_code(void)
{
    u32 *start = &VU1Transform_CodeStart;
    u32 *end   = &VU1Transform_CodeEnd;
    u32  qwc   = ((u8 *)end - (u8 *)start + 15) >> 4;
    /* UNPACK V4_32 para VU_MEM em 0 — usaria VIFcode MPG (load microprog) */
    dma_channel_send_normal(DMA_CHANNEL_VIF1, start, qwc, 0, 0);
    dma_channel_wait(DMA_CHANNEL_VIF1, 0);
    s_vu1_loaded = 1;
}

/* Modelos sao carregados em static zone no boot. */
static model_t s_models[MDL_COUNT];

static void model_load(model_id_t id, const char *path)
{
    /* TODO real: parse de .mdl quantizado. Aqui registramos placeholder
     * com bbox conhecido para nao quebrar o resto do pipeline. */
    (void)path;
    s_models[id].verts  = zone_alloc(&g_zone_static, 4 * sizeof(vtx16_t), 16);
    s_models[id].tris   = zone_alloc(&g_zone_static, 2 * sizeof(tri16_t), 16);
    s_models[id].nverts = 4;
    s_models[id].ntris  = 2;
    s_models[id].scale  = 0.01f;
    s_models[id].tex_id = id;
    s_models[id].radius = 2.0f;
}

void render_boot(void)
{
    model_load(MDL_TREE_PINE, "assets/tree_pine.mdl");
    model_load(MDL_TREE_OAK,  "assets/tree_oak.mdl");
    model_load(MDL_TREE_BUSH, "assets/bush.mdl");
    model_load(MDL_PLAYER,    "assets/player.mdl");
    model_load(MDL_ZOMBIE,    "assets/zombie.mdl");
    model_load(MDL_ROCK,      "assets/rock.mdl");
    model_load(MDL_CRATE,     "assets/crate.mdl");
    model_load(MDL_PALM,       "assets/palm.mdl");
    model_load(MDL_DEAD_TREE,  "assets/dead_tree.mdl");
    model_load(MDL_BUSH_BERRY, "assets/bush_berry.mdl");
    model_load(MDL_STRUCTURE,  "assets/structure.mdl");
    model_load(MDL_CAMPFIRE,   "assets/campfire.mdl");
    model_load(MDL_ANIMAL,     "assets/animal.mdl");
    model_load(MDL_PROJECTILE, "assets/projectile.mdl");
    model_load(MDL_LOOT,       "assets/loot.mdl");
    model_load(MDL_WATER,      "assets/water.mdl");
}

void render_begin(const mat4_t *view, const mat4_t *proj)
{
    s_view = *view;
    s_proj = *proj;
    mat4_mul(&s_view_proj, proj, view);
    frustum_extract(&s_frustum, &s_view_proj);
    s_cmd_count = 0;
    memset(&g_rstats, 0, sizeof g_rstats);

    if (!s_vu1_loaded) vu1_upload_code();
}

void render_submit(const draw_cmd_t *cmd)
{
    if (s_cmd_count >= CMD_MAX) return;

    const model_t *m = &s_models[cmd->model];
    float x = FX_TO_FLOAT(cmd->pos.x);
    float y = FX_TO_FLOAT(cmd->pos.y);
    float z = FX_TO_FLOAT(cmd->pos.z);

    if (!frustum_test_sphere(&s_frustum, x, y, z, m->radius)) {
        g_rstats.culled++;
        return;
    }

    s_cmds[s_cmd_count++] = *cmd;
}

/* Compara cmds por modelo entao por LOD — radix-like simples. */
static int cmd_key(const draw_cmd_t *c)
{
    return ((int)c->model << 8) | c->lod;
}

static void cmds_sort(void)
{
    /* insertion sort — N pequeno, listas quase-ordenadas em pratica. */
    for (u32 i = 1; i < s_cmd_count; ++i) {
        draw_cmd_t t = s_cmds[i];
        int k = cmd_key(&t);
        u32 j = i;
        while (j > 0 && cmd_key(&s_cmds[j-1]) > k) {
            s_cmds[j] = s_cmds[j-1]; j--;
        }
        s_cmds[j] = t;
    }
}

static void render_batch(u32 first, u32 count)
{
    const draw_cmd_t *head = &s_cmds[first];
    const model_t    *m    = &s_models[head->model];

    /* Monta matrizes World * View * Proj em scratchpad para o VU1.
     * Layout: [VP|4qw][N x World|4qw cada] */
    qword_t *spr = SPR(SPR_OFS_MATRIX);
    memcpy(spr, &s_view_proj, sizeof(mat4_t));
    qword_t *mw  = spr + 4;

    for (u32 i = 0; i < count && i < BATCH_MAX; ++i) {
        const draw_cmd_t *c = &head[i];
        mat4_t world;
        mat4_rotate_y(&world, (float)c->rot_y * (2.0f * PI_F / 65536.0f));
        world.m[12] = FX_TO_FLOAT(c->pos.x);
        world.m[13] = FX_TO_FLOAT(c->pos.y);
        world.m[14] = FX_TO_FLOAT(c->pos.z);
        memcpy(&mw[i * 4], &world, sizeof(mat4_t));
    }

    /* TODO: construir VIF list real (STCYCL, UNPACK V4_32, MSCAL).
     * gskit nao expoe isso, entao em producao se monta a mao com
     * macros do dma.h + libpacket. */

    g_rstats.draw_calls++;
    g_rstats.vertices  += m->nverts * count;
    g_rstats.triangles += m->ntris  * count;
}

void render_flush(void)
{
    cmds_sort();
    u32 i = 0;
    while (i < s_cmd_count) {
        u32 j = i + 1;
        int k = cmd_key(&s_cmds[i]);
        while (j < s_cmd_count && cmd_key(&s_cmds[j]) == k &&
               (j - i) < BATCH_MAX) j++;
        render_batch(i, j - i);
        i = j;
    }
}
