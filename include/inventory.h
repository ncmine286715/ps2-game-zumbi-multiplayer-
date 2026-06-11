#ifndef ZUMBI_INVENTORY_H
#define ZUMBI_INVENTORY_H

#include "types.h"

/* IDs de item — manter compactos (8 bits) para netcode. */
typedef enum {
    ITEM_NONE = 0,
    ITEM_WOOD,
    ITEM_STICK,
    ITEM_LEAF,
    ITEM_STONE,
    ITEM_FIBER,
    ITEM_AXE_WOOD,
    ITEM_PICK_WOOD,
    ITEM_SPEAR,
    ITEM_TORCH,
    ITEM_CAMPFIRE,
    ITEM_WALL_WOOD,
    ITEM_BANDAGE,
    ITEM_BERRY,
    ITEM_MEAT_RAW,
    ITEM_MEAT_COOKED,
    ITEM_COUNT
} item_id_t;

typedef struct {
    u8 id;          /* item_id_t */
    u8 count;       /* 0 = vazio */
} inv_slot_t;

typedef struct {
    inv_slot_t slots[MAX_INV_SLOTS];
    u8         hotbar;        /* slot equipado */
} inventory_t;

void inv_clear(inventory_t *inv);
int  inv_add  (inventory_t *inv, item_id_t id, u8 count);
int  inv_remove(inventory_t *inv, item_id_t id, u8 count);
int  inv_count(const inventory_t *inv, item_id_t id);
item_id_t inv_equipped(const inventory_t *inv);

/* Metadados de item carregados em static zone. */
typedef struct {
    const char *name;
    u8  max_stack;
    u8  is_tool;
    u8  damage;       /* dano se for tool/arma */
    u8  durability;
} item_def_t;

extern const item_def_t g_item_defs[ITEM_COUNT];

#endif
