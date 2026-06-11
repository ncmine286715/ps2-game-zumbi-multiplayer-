#ifndef ZUMBI_CRAFTING_H
#define ZUMBI_CRAFTING_H

#include "inventory.h"

#define RECIPE_MAX_INPUTS 4

typedef struct {
    u8 id;       /* item_id_t */
    u8 count;
} recipe_ing_t;

typedef struct {
    const char     *name;
    recipe_ing_t    inputs[RECIPE_MAX_INPUTS];
    recipe_ing_t    output;
    u8              ninputs;
    u8              needs_workbench;
    u16             craft_time_ms;
} recipe_t;

extern const recipe_t g_recipes[];
extern const u32      g_recipe_count;

/* Retorna 1 se craft foi feito, 0 se faltaram materiais. */
int craft_try(inventory_t *inv, u32 recipe_idx);
int craft_can(const inventory_t *inv, u32 recipe_idx);

#endif
