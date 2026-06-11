#include "world.h"
#include "render.h"
#include "spatial.h"
#include "inventory.h"
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

/* Especies (offset interno) -> model id de render. */
enum { SP_PINE = 0, SP_OAK, SP_BUSH, SP_PALM, SP_DEAD };

static model_id_t species_model(u8 sp)
{
    switch (sp) {
    case SP_OAK:  return MDL_TREE_OAK;
    case SP_BUSH: return MDL_BUSH_BERRY;
    case SP_PALM: return MDL_PALM;
    case SP_DEAD: return MDL_DEAD_TREE;
    default:      return MDL_TREE_PINE;
    }
}

/* Hash 2D estavel para biomas (independe da ordem de geracao). */
static u32 hash2(s32 a, s32 b)
{
    u32 h = (u32)(a * 73856093) ^ (u32)(b * 19349663);
    h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
    return h;
}

biome_t world_biome_at(fx_t x, fx_t z)
{
    s32 mx = FX_TO_INT(x), mz = FX_TO_INT(z);
    /* Faixa de praia perto das bordas do mapa. */
    if (mx >  WORLD_HALF_M - 48 || mx < -(WORLD_HALF_M - 48) ||
        mz >  WORLD_HALF_M - 48 || mz < -(WORLD_HALF_M - 48))
        return BIOME_BEACH;
    /* Celulas de 64 m escolhem o bioma. */
    u32 h = hash2(mx >> 6, mz >> 6) % 100;
    if (h < 45) return BIOME_FOREST;
    if (h < 65) return BIOME_PLAINS;
    if (h < 82) return BIOME_SWAMP;
    return BIOME_ROCKY;
}

void world_generate_forest(u32 seed)
{
    memset(&g_world, 0, sizeof g_world);
    g_world.seed        = seed;
    prng_state          = seed ? seed : 1;
    g_world.world_min_x = FX_FROM_INT(-WORLD_HALF_M);
    g_world.world_min_z = FX_FROM_INT(-WORLD_HALF_M);
    g_world.world_max_x = FX_FROM_INT( WORLD_HALF_M);
    g_world.world_max_z = FX_FROM_INT( WORLD_HALF_M);

    /* ---- arvores: densidade depende do bioma ---- */
    u16 ti = 0;
    for (u16 i = 0; i < MAX_TREES && ti < MAX_TREES; ++i) {
        u32 r1 = prng(), r2 = prng(), r3 = prng();
        fx_t px = FX_FROM_INT(-WORLD_HALF_M) + (fx_t)((r1 % WORLD_SIZE_M) * FX_ONE);
        fx_t pz = FX_FROM_INT(-WORLD_HALF_M) + (fx_t)((r2 % WORLD_SIZE_M) * FX_ONE);
        biome_t b = world_biome_at(px, pz);

        u8 species; u32 chance;
        switch (b) {
        case BIOME_FOREST: species = (r3 % 3 == 0) ? SP_OAK :
                                     (r3 % 3 == 1) ? SP_PINE : SP_BUSH;
                           chance = 90; break;
        case BIOME_PLAINS: species = (r3 & 1) ? SP_BUSH : SP_PINE;
                           chance = 25; break;
        case BIOME_SWAMP:  species = SP_DEAD; chance = 60; break;
        case BIOME_BEACH:  species = SP_PALM; chance = 30; break;
        case BIOME_ROCKY:  species = SP_PINE; chance = 15; break;
        default:           species = SP_PINE; chance = 50; break;
        }
        if ((r3 >> 8) % 100 >= chance) continue;   /* vazio nesse ponto */

        tree_t *t = &g_world.trees[ti++];
        t->pos.x   = px;
        t->pos.z   = pz;
        t->pos.y   = 0;
        t->rot_y   = (u16)(r3 & 0xFFFF);
        t->species = species;
        t->hp      = (species == SP_BUSH) ? 20 :
                     (species == SP_OAK)  ? 120 :
                     (species == SP_DEAD) ? 60  : 80;
    }
    g_world.tree_count = ti;

    /* ---- pedras/minerio: concentradas no bioma ROCKY ---- */
    u16 ri = 0;
    for (u16 i = 0; i < MAX_ROCKS * 4 && ri < MAX_ROCKS; ++i) {
        u32 r1 = prng(), r2 = prng(), r3 = prng();
        fx_t px = FX_FROM_INT(-WORLD_HALF_M) + (fx_t)((r1 % WORLD_SIZE_M) * FX_ONE);
        fx_t pz = FX_FROM_INT(-WORLD_HALF_M) + (fx_t)((r2 % WORLD_SIZE_M) * FX_ONE);
        biome_t b = world_biome_at(px, pz);
        u32 chance = (b == BIOME_ROCKY) ? 80 : (b == BIOME_BEACH) ? 10 : 25;
        if ((r3 >> 4) % 100 >= chance) continue;

        rock_t *rk = &g_world.rocks[ri++];
        rk->pos.x = px; rk->pos.z = pz; rk->pos.y = 0;
        rk->rot_y = (u16)(r3 & 0xFFFF);
        /* ferro e carvao mais comuns no bioma rochoso. */
        u32 k = r3 % 100;
        rk->kind = (b == BIOME_ROCKY)
                   ? (k < 40 ? 1 : k < 60 ? 2 : 0)
                   : (k < 12 ? 1 : k < 20 ? 2 : 0);
        rk->hp   = (rk->kind == 0) ? 120 : 160;
    }
    g_world.rock_count = ri;

    /* ---- baus de loot espalhados ---- */
    static const u8 loot_table[] = {
        ITEM_CANNED_FOOD, ITEM_BANDAGE, ITEM_NAIL, ITEM_SCRAP_METAL,
        ITEM_AMMO_9MM, ITEM_CLOTH, ITEM_ROPE, ITEM_ANTIBIOTIC,
        ITEM_IRON_INGOT, ITEM_GUNPOWDER, ITEM_AMMO_SHELL, ITEM_MEDKIT,
    };
    for (u16 i = 0; i < MAX_LOOT; ++i) {
        u32 r1 = prng(), r2 = prng(), r3 = prng();
        loot_t *lt = &g_world.loot[i];
        lt->active = 1; lt->opened = 0;
        lt->pos.x  = FX_FROM_INT(-WORLD_HALF_M) + (fx_t)((r1 % WORLD_SIZE_M) * FX_ONE);
        lt->pos.z  = FX_FROM_INT(-WORLD_HALF_M) + (fx_t)((r2 % WORLD_SIZE_M) * FX_ONE);
        lt->item   = loot_table[r3 % (sizeof loot_table)];
        lt->count  = 1 + (r3 >> 8) % 4;
    }
    g_world.loot_count = MAX_LOOT;
}

void world_tick(u32 dt_ms)
{
    (void)dt_ms;
    for (u16 i = 0; i < g_world.tree_count; ++i) {
        if (!g_world.trees[i].hp) continue;
        spatial_insert(ENT_TREE, i,
                       g_world.trees[i].pos.x, g_world.trees[i].pos.z);
    }
    for (u16 i = 0; i < g_world.rock_count; ++i) {
        if (!g_world.rocks[i].hp) continue;
        /* pedras entram no grid como ENT_TREE p/ raycast reaproveitar a
         * mesma query — diferenciadas pelo id range no consumidor. */
        spatial_insert(ENT_TREE, (u16)(MAX_TREES + i),
                       g_world.rocks[i].pos.x, g_world.rocks[i].pos.z);
    }
}

void world_render(void)
{
    draw_cmd_t cmd;
    cmd.lod = 0; cmd.flags = 0;
    for (u16 i = 0; i < g_world.tree_count; ++i) {
        const tree_t *t = &g_world.trees[i];
        if (!t->hp) continue;
        cmd.model = species_model(t->species);
        cmd.pos   = t->pos;
        cmd.rot_y = t->rot_y;
        render_submit(&cmd);
    }
    for (u16 i = 0; i < g_world.rock_count; ++i) {
        const rock_t *r = &g_world.rocks[i];
        if (!r->hp) continue;
        cmd.model = MDL_ROCK;
        cmd.pos   = r->pos;
        cmd.rot_y = r->rot_y;
        render_submit(&cmd);
    }
    for (u16 i = 0; i < g_world.loot_count; ++i) {
        const loot_t *l = &g_world.loot[i];
        if (!l->active || l->opened) continue;
        cmd.model = MDL_LOOT;
        cmd.pos   = l->pos;
        cmd.rot_y = 0;
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
        (void)by_player;
    } else {
        t->hp -= dmg;
    }
}

void world_damage_rock(u16 rock_id, u8 dmg, u8 by_player)
{
    if (rock_id >= g_world.rock_count) return;
    rock_t *r = &g_world.rocks[rock_id];
    if (!r->hp) return;
    (void)by_player;
    if (r->hp <= dmg) r->hp = 0;
    else              r->hp -= dmg;
}
