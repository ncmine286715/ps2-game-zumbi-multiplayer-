#ifndef ZUMBI_WORLD_H
#define ZUMBI_WORLD_H

#include "types.h"

typedef struct {
    vec3_t pos;
    u16    rot_y;
    u8     species;     /* MDL_TREE_PINE / OAK / BUSH */
    u8     hp;          /* 1..255 — derruba com machado */
} tree_t;

typedef struct {
    tree_t trees[MAX_TREES];
    u16    tree_count;
    u32    seed;
    fx_t   world_min_x, world_min_z;
    fx_t   world_max_x, world_max_z;
} world_t;

extern world_t g_world;

/* Floresta procedural deterministica — mesmo seed = mesma floresta
 * em todos os clientes, sem sync de geometria pela rede. */
void world_generate_forest(u32 seed);
void world_tick(u32 dt_ms);
void world_render(void);

/* Resource quando arvore derruba. */
typedef enum {
    DROP_WOOD = 1,
    DROP_STICK,
    DROP_LEAF,
} drop_kind_t;

void world_damage_tree(u16 tree_id, u8 dmg, u8 by_player);

#endif
