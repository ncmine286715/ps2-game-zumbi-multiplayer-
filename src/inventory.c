#include "inventory.h"
#include <string.h>

const item_def_t g_item_defs[ITEM_COUNT] = {
    [ITEM_NONE]       = { "none",        0,  0,  0,   0 },
    [ITEM_WOOD]       = { "Madeira",     64, 0,  0,   0 },
    [ITEM_STICK]      = { "Galho",       64, 0,  0,   0 },
    [ITEM_LEAF]       = { "Folha",       64, 0,  0,   0 },
    [ITEM_STONE]      = { "Pedra",       64, 0,  0,   0 },
    [ITEM_FIBER]      = { "Fibra",       64, 0,  0,   0 },
    [ITEM_AXE_WOOD]   = { "Machado",     1,  1,  20,  100 },
    [ITEM_PICK_WOOD]  = { "Picareta",    1,  1,  15,  120 },
    [ITEM_SPEAR]      = { "Lanca",       1,  1,  35,  80  },
    [ITEM_TORCH]      = { "Tocha",       4,  0,  5,   30  },
    [ITEM_CAMPFIRE]   = { "Fogueira",    1,  0,  0,   0   },
    [ITEM_WALL_WOOD]  = { "Parede",      8,  0,  0,   200 },
    [ITEM_BANDAGE]    = { "Bandagem",    8,  0,  0,   0   },
    [ITEM_BERRY]      = { "Fruta",       32, 0,  0,   0   },
    [ITEM_MEAT_RAW]   = { "Carne crua",  16, 0,  0,   0   },
    [ITEM_MEAT_COOKED]= { "Carne assada",16, 0,  0,   0   },
};

void inv_clear(inventory_t *inv)
{
    memset(inv, 0, sizeof *inv);
}

int inv_add(inventory_t *inv, item_id_t id, u8 count)
{
    if (!count || id == ITEM_NONE) return 0;
    u8 max_stack = g_item_defs[id].max_stack;
    if (!max_stack) max_stack = 1;
    /* Empilha em slot existente. */
    for (int i = 0; i < MAX_INV_SLOTS && count; ++i) {
        if (inv->slots[i].id == id && inv->slots[i].count < max_stack) {
            u8 free = max_stack - inv->slots[i].count;
            u8 take = (count < free) ? count : free;
            inv->slots[i].count += take;
            count -= take;
        }
    }
    /* Cria stacks novas. */
    for (int i = 0; i < MAX_INV_SLOTS && count; ++i) {
        if (!inv->slots[i].count) {
            u8 take = (count < max_stack) ? count : max_stack;
            inv->slots[i].id    = id;
            inv->slots[i].count = take;
            count -= take;
        }
    }
    return count == 0;
}

int inv_remove(inventory_t *inv, item_id_t id, u8 count)
{
    int total = inv_count(inv, id);
    if (total < count) return 0;
    for (int i = 0; i < MAX_INV_SLOTS && count; ++i) {
        if (inv->slots[i].id != id) continue;
        u8 take = (count < inv->slots[i].count) ? count : inv->slots[i].count;
        inv->slots[i].count -= take;
        count -= take;
        if (!inv->slots[i].count) inv->slots[i].id = ITEM_NONE;
    }
    return 1;
}

int inv_count(const inventory_t *inv, item_id_t id)
{
    int n = 0;
    for (int i = 0; i < MAX_INV_SLOTS; ++i)
        if (inv->slots[i].id == id) n += inv->slots[i].count;
    return n;
}

item_id_t inv_equipped(const inventory_t *inv)
{
    return (item_id_t)inv->slots[inv->hotbar].id;
}
