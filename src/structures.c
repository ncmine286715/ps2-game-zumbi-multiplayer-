/* structures.c — construcoes colocaveis e estacoes de craft.
 *
 * Pool fixo (zero alocacao). Estacoes (bancada/forja/fogueira) sao
 * consultadas por structures_near() para habilitar receitas e cozinhar.
 * Fogueiras/forjas/lampadas tem estado "lit" e consomem combustivel;
 * canteiros (ST_FARM_PLOT) avancam "growth" com o tempo. */

#include "structures.h"
#include "render.h"
#include "spatial.h"
#include "inventory.h"
#include <string.h>

structure_t g_structures[MAX_STRUCTURES];
u16         g_structure_count;

/* hp inicial por tipo (espelha durability do item, mas em s16). */
static const s16 s_type_hp[ST_COUNT] = {
    [ST_CAMPFIRE]    = 80,
    [ST_FOUNDATION]  = 400,
    [ST_FLOOR]       = 200,
    [ST_WALL_WOOD]   = 200,
    [ST_WALL_STONE]  = 500,
    [ST_DOOR]        = 180,
    [ST_WINDOW]      = 120,
    [ST_ROOF]        = 160,
    [ST_STAIRS]      = 180,
    [ST_FENCE]       = 120,
    [ST_GATE]        = 200,
    [ST_WORKBENCH]   = 150,
    [ST_FORGE]       = 250,
    [ST_FURNACE]     = 250,
    [ST_STORAGE_BOX] = 150,
    [ST_BED]         = 100,
    [ST_WATER_BARREL]= 120,
    [ST_FARM_PLOT]   = 80,
    [ST_TRAP_SPIKE]  = 60,
    [ST_WATCHTOWER]  = 400,
    [ST_GENERATOR]   = 200,
    [ST_LAMP_POST]   = 120,
    [ST_TENT]        = 100,
};

structure_type_t structure_type_of_item(item_id_t item)
{
    switch (item) {
    case ITEM_CAMPFIRE:    return ST_CAMPFIRE;
    case ITEM_FOUNDATION:  return ST_FOUNDATION;
    case ITEM_FLOOR_WOOD:  return ST_FLOOR;
    case ITEM_WALL_WOOD:   return ST_WALL_WOOD;
    case ITEM_WALL_STONE:  return ST_WALL_STONE;
    case ITEM_DOOR_WOOD:   return ST_DOOR;
    case ITEM_WINDOW:      return ST_WINDOW;
    case ITEM_ROOF:        return ST_ROOF;
    case ITEM_STAIRS:      return ST_STAIRS;
    case ITEM_FENCE:       return ST_FENCE;
    case ITEM_GATE:        return ST_GATE;
    case ITEM_WORKBENCH:   return ST_WORKBENCH;
    case ITEM_FORGE:       return ST_FORGE;
    case ITEM_FURNACE:     return ST_FURNACE;
    case ITEM_STORAGE_BOX: return ST_STORAGE_BOX;
    case ITEM_BED:         return ST_BED;
    case ITEM_WATER_BARREL:return ST_WATER_BARREL;
    case ITEM_FARM_PLOT:   return ST_FARM_PLOT;
    case ITEM_TRAP_SPIKE:  return ST_TRAP_SPIKE;
    case ITEM_WATCHTOWER:  return ST_WATCHTOWER;
    case ITEM_GENERATOR:   return ST_GENERATOR;
    case ITEM_LAMP_POST:   return ST_LAMP_POST;
    case ITEM_TENT:        return ST_TENT;
    default:               return ST_NONE;
    }
}

void structures_boot(void)
{
    memset(g_structures, 0, sizeof g_structures);
    g_structure_count = 0;
}

int structures_place(item_id_t item, fx_t x, fx_t z, u16 rot_y, u8 owner)
{
    structure_type_t type = structure_type_of_item(item);
    if (type == ST_NONE) return -1;

    for (int i = 0; i < MAX_STRUCTURES; ++i) {
        structure_t *s = &g_structures[i];
        if (s->active) continue;
        memset(s, 0, sizeof *s);
        s->active = 1;
        s->type   = (u8)type;
        s->owner  = owner;
        s->hp     = s_type_hp[type] ? s_type_hp[type] : 100;
        s->rot_y  = rot_y;
        s->pos.x  = x;
        s->pos.z  = z;
        if (type == ST_CAMPFIRE || type == ST_FORGE || type == ST_FURNACE) {
            s->lit  = 1;
            s->fuel = 60;
        }
        if (i >= g_structure_count) g_structure_count = i + 1;
        return i;
    }
    return -1;
}

void structures_damage(u16 idx, s16 dmg)
{
    if (idx >= MAX_STRUCTURES) return;
    structure_t *s = &g_structures[idx];
    if (!s->active) return;
    s->hp -= dmg;
    if (s->hp <= 0) {
        s->active = 0;
        s->hp = 0;
    }
}

void structures_tick(u32 dt_ms)
{
    for (int i = 0; i < MAX_STRUCTURES; ++i) {
        structure_t *s = &g_structures[i];
        if (!s->active) continue;

        /* Consumo de combustivel das estacoes acesas. */
        if (s->lit && s->fuel) {
            static u32 fuel_acc = 0;
            fuel_acc += dt_ms;
            if (fuel_acc >= 4000) { s->fuel--; fuel_acc = 0; }
            if (!s->fuel) s->lit = 0;
        }

        /* Crescimento das plantacoes. */
        if (s->type == ST_FARM_PLOT && s->growth < 255) {
            static u32 grow_acc = 0;
            grow_acc += dt_ms;
            if (grow_acc >= 3000) {
                if (s->growth < 250) s->growth += 5; else s->growth = 255;
                grow_acc = 0;
            }
        }

        /* Estruturas ocupam o grid para colisao/raycast. */
        spatial_insert(ENT_ITEM, (u16)i, s->pos.x, s->pos.z);
    }
}

void structures_render(void)
{
    draw_cmd_t c = {0};
    for (int i = 0; i < MAX_STRUCTURES; ++i) {
        const structure_t *s = &g_structures[i];
        if (!s->active) continue;
        c.model = (s->type == ST_CAMPFIRE) ? MDL_CAMPFIRE :
                  (s->type == ST_STORAGE_BOX) ? MDL_LOOT : MDL_STRUCTURE;
        c.pos   = s->pos;
        c.rot_y = s->rot_y;
        c.flags = (u8)s->type;
        render_submit(&c);
    }
}

int structures_near(structure_type_t type, fx_t x, fx_t z, fx_t radius_m)
{
    fx_t r2 = FX_MUL(radius_m, radius_m);
    for (int i = 0; i < MAX_STRUCTURES; ++i) {
        const structure_t *s = &g_structures[i];
        if (!s->active || s->type != (u8)type) continue;
        fx_t dx = s->pos.x - x, dz = s->pos.z - z;
        if (FX_MUL(dx, dx) + FX_MUL(dz, dz) <= r2) return i;
    }
    return -1;
}
