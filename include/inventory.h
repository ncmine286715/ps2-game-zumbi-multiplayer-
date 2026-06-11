#ifndef ZUMBI_INVENTORY_H
#define ZUMBI_INVENTORY_H

#include "types.h"

/* IDs de item — manter compactos (8 bits) para netcode.
 *
 * Organizado em blocos: recursos, comida/remedio, municao, as 40
 * TOOLS/armas (ITEM_TOOL_FIRST..ITEM_TOOL_LAST), itens de construcao e
 * armadura. ITEM_SPEAR continua valido como alias de ITEM_SPEAR_WOOD. */
typedef enum {
    ITEM_NONE = 0,

    /* ---- recursos brutos / refinados ---- */
    ITEM_WOOD,
    ITEM_STICK,
    ITEM_LEAF,
    ITEM_STONE,
    ITEM_FIBER,
    ITEM_FLINT,
    ITEM_IRON_ORE,
    ITEM_IRON_INGOT,
    ITEM_COAL,
    ITEM_CHARCOAL,
    ITEM_CLAY,
    ITEM_SAND,
    ITEM_GLASS,
    ITEM_ROPE,
    ITEM_CLOTH,
    ITEM_LEATHER,
    ITEM_NAIL,
    ITEM_PLANK,
    ITEM_GUNPOWDER,
    ITEM_SCRAP_METAL,
    ITEM_WIRE,
    ITEM_GEAR,
    ITEM_BONE,
    ITEM_FEATHER,

    /* ---- comida / consumiveis / remedio ---- */
    ITEM_BERRY,
    ITEM_MUSHROOM,
    ITEM_MEAT_RAW,
    ITEM_MEAT_COOKED,
    ITEM_FISH_RAW,
    ITEM_FISH_COOKED,
    ITEM_WATER_DIRTY,
    ITEM_WATER_CLEAN,
    ITEM_CANNED_FOOD,
    ITEM_SEED,
    ITEM_CROP,
    ITEM_BANDAGE,
    ITEM_MEDKIT,
    ITEM_ANTIBIOTIC,
    ITEM_PAINKILLER,

    /* ---- municao ---- */
    ITEM_ARROW,
    ITEM_BOLT,
    ITEM_PEBBLE,
    ITEM_AMMO_9MM,
    ITEM_AMMO_RIFLE,
    ITEM_AMMO_SHELL,

    /* ========== 40 FERRAMENTAS / ARMAS ========== */
    ITEM_TOOL_FIRST,
    ITEM_AXE_WOOD = ITEM_TOOL_FIRST,  /* 1  */
    ITEM_AXE_STONE,                   /* 2  */
    ITEM_AXE_IRON,                    /* 3  */
    ITEM_PICK_WOOD,                   /* 4  */
    ITEM_PICK_STONE,                  /* 5  */
    ITEM_PICK_IRON,                   /* 6  */
    ITEM_KNIFE_STONE,                 /* 7  */
    ITEM_KNIFE_IRON,                  /* 8  */
    ITEM_SPEAR_WOOD,                  /* 9  */
    ITEM_SPEAR_IRON,                  /* 10 */
    ITEM_HAMMER,                      /* 11 */
    ITEM_SHOVEL_WOOD,                 /* 12 */
    ITEM_SHOVEL_IRON,                 /* 13 */
    ITEM_HOE,                         /* 14 */
    ITEM_FISHING_ROD,                 /* 15 */
    ITEM_BOW,                         /* 16 */
    ITEM_CROSSBOW,                    /* 17 */
    ITEM_SLINGSHOT,                   /* 18 */
    ITEM_MACHETE,                     /* 19 */
    ITEM_CLUB,                        /* 20 */
    ITEM_BAT_NAIL,                    /* 21 */
    ITEM_SWORD_IRON,                  /* 22 */
    ITEM_SLEDGEHAMMER,                /* 23 */
    ITEM_WRENCH,                      /* 24 */
    ITEM_SCISSORS,                    /* 25 */
    ITEM_NEEDLE,                      /* 26 */
    ITEM_SAW,                         /* 27 */
    ITEM_CHISEL,                      /* 28 */
    ITEM_SICKLE,                      /* 29 */
    ITEM_FLINT_STEEL,                 /* 30 */
    ITEM_TORCH,                       /* 31 */
    ITEM_LANTERN,                     /* 32 */
    ITEM_BINOCULARS,                  /* 33 */
    ITEM_COMPASS,                     /* 34 */
    ITEM_MAP,                         /* 35 */
    ITEM_REPAIR_KIT,                  /* 36 */
    ITEM_PISTOL,                      /* 37 */
    ITEM_RIFLE,                       /* 38 */
    ITEM_SHOTGUN,                     /* 39 */
    ITEM_MOLOTOV,                     /* 40 */
    ITEM_TOOL_LAST = ITEM_MOLOTOV,

    /* ---- itens de construcao (colocam structure_t no mundo) ---- */
    ITEM_CAMPFIRE,
    ITEM_FOUNDATION,
    ITEM_FLOOR_WOOD,
    ITEM_WALL_WOOD,
    ITEM_WALL_STONE,
    ITEM_DOOR_WOOD,
    ITEM_WINDOW,
    ITEM_ROOF,
    ITEM_STAIRS,
    ITEM_FENCE,
    ITEM_GATE,
    ITEM_WORKBENCH,
    ITEM_FORGE,
    ITEM_FURNACE,
    ITEM_STORAGE_BOX,
    ITEM_BED,
    ITEM_WATER_BARREL,
    ITEM_FARM_PLOT,
    ITEM_TRAP_SPIKE,
    ITEM_WATCHTOWER,
    ITEM_GENERATOR,
    ITEM_LAMP_POST,
    ITEM_TENT,

    /* ---- armadura / vestimenta ---- */
    ITEM_ARMOR_CLOTH,
    ITEM_ARMOR_LEATHER,
    ITEM_ARMOR_IRON,
    ITEM_HELMET,
    ITEM_BACKPACK,

    ITEM_COUNT
} item_id_t;

/* Alias retrocompativel com a base inicial. */
#define ITEM_SPEAR     ITEM_SPEAR_WOOD
#define ITEM_PICK_WOOD_ALIAS ITEM_PICK_WOOD

#define ITEM_IS_TOOL(id)  ((id) >= ITEM_TOOL_FIRST && (id) <= ITEM_TOOL_LAST)
#define NUM_TOOLS         (ITEM_TOOL_LAST - ITEM_TOOL_FIRST + 1)

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
    u16 durability;   /* usos ate quebrar (pode passar de 255) */
} item_def_t;

extern const item_def_t g_item_defs[ITEM_COUNT];

#endif
