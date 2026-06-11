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

    /* ---- refino / materiais ---- */
    { "Tabua",      { {ITEM_WOOD,1},{0,0},{0,0},{0,0} }, { ITEM_PLANK,2 },   1, 0, 80 },
    { "Corda",      { {ITEM_FIBER,3},{0,0},{0,0},{0,0} }, { ITEM_ROPE,1 },    1, 0, 100 },
    { "Tecido",     { {ITEM_FIBER,5},{0,0},{0,0},{0,0} }, { ITEM_CLOTH,1 },   1, 0, 120 },
    { "Carvao veg.",{ {ITEM_WOOD,2},{0,0},{0,0},{0,0} },  { ITEM_CHARCOAL,1 },2, 1, 150 },
    { "Lingote",    { {ITEM_IRON_ORE,2},{ITEM_COAL,1},{0,0},{0,0} }, { ITEM_IRON_INGOT,1 }, 2, 1, 200 },
    { "Vidro",      { {ITEM_SAND,2},{ITEM_COAL,1},{0,0},{0,0} },     { ITEM_GLASS,1 },      2, 1, 180 },
    { "Prego",      { {ITEM_IRON_INGOT,1},{0,0},{0,0},{0,0} },       { ITEM_NAIL,4 },       1, 1, 120 },
    { "Polvora",    { {ITEM_COAL,1},{ITEM_STONE,1},{0,0},{0,0} },    { ITEM_GUNPOWDER,1 },  2, 1, 150 },
    { "Flecha",     { {ITEM_STICK,1},{ITEM_FLINT,1},{ITEM_FEATHER,1},{0,0} }, { ITEM_ARROW,4 }, 3, 0, 100 },

    /* ---- ferramentas / armas ---- */
    { "Machado pedra", { {ITEM_WOOD,2},{ITEM_STONE,3},{ITEM_FIBER,2},{0,0} }, { ITEM_AXE_STONE,1 }, 3, 0, 200 },
    { "Machado ferro", { {ITEM_PLANK,2},{ITEM_IRON_INGOT,3},{ITEM_ROPE,1},{0,0} }, { ITEM_AXE_IRON,1 }, 3, 1, 250 },
    { "Picareta pedra",{ {ITEM_WOOD,2},{ITEM_STONE,3},{ITEM_STICK,2},{0,0} }, { ITEM_PICK_STONE,1 }, 3, 0, 200 },
    { "Picareta ferro",{ {ITEM_PLANK,2},{ITEM_IRON_INGOT,3},{ITEM_ROPE,1},{0,0} }, { ITEM_PICK_IRON,1 }, 3, 1, 250 },
    { "Faca pedra",    { {ITEM_STONE,1},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_KNIFE_STONE,1 }, 2, 0, 120 },
    { "Faca ferro",    { {ITEM_IRON_INGOT,1},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_KNIFE_IRON,1 }, 2, 1, 150 },
    { "Lanca ferro",   { {ITEM_STICK,3},{ITEM_IRON_INGOT,1},{ITEM_ROPE,1},{0,0} }, { ITEM_SPEAR_IRON,1 }, 3, 1, 200 },
    { "Martelo",       { {ITEM_STICK,2},{ITEM_STONE,2},{0,0},{0,0} }, { ITEM_HAMMER,1 }, 2, 0, 150 },
    { "Pa madeira",    { {ITEM_PLANK,2},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_SHOVEL_WOOD,1 }, 2, 0, 150 },
    { "Pa ferro",      { {ITEM_IRON_INGOT,2},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_SHOVEL_IRON,1 }, 2, 1, 200 },
    { "Enxada",        { {ITEM_PLANK,1},{ITEM_STICK,2},{0,0},{0,0} }, { ITEM_HOE,1 }, 2, 0, 150 },
    { "Vara pesca",    { {ITEM_STICK,3},{ITEM_FIBER,2},{ITEM_ROPE,1},{0,0} }, { ITEM_FISHING_ROD,1 }, 3, 0, 150 },
    { "Arco",          { {ITEM_STICK,3},{ITEM_ROPE,1},{ITEM_FIBER,2},{0,0} }, { ITEM_BOW,1 }, 3, 0, 200 },
    { "Besta",         { {ITEM_PLANK,2},{ITEM_IRON_INGOT,1},{ITEM_ROPE,1},{0,0} }, { ITEM_CROSSBOW,1 }, 3, 1, 250 },
    { "Estilingue",    { {ITEM_STICK,1},{ITEM_ROPE,1},{ITEM_LEATHER,1},{0,0} }, { ITEM_SLINGSHOT,1 }, 3, 0, 120 },
    { "Facao",         { {ITEM_IRON_INGOT,2},{ITEM_LEATHER,1},{0,0},{0,0} }, { ITEM_MACHETE,1 }, 2, 1, 200 },
    { "Porrete",       { {ITEM_WOOD,3},{0,0},{0,0},{0,0} }, { ITEM_CLUB,1 }, 1, 0, 100 },
    { "Taco pregos",   { {ITEM_CLUB,1},{ITEM_NAIL,4},{0,0},{0,0} }, { ITEM_BAT_NAIL,1 }, 2, 0, 150 },
    { "Espada ferro",  { {ITEM_IRON_INGOT,3},{ITEM_LEATHER,1},{0,0},{0,0} }, { ITEM_SWORD_IRON,1 }, 2, 1, 250 },
    { "Marreta",       { {ITEM_IRON_INGOT,4},{ITEM_STICK,2},{0,0},{0,0} }, { ITEM_SLEDGEHAMMER,1 }, 2, 1, 250 },
    { "Chave inglesa", { {ITEM_IRON_INGOT,2},{0,0},{0,0},{0,0} }, { ITEM_WRENCH,1 }, 1, 1, 150 },
    { "Tesoura",       { {ITEM_IRON_INGOT,1},{0,0},{0,0},{0,0} }, { ITEM_SCISSORS,1 }, 1, 1, 120 },
    { "Agulha",        { {ITEM_IRON_INGOT,1},{0,0},{0,0},{0,0} }, { ITEM_NEEDLE,1 }, 1, 1, 100 },
    { "Serra",         { {ITEM_IRON_INGOT,2},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_SAW,1 }, 2, 1, 150 },
    { "Formao",        { {ITEM_IRON_INGOT,1},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_CHISEL,1 }, 2, 1, 120 },
    { "Foice",         { {ITEM_IRON_INGOT,2},{ITEM_STICK,1},{0,0},{0,0} }, { ITEM_SICKLE,1 }, 2, 1, 150 },
    { "Isqueiro",      { {ITEM_FLINT,1},{ITEM_SCRAP_METAL,1},{0,0},{0,0} }, { ITEM_FLINT_STEEL,1 }, 2, 0, 120 },
    { "Lanterna",      { {ITEM_SCRAP_METAL,2},{ITEM_GLASS,1},{ITEM_WIRE,1},{0,0} }, { ITEM_LANTERN,1 }, 3, 1, 180 },
    { "Binoculo",      { {ITEM_GLASS,2},{ITEM_SCRAP_METAL,1},{0,0},{0,0} }, { ITEM_BINOCULARS,1 }, 2, 1, 150 },
    { "Bussola",       { {ITEM_IRON_INGOT,1},{ITEM_GLASS,1},{0,0},{0,0} }, { ITEM_COMPASS,1 }, 2, 1, 150 },
    { "Mapa",          { {ITEM_PLANK,1},{ITEM_CLOTH,1},{0,0},{0,0} }, { ITEM_MAP,1 }, 2, 0, 120 },
    { "Kit reparo",    { {ITEM_SCRAP_METAL,2},{ITEM_WIRE,1},{ITEM_CLOTH,1},{0,0} }, { ITEM_REPAIR_KIT,1 }, 3, 1, 150 },
    { "Pistola",       { {ITEM_SCRAP_METAL,3},{ITEM_GEAR,1},{ITEM_WIRE,1},{0,0} }, { ITEM_PISTOL,1 }, 3, 1, 250 },
    { "Rifle",         { {ITEM_SCRAP_METAL,4},{ITEM_GEAR,2},{ITEM_PLANK,1},{0,0} }, { ITEM_RIFLE,1 }, 3, 1, 255 },
    { "Escopeta",      { {ITEM_SCRAP_METAL,4},{ITEM_GEAR,1},{ITEM_PLANK,1},{0,0} }, { ITEM_SHOTGUN,1 }, 3, 1, 255 },
    { "Molotov",       { {ITEM_GLASS,1},{ITEM_CLOTH,1},{ITEM_GUNPOWDER,1},{0,0} }, { ITEM_MOLOTOV,1 }, 3, 0, 120 },

    /* ---- municao ---- */
    { "Bala 9mm",    { {ITEM_SCRAP_METAL,1},{ITEM_GUNPOWDER,1},{0,0},{0,0} }, { ITEM_AMMO_9MM,6 }, 2, 1, 120 },
    { "Bala rifle",  { {ITEM_SCRAP_METAL,1},{ITEM_GUNPOWDER,2},{0,0},{0,0} }, { ITEM_AMMO_RIFLE,4 }, 2, 1, 140 },
    { "Cartucho",    { {ITEM_SCRAP_METAL,1},{ITEM_GUNPOWDER,2},{ITEM_GEAR,0},{0,0} }, { ITEM_AMMO_SHELL,3 }, 2, 1, 140 },
    { "Virote",      { {ITEM_STICK,1},{ITEM_IRON_INGOT,1},{0,0},{0,0} }, { ITEM_BOLT,4 }, 2, 1, 100 },

    /* ---- estruturas (construcao) ---- */
    { "Fogueira",    { {ITEM_WOOD,4},{ITEM_STONE,3},{0,0},{0,0} }, { ITEM_CAMPFIRE,1 }, 2, 0, 200 },
    { "Alicerce",    { {ITEM_PLANK,4},{ITEM_STONE,2},{0,0},{0,0} }, { ITEM_FOUNDATION,1 }, 2, 0, 200 },
    { "Piso",        { {ITEM_PLANK,3},{0,0},{0,0},{0,0} }, { ITEM_FLOOR_WOOD,1 }, 1, 0, 150 },
    { "Parede pedra",{ {ITEM_STONE,8},{ITEM_NAIL,2},{0,0},{0,0} }, { ITEM_WALL_STONE,1 }, 2, 1, 250 },
    { "Porta",       { {ITEM_PLANK,4},{ITEM_NAIL,2},{0,0},{0,0} }, { ITEM_DOOR_WOOD,1 }, 2, 0, 180 },
    { "Janela",      { {ITEM_PLANK,2},{ITEM_GLASS,1},{0,0},{0,0} }, { ITEM_WINDOW,1 }, 2, 0, 150 },
    { "Telhado",     { {ITEM_PLANK,4},{ITEM_FIBER,4},{0,0},{0,0} }, { ITEM_ROOF,1 }, 2, 0, 160 },
    { "Escada",      { {ITEM_PLANK,3},{ITEM_NAIL,2},{0,0},{0,0} }, { ITEM_STAIRS,1 }, 2, 0, 160 },
    { "Cerca",       { {ITEM_STICK,4},{ITEM_ROPE,1},{0,0},{0,0} }, { ITEM_FENCE,1 }, 2, 0, 100 },
    { "Portao",      { {ITEM_PLANK,4},{ITEM_ROPE,1},{0,0},{0,0} }, { ITEM_GATE,1 }, 2, 0, 160 },
    { "Bancada",     { {ITEM_PLANK,4},{ITEM_STICK,2},{0,0},{0,0} }, { ITEM_WORKBENCH,1 }, 2, 0, 200 },
    { "Forja",       { {ITEM_STONE,8},{ITEM_CLAY,4},{ITEM_COAL,2},{0,0} }, { ITEM_FORGE,1 }, 3, 1, 250 },
    { "Fornalha",    { {ITEM_STONE,6},{ITEM_CLAY,4},{0,0},{0,0} }, { ITEM_FURNACE,1 }, 2, 0, 250 },
    { "Bau",         { {ITEM_PLANK,4},{ITEM_NAIL,4},{0,0},{0,0} }, { ITEM_STORAGE_BOX,1 }, 2, 0, 180 },
    { "Cama",        { {ITEM_PLANK,3},{ITEM_CLOTH,3},{0,0},{0,0} }, { ITEM_BED,1 }, 2, 0, 180 },
    { "Barril agua", { {ITEM_PLANK,4},{ITEM_IRON_INGOT,1},{0,0},{0,0} }, { ITEM_WATER_BARREL,1 }, 2, 0, 180 },
    { "Canteiro",    { {ITEM_WOOD,2},{ITEM_FIBER,2},{0,0},{0,0} }, { ITEM_FARM_PLOT,1 }, 2, 0, 120 },
    { "Armadilha",   { {ITEM_STICK,2},{ITEM_IRON_INGOT,1},{0,0},{0,0} }, { ITEM_TRAP_SPIKE,1 }, 2, 1, 150 },
    { "Torre vigia", { {ITEM_PLANK,8},{ITEM_NAIL,6},{ITEM_ROPE,2},{0,0} }, { ITEM_WATCHTOWER,1 }, 3, 1, 255 },
    { "Gerador",     { {ITEM_SCRAP_METAL,4},{ITEM_GEAR,2},{ITEM_WIRE,2},{0,0} }, { ITEM_GENERATOR,1 }, 3, 1, 255 },
    { "Poste luz",   { {ITEM_SCRAP_METAL,2},{ITEM_GLASS,1},{ITEM_WIRE,2},{0,0} }, { ITEM_LAMP_POST,1 }, 3, 1, 180 },
    { "Barraca",     { {ITEM_CLOTH,4},{ITEM_STICK,4},{ITEM_ROPE,2},{0,0} }, { ITEM_TENT,1 }, 3, 0, 180 },

    /* ---- armadura / medico ---- */
    { "Roupa pano",   { {ITEM_CLOTH,4},{0,0},{0,0},{0,0} }, { ITEM_ARMOR_CLOTH,1 }, 1, 0, 150 },
    { "Armad. couro", { {ITEM_LEATHER,5},{ITEM_ROPE,2},{0,0},{0,0} }, { ITEM_ARMOR_LEATHER,1 }, 2, 0, 200 },
    { "Armad. ferro", { {ITEM_IRON_INGOT,5},{ITEM_LEATHER,2},{0,0},{0,0} }, { ITEM_ARMOR_IRON,1 }, 2, 1, 250 },
    { "Capacete",     { {ITEM_IRON_INGOT,3},{ITEM_CLOTH,1},{0,0},{0,0} }, { ITEM_HELMET,1 }, 2, 1, 200 },
    { "Mochila",      { {ITEM_LEATHER,4},{ITEM_ROPE,2},{0,0},{0,0} }, { ITEM_BACKPACK,1 }, 2, 0, 200 },
    { "Kit medico",   { {ITEM_CLOTH,2},{ITEM_FIBER,2},{ITEM_BANDAGE,2},{0,0} }, { ITEM_MEDKIT,1 }, 3, 0, 200 },
    { "Agua limpa",   { {ITEM_WATER_DIRTY,1},{ITEM_COAL,1},{0,0},{0,0} }, { ITEM_WATER_CLEAN,1 }, 2, 0, 100 },
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
