#ifndef ZUMBI_WORLD_H
#define ZUMBI_WORLD_H

#include "types.h"

/* Biomas do mapa expandido (512 x 512 m). Determinados pela posicao
 * para que a geracao continue deterministica por seed. */
typedef enum {
    BIOME_FOREST = 0,   /* pinheiros/carvalhos densos */
    BIOME_PLAINS,       /* campo aberto, poucas arvores */
    BIOME_SWAMP,        /* arvores mortas, mais zumbis */
    BIOME_BEACH,        /* palmeiras, areia, agua */
    BIOME_ROCKY,        /* pedras/minerio para minerar */
    BIOME_COUNT
} biome_t;

typedef struct {
    vec3_t pos;
    u16    rot_y;
    u8     species;     /* MDL_TREE_PINE / OAK / BUSH / PALM / DEAD_TREE */
    u8     hp;          /* 1..255 — derruba com machado */
} tree_t;

typedef struct {
    vec3_t pos;
    u16    rot_y;
    u8     kind;        /* 0 = pedra comum, 1 = veio de ferro, 2 = carvao */
    u8     hp;          /* esgota com picareta */
} rock_t;

typedef struct {
    vec3_t pos;
    u8     active;
    u8     item;        /* item_id_t dentro do bau */
    u8     count;
    u8     opened;
} loot_t;

typedef struct {
    tree_t trees[MAX_TREES];
    rock_t rocks[MAX_ROCKS];
    loot_t loot[MAX_LOOT];
    u16    tree_count;
    u16    rock_count;
    u16    loot_count;
    u32    seed;
    fx_t   world_min_x, world_min_z;
    fx_t   world_max_x, world_max_z;
} world_t;

extern world_t g_world;

/* Bioma em uma posicao do mundo (deterministico). */
biome_t world_biome_at(fx_t x, fx_t z);

/* Mundo procedural deterministico — mesmo seed = mesmo mapa em todos os
 * clientes, sem sync de geometria pela rede. */
void world_generate_forest(u32 seed);   /* nome mantido por compat. */
void world_tick(u32 dt_ms);
void world_render(void);

/* Resource quando arvore derruba. */
typedef enum {
    DROP_WOOD = 1,
    DROP_STICK,
    DROP_LEAF,
} drop_kind_t;

void world_damage_tree(u16 tree_id, u8 dmg, u8 by_player);
void world_damage_rock(u16 rock_id, u8 dmg, u8 by_player);

#endif
