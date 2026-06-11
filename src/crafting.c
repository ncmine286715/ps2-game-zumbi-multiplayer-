#include "crafting.h"

/* Receitas basicas inspiradas em jogos de sobrevivencia. As receitas
 * ficam em ROM (static const) — zero alocacao. */
const recipe_t g_recipes[] = {
    {
        .name = "Machado de madeira",
        .inputs = { {ITEM_WOOD, 3}, {ITEM_STICK, 2}, {ITEM_FIBER, 1}, {0,0} },
        .ninputs = 3,
        .output = { ITEM_AXE_WOOD, 1 },
        .needs_workbench = 0,
        .craft_time_ms = 200,
    },
    {
        .name = "Picareta de madeira",
        .inputs = { {ITEM_WOOD, 2}, {ITEM_STONE, 3}, {ITEM_STICK, 2}, {0,0} },
        .ninputs = 3,
        .output = { ITEM_PICK_WOOD, 1 },
        .needs_workbench = 0,
        .craft_time_ms = 200,
    },
    {
        .name = "Lanca",
        .inputs = { {ITEM_STICK, 4}, {ITEM_STONE, 1}, {ITEM_FIBER, 2}, {0,0} },
        .ninputs = 3,
        .output = { ITEM_SPEAR, 1 },
        .needs_workbench = 0,
        .craft_time_ms = 250,
    },
    {
        .name = "Tocha",
        .inputs = { {ITEM_STICK, 1}, {ITEM_LEAF, 2}, {0,0}, {0,0} },
        .ninputs = 2,
        .output = { ITEM_TORCH, 1 },
        .needs_workbench = 0,
        .craft_time_ms = 100,
    },
    {
        .name = "Fogueira",
        .inputs = { {ITEM_WOOD, 4}, {ITEM_STONE, 3}, {0,0}, {0,0} },
        .ninputs = 2,
        .output = { ITEM_CAMPFIRE, 1 },
        .needs_workbench = 0,
        .craft_time_ms = 250,
    },
    {
        .name = "Parede de madeira",
        .inputs = { {ITEM_WOOD, 6}, {0,0}, {0,0}, {0,0} },
        .ninputs = 1,
        .output = { ITEM_WALL_WOOD, 1 },
        .needs_workbench = 1,
        .craft_time_ms = 300,
    },
    {
        .name = "Bandagem",
        .inputs = { {ITEM_FIBER, 3}, {ITEM_LEAF, 1}, {0,0}, {0,0} },
        .ninputs = 2,
        .output = { ITEM_BANDAGE, 1 },
        .needs_workbench = 0,
        .craft_time_ms = 150,
    },
    {
        .name = "Carne assada",
        .inputs = { {ITEM_MEAT_RAW, 1}, {0,0}, {0,0}, {0,0} },
        .ninputs = 1,
        .output = { ITEM_MEAT_COOKED, 1 },
        .needs_workbench = 0,    /* precisa fogueira no mundo */
        .craft_time_ms = 200,
    },
};

const u32 g_recipe_count = sizeof(g_recipes) / sizeof(g_recipes[0]);

int craft_can(const inventory_t *inv, u32 idx)
{
    if (idx >= g_recipe_count) return 0;
    const recipe_t *r = &g_recipes[idx];
    for (int i = 0; i < r->ninputs; ++i)
        if (inv_count(inv, r->inputs[i].id) < r->inputs[i].count)
            return 0;
    return 1;
}

int craft_try(inventory_t *inv, u32 idx)
{
    if (!craft_can(inv, idx)) return 0;
    const recipe_t *r = &g_recipes[idx];
    for (int i = 0; i < r->ninputs; ++i)
        inv_remove(inv, r->inputs[i].id, r->inputs[i].count);
    inv_add(inv, r->output.id, r->output.count);
    return 1;
}
