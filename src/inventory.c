#include "inventory.h"
#include <string.h>

/* Tabela mestra. Designated initializers: itens nao listados ficam {0}.
 * Campos: { nome, max_stack, is_tool, damage, durability }. */
const item_def_t g_item_defs[ITEM_COUNT] = {
    [ITEM_NONE]        = { "none",          0,  0,  0,   0 },

    /* recursos */
    [ITEM_WOOD]        = { "Madeira",       64, 0,  0,   0 },
    [ITEM_STICK]       = { "Galho",         64, 0,  0,   0 },
    [ITEM_LEAF]        = { "Folha",         64, 0,  0,   0 },
    [ITEM_STONE]       = { "Pedra",         64, 0,  0,   0 },
    [ITEM_FIBER]       = { "Fibra",         64, 0,  0,   0 },
    [ITEM_FLINT]       = { "Pederneira",    64, 0,  0,   0 },
    [ITEM_IRON_ORE]    = { "Minerio ferro", 32, 0,  0,   0 },
    [ITEM_IRON_INGOT]  = { "Lingote ferro", 32, 0,  0,   0 },
    [ITEM_COAL]        = { "Carvao",        32, 0,  0,   0 },
    [ITEM_CHARCOAL]    = { "Carvao veg.",   32, 0,  0,   0 },
    [ITEM_CLAY]        = { "Argila",        32, 0,  0,   0 },
    [ITEM_SAND]        = { "Areia",         32, 0,  0,   0 },
    [ITEM_GLASS]       = { "Vidro",         16, 0,  0,   0 },
    [ITEM_ROPE]        = { "Corda",         16, 0,  0,   0 },
    [ITEM_CLOTH]       = { "Tecido",        16, 0,  0,   0 },
    [ITEM_LEATHER]     = { "Couro",         16, 0,  0,   0 },
    [ITEM_NAIL]        = { "Prego",         64, 0,  0,   0 },
    [ITEM_PLANK]       = { "Tabua",         32, 0,  0,   0 },
    [ITEM_GUNPOWDER]   = { "Polvora",       16, 0,  0,   0 },
    [ITEM_SCRAP_METAL] = { "Sucata",        32, 0,  0,   0 },
    [ITEM_WIRE]        = { "Fio",           32, 0,  0,   0 },
    [ITEM_GEAR]        = { "Engrenagem",    16, 0,  0,   0 },
    [ITEM_BONE]        = { "Osso",          32, 0,  0,   0 },
    [ITEM_FEATHER]     = { "Pena",          32, 0,  0,   0 },

    /* comida / remedio */
    [ITEM_BERRY]       = { "Fruta",         32, 0,  0,   0 },
    [ITEM_MUSHROOM]    = { "Cogumelo",      32, 0,  0,   0 },
    [ITEM_MEAT_RAW]    = { "Carne crua",    16, 0,  0,   0 },
    [ITEM_MEAT_COOKED] = { "Carne assada",  16, 0,  0,   0 },
    [ITEM_FISH_RAW]    = { "Peixe cru",     16, 0,  0,   0 },
    [ITEM_FISH_COOKED] = { "Peixe assado",  16, 0,  0,   0 },
    [ITEM_WATER_DIRTY] = { "Agua suja",     8,  0,  0,   0 },
    [ITEM_WATER_CLEAN] = { "Agua limpa",    8,  0,  0,   0 },
    [ITEM_CANNED_FOOD] = { "Enlatado",      8,  0,  0,   0 },
    [ITEM_SEED]        = { "Semente",       32, 0,  0,   0 },
    [ITEM_CROP]        = { "Vegetal",       32, 0,  0,   0 },
    [ITEM_BANDAGE]     = { "Bandagem",      8,  0,  0,   0 },
    [ITEM_MEDKIT]      = { "Kit medico",    4,  0,  0,   0 },
    [ITEM_ANTIBIOTIC]  = { "Antibiotico",   8,  0,  0,   0 },
    [ITEM_PAINKILLER]  = { "Analgesico",    8,  0,  0,   0 },

    /* municao */
    [ITEM_ARROW]       = { "Flecha",        32, 0,  8,   0 },
    [ITEM_BOLT]        = { "Virote",        32, 0,  12,  0 },
    [ITEM_PEBBLE]      = { "Seixo",         32, 0,  4,   0 },
    [ITEM_AMMO_9MM]    = { "Bala 9mm",      32, 0,  22,  0 },
    [ITEM_AMMO_RIFLE]  = { "Bala rifle",    32, 0,  40,  0 },
    [ITEM_AMMO_SHELL]  = { "Cartucho",      16, 0,  55,  0 },

    /* ferramentas / armas (40) */
    [ITEM_AXE_WOOD]    = { "Machado mad.",  1,  1,  20,  100 },
    [ITEM_AXE_STONE]   = { "Machado pedra", 1,  1,  28,  160 },
    [ITEM_AXE_IRON]    = { "Machado ferro", 1,  1,  40,  300 },
    [ITEM_PICK_WOOD]   = { "Picareta mad.", 1,  1,  15,  120 },
    [ITEM_PICK_STONE]  = { "Picareta pedra",1,  1,  22,  180 },
    [ITEM_PICK_IRON]   = { "Picareta ferro",1,  1,  32,  320 },
    [ITEM_KNIFE_STONE] = { "Faca pedra",    1,  1,  18,  120 },
    [ITEM_KNIFE_IRON]  = { "Faca ferro",    1,  1,  26,  240 },
    [ITEM_SPEAR_WOOD]  = { "Lanca mad.",    1,  1,  35,  80  },
    [ITEM_SPEAR_IRON]  = { "Lanca ferro",   1,  1,  48,  200 },
    [ITEM_HAMMER]      = { "Martelo",       1,  1,  18,  250 },
    [ITEM_SHOVEL_WOOD] = { "Pa mad.",       1,  1,  10,  120 },
    [ITEM_SHOVEL_IRON] = { "Pa ferro",      1,  1,  16,  280 },
    [ITEM_HOE]         = { "Enxada",        1,  1,  8,   160 },
    [ITEM_FISHING_ROD] = { "Vara pesca",    1,  1,  2,   100 },
    [ITEM_BOW]         = { "Arco",          1,  1,  45,  150 },
    [ITEM_CROSSBOW]    = { "Besta",         1,  1,  70,  180 },
    [ITEM_SLINGSHOT]   = { "Estilingue",    1,  1,  20,  120 },
    [ITEM_MACHETE]     = { "Facao",         1,  1,  44,  220 },
    [ITEM_CLUB]        = { "Porrete",       1,  1,  30,  140 },
    [ITEM_BAT_NAIL]    = { "Taco pregos",   1,  1,  52,  120 },
    [ITEM_SWORD_IRON]  = { "Espada ferro",  1,  1,  60,  260 },
    [ITEM_SLEDGEHAMMER]= { "Marreta",       1,  1,  58,  200 },
    [ITEM_WRENCH]      = { "Chave inglesa", 1,  1,  24,  220 },
    [ITEM_SCISSORS]    = { "Tesoura",       1,  1,  12,  100 },
    [ITEM_NEEDLE]      = { "Agulha",        1,  1,  4,   80  },
    [ITEM_SAW]         = { "Serra",         1,  1,  20,  160 },
    [ITEM_CHISEL]      = { "Formao",        1,  1,  16,  140 },
    [ITEM_SICKLE]      = { "Foice",         1,  1,  26,  160 },
    [ITEM_FLINT_STEEL] = { "Isqueiro",      1,  1,  2,   200 },
    [ITEM_TORCH]       = { "Tocha",         4,  1,  8,   30  },
    [ITEM_LANTERN]     = { "Lanterna",      1,  1,  2,   200 },
    [ITEM_BINOCULARS]  = { "Binoculo",      1,  1,  0,   255 },
    [ITEM_COMPASS]     = { "Bussola",       1,  1,  0,   255 },
    [ITEM_MAP]         = { "Mapa",          1,  1,  0,   255 },
    [ITEM_REPAIR_KIT]  = { "Kit reparo",    4,  1,  0,   50  },
    [ITEM_PISTOL]      = { "Pistola",       1,  1,  55,  300 },
    [ITEM_RIFLE]       = { "Rifle",         1,  1,  85,  280 },
    [ITEM_SHOTGUN]     = { "Escopeta",      1,  1,  110, 260 },
    [ITEM_MOLOTOV]     = { "Molotov",       4,  1,  90,  1   },

    /* construcao */
    [ITEM_CAMPFIRE]    = { "Fogueira",      4,  0,  0,   0 },
    [ITEM_FOUNDATION]  = { "Alicerce",      8,  0,  0,   400 },
    [ITEM_FLOOR_WOOD]  = { "Piso mad.",     8,  0,  0,   200 },
    [ITEM_WALL_WOOD]   = { "Parede mad.",   8,  0,  0,   200 },
    [ITEM_WALL_STONE]  = { "Parede pedra",  8,  0,  0,   500 },
    [ITEM_DOOR_WOOD]   = { "Porta",         4,  0,  0,   180 },
    [ITEM_WINDOW]      = { "Janela",        4,  0,  0,   120 },
    [ITEM_ROOF]        = { "Telhado",       8,  0,  0,   160 },
    [ITEM_STAIRS]      = { "Escada",        4,  0,  0,   180 },
    [ITEM_FENCE]       = { "Cerca",         8,  0,  0,   120 },
    [ITEM_GATE]        = { "Portao",        4,  0,  0,   200 },
    [ITEM_WORKBENCH]   = { "Bancada",       1,  0,  0,   150 },
    [ITEM_FORGE]       = { "Forja",         1,  0,  0,   250 },
    [ITEM_FURNACE]     = { "Fornalha",      1,  0,  0,   250 },
    [ITEM_STORAGE_BOX] = { "Bau",           4,  0,  0,   150 },
    [ITEM_BED]         = { "Cama",          2,  0,  0,   100 },
    [ITEM_WATER_BARREL]= { "Barril agua",   2,  0,  0,   120 },
    [ITEM_FARM_PLOT]   = { "Canteiro",      8,  0,  0,   80  },
    [ITEM_TRAP_SPIKE]  = { "Armadilha",     8,  0,  40,  60  },
    [ITEM_WATCHTOWER]  = { "Torre vigia",   1,  0,  0,   400 },
    [ITEM_GENERATOR]   = { "Gerador",       1,  0,  0,   200 },
    [ITEM_LAMP_POST]   = { "Poste luz",     4,  0,  0,   120 },
    [ITEM_TENT]        = { "Barraca",       1,  0,  0,   100 },

    /* armadura */
    [ITEM_ARMOR_CLOTH]   = { "Roupa pano",  1,  0,  0,   80  },
    [ITEM_ARMOR_LEATHER] = { "Armad. couro",1,  0,  0,   160 },
    [ITEM_ARMOR_IRON]    = { "Armad. ferro",1,  0,  0,   320 },
    [ITEM_HELMET]        = { "Capacete",    1,  0,  0,   200 },
    [ITEM_BACKPACK]      = { "Mochila",     1,  0,  0,   255 },
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
