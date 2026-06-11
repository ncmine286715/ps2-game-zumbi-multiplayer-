/* mechanics.c — sistemas de jogo (sobrevivencia, ambiente, suporte de
 * combate). Chamado uma vez por tick fixo a partir do loop principal.
 *
 * Filosofia: tudo em inteiro/fixed-point, sem alocacao, percorrendo
 * apenas as entidades ativas. As 40 mecanicas estao catalogadas em
 * g_mechanics[] para o overlay de debug e a documentacao. */

#include "mechanics.h"
#include "player.h"
#include "zombie.h"
#include "structures.h"
#include "inventory.h"
#include "audio.h"
#include <string.h>

env_t g_env;

const mech_def_t g_mechanics[MECH_COUNT] = {
    [MECH_MOVE]         = { "Movimento",        MCAT_SURVIVAL,    1 },
    [MECH_SPRINT]       = { "Corrida",          MCAT_SURVIVAL,    1 },
    [MECH_STAMINA]      = { "Stamina",          MCAT_SURVIVAL,    1 },
    [MECH_HUNGER]       = { "Fome",             MCAT_SURVIVAL,    1 },
    [MECH_THIRST]       = { "Sede",             MCAT_SURVIVAL,    1 },
    [MECH_HEALTH_REGEN] = { "Regen. vida",      MCAT_SURVIVAL,    1 },
    [MECH_BLEEDING]     = { "Sangramento",      MCAT_SURVIVAL,    1 },
    [MECH_INFECTION]    = { "Infeccao",         MCAT_SURVIVAL,    1 },
    [MECH_TEMPERATURE]  = { "Temperatura",      MCAT_SURVIVAL,    1 },
    [MECH_DAYNIGHT]     = { "Ciclo dia/noite",  MCAT_ENVIRONMENT, 1 },
    [MECH_WEATHER]      = { "Clima",            MCAT_ENVIRONMENT, 1 },
    [MECH_WETNESS]      = { "Umidade",          MCAT_ENVIRONMENT, 1 },
    [MECH_FIRE]         = { "Fogo/fogueira",    MCAT_ENVIRONMENT, 1 },
    [MECH_COOKING]      = { "Cozinhar",         MCAT_GATHERING,   0 },
    [MECH_FISHING]      = { "Pescar",           MCAT_GATHERING,   0 },
    [MECH_FARMING]      = { "Plantar",          MCAT_GATHERING,   1 },
    [MECH_FORAGING]     = { "Coletar",          MCAT_GATHERING,   0 },
    [MECH_HUNTING]      = { "Cacar",            MCAT_GATHERING,   0 },
    [MECH_CHOPPING]     = { "Cortar arvore",    MCAT_GATHERING,   0 },
    [MECH_MINING]       = { "Minerar",          MCAT_GATHERING,   0 },
    [MECH_CRAFTING]     = { "Crafting",         MCAT_CRAFTBUILD,  0 },
    [MECH_BUILDING]     = { "Construir",        MCAT_CRAFTBUILD,  0 },
    [MECH_DECAY]        = { "Decaimento",       MCAT_CRAFTBUILD,  1 },
    [MECH_DURABILITY]   = { "Durabilidade",     MCAT_CRAFTBUILD,  0 },
    [MECH_REPAIR]       = { "Reparar",          MCAT_CRAFTBUILD,  0 },
    [MECH_ARMOR]        = { "Armadura",         MCAT_COMBAT,      0 },
    [MECH_RANGED]       = { "Combate distancia",MCAT_COMBAT,      0 },
    [MECH_MELEE]        = { "Combate corpo",    MCAT_COMBAT,      0 },
    [MECH_STEALTH]      = { "Furtividade",      MCAT_COMBAT,      0 },
    [MECH_AGGRO]        = { "Atracao zumbi",    MCAT_COMBAT,      0 },
    [MECH_NIGHT_HORDE]  = { "Horda noturna",    MCAT_COMBAT,      1 },
    [MECH_SPAWN_WAVES]  = { "Ondas de spawn",   MCAT_COMBAT,      1 },
    [MECH_LOOT]         = { "Loot",             MCAT_GATHERING,   0 },
    [MECH_TRADING]      = { "Troca",            MCAT_MULTIPLAYER, 0 },
    [MECH_RESPAWN]      = { "Renascer",         MCAT_PROGRESSION, 1 },
    [MECH_SLEEP]        = { "Dormir",           MCAT_SURVIVAL,    0 },
    [MECH_LEVELING]     = { "Nivel/XP",         MCAT_PROGRESSION, 0 },
    [MECH_STATUS_FX]    = { "Status",           MCAT_SURVIVAL,    1 },
    [MECH_COOP_LOCAL]   = { "Co-op local",      MCAT_MULTIPLAYER, 0 },
    [MECH_MINIMAP]      = { "Minimapa",         MCAT_MULTIPLAYER, 0 },
};

/* PRNG local para clima/loot. */
static u32 s_rng = 0x1234567u;
static u32 rng(void)
{
    u32 x = s_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return s_rng = x;
}

void mechanics_boot(void)
{
    memset(&g_env, 0, sizeof g_env);
    g_env.time_of_day  = DAY_TICKS / 4;    /* comeca de manha */
    g_env.day_count    = 1;
    g_env.weather      = WX_CLEAR;
    g_env.ambient_temp = 550;
    g_env.weather_timer = 1800;
    s_rng = 0xA5A5F00Du;
}

int env_is_night(void) { return g_env.is_night; }

int env_light_level(void)
{
    /* Curva triangular: pico ao meio-dia, fundo a meia-noite. */
    u32 t = g_env.time_of_day;
    s32 noon_dist = (s32)t - (s32)(DAY_TICKS / 2);
    if (noon_dist < 0) noon_dist = -noon_dist;
    s32 l = 255 - (noon_dist * 255 / (DAY_TICKS / 2));
    if (g_env.weather == WX_STORM) l -= 80;
    else if (g_env.weather == WX_RAIN || g_env.weather == WX_FOG) l -= 40;
    if (l < 0) l = 0;
    if (l > 255) l = 255;
    return l;
}

static void env_tick(void)
{
    if (++g_env.time_of_day >= DAY_TICKS) {
        g_env.time_of_day = 0;
        g_env.day_count++;
    }
    u8 was_night = g_env.is_night;
    g_env.is_night = (env_light_level() < 70);

    /* Transicao de clima. */
    if (g_env.weather_timer == 0) {
        u32 r = rng() % 100;
        g_env.weather = (r < 55) ? WX_CLEAR :
                        (r < 80) ? WX_RAIN  :
                        (r < 92) ? WX_FOG   : WX_STORM;
        g_env.weather_timer = 1200 + (rng() % 2400);
    } else {
        g_env.weather_timer--;
    }

    /* Temperatura ambiente: dia quente, noite fria, chuva esfria. */
    s16 base = g_env.is_night ? 380 : 560;
    if (g_env.weather == WX_RAIN)  base -= 60;
    if (g_env.weather == WX_STORM) base -= 110;
    g_env.ambient_temp = base;

    /* Borda de subida da noite: dispara horda noturna. */
    if (!was_night && g_env.is_night) {
        extern player_t g_players[MAX_PLAYERS];
        int waves = 6 + g_env.day_count;       /* escala com os dias */
        if (waves > 24) waves = 24;
        for (int p = 0; p < MAX_PLAYERS; ++p) {
            if (!g_players[p].active || g_players[p].state == PSTATE_DEAD)
                continue;
            for (int w = 0; w < waves; ++w) {
                fx_t ang = (fx_t)(rng() & 0xFFFF);
                fx_t r   = FX_FROM_INT(18 + (rng() % 12));
                /* offset radial aproximado via tabela de quadrantes. */
                fx_t ox = ((ang & 0x4000) ? r : -r);
                fx_t oz = ((ang & 0x8000) ? r : -r);
                zombie_spawn(g_players[p].pos.x + ox,
                             g_players[p].pos.z + oz);
            }
        }
    }
}

/* Temperatura efetiva considerando fogueiras proximas e umidade. */
static void player_thermal(player_t *p)
{
    s16 target = g_env.ambient_temp;

    /* Calor de fogueira/forja perto (raio 6 m). */
    if (structures_near(ST_CAMPFIRE, p->pos.x, p->pos.z, FX_FROM_INT(6)) >= 0 ||
        structures_near(ST_FORGE,    p->pos.x, p->pos.z, FX_FROM_INT(6)) >= 0)
        target += 250;

    /* Estar molhado puxa para o frio. */
    target -= (s16)(p->wetness);

    /* Tocha equipada aquece um pouco. */
    if (inv_equipped(&p->inv) == ITEM_TORCH) target += 60;

    if (p->temperature < target)      p->temperature += 2;
    else if (p->temperature > target) p->temperature -= 2;
    if (p->temperature < 0)    p->temperature = 0;
    if (p->temperature > 1000) p->temperature = 1000;

    /* Extremos causam dano. */
    if (p->temperature < 150 || p->temperature > 900) {
        static u32 acc = 0; acc += 20;
        if (acc >= 2000) { p->hp -= 3; acc = 0; }
    }
}

static void player_survival(player_t *p, u32 dt_ms)
{
    /* Stamina: corrida gasta, repouso recupera. */
    if (p->state == PSTATE_RUN) {
        p->stamina -= 5;
        if (p->stamina <= 0) { p->stamina = 0; p->state = PSTATE_WALK; }
    } else {
        if (p->stamina < 1000) p->stamina += (p->state == PSTATE_IDLE) ? 6 : 3;
        if (p->stamina > 1000) p->stamina = 1000;
    }

    /* Umidade pela chuva; seca perto do fogo. */
    if (g_env.weather == WX_RAIN || g_env.weather == WX_STORM) {
        if (p->wetness < 255) p->wetness += 2;
    } else if (structures_near(ST_CAMPFIRE, p->pos.x, p->pos.z, FX_FROM_INT(5)) >= 0) {
        if (p->wetness > 3) p->wetness -= 3; else p->wetness = 0;
    } else if (p->wetness > 0) {
        p->wetness--;
    }

    player_thermal(p);

    /* Sangramento: dano continuo, decai devagar. */
    if (p->bleeding) {
        static u32 acc = 0; acc += dt_ms;
        if (acc >= 1000) { p->hp -= 2; if (p->bleeding) p->bleeding--; acc = 0; }
    }

    /* Infeccao: progride; alta drena vida. */
    if (p->infection) {
        static u32 acc = 0; acc += dt_ms;
        if (acc >= 3000) {
            if (p->infection < 255) p->infection++;
            if (p->infection > 160) p->hp -= 4;
            acc = 0;
        }
    }

    /* Regeneracao quando bem alimentado, hidratado e sem sangrar. */
    if (p->hunger > 300 && p->thirst > 300 && !p->bleeding && p->hp < 1000) {
        static u32 acc = 0; acc += dt_ms;
        if (acc >= 1500) { p->hp += 2; if (p->hp > 1000) p->hp = 1000; acc = 0; }
    }

    if (p->hp <= 0) { p->hp = 0; p->state = PSTATE_DEAD; }
}

void mechanics_tick(u32 dt_ms)
{
    extern player_t g_players[MAX_PLAYERS];
    env_tick();
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        player_t *p = &g_players[i];
        if (!p->active || p->state == PSTATE_DEAD) continue;
        player_survival(p, dt_ms);
    }
}

/* ---------------- eventos ---------------- */

void mech_on_bite(player_t *p, u8 zombie_dmg)
{
    /* Armadura absorve parte do dano. */
    u8 dmg = zombie_dmg;
    if (p->armor_item && p->armor_dura) {
        u8 absorb = dmg / 2;
        dmg -= absorb;
        p->armor_dura = (p->armor_dura > absorb) ? p->armor_dura - absorb : 0;
        if (!p->armor_dura) p->armor_item = ITEM_NONE;
    }
    p->hp -= dmg;
    if (p->bleeding < 200) p->bleeding += 40;
    if ((rng() & 3) == 0 && p->infection < 200) p->infection += 25; /* 25% */
}

void mech_on_noise(fx_t x, fx_t z, u8 loudness)
{
    extern zombie_t g_zombies[MAX_ZOMBIES];
    fx_t radius = FX_FROM_INT(loudness);     /* loudness ~ metros */
    fx_t r2 = FX_MUL(radius, radius);
    for (int i = 0; i < MAX_ZOMBIES; ++i) {
        zombie_t *zo = &g_zombies[i];
        if (!zo->active || zo->state == ZSTATE_DEAD) continue;
        fx_t dx = zo->pos.x - x, dz = zo->pos.z - z;
        if (FX_MUL(dx, dx) + FX_MUL(dz, dz) <= r2)
            zo->state = ZSTATE_CHASE;        /* acorda e investiga */
    }
}

int mech_consume(player_t *p, item_id_t item)
{
    s16 hunger = 0, thirst = 0, heal = 0;
    u8  clears_bleed = 0, clears_infect = 0;

    switch (item) {
    case ITEM_BERRY:        hunger = 80;  break;
    case ITEM_MUSHROOM:     hunger = 60;  break;
    case ITEM_MEAT_COOKED:  hunger = 300; heal = 20; break;
    case ITEM_MEAT_RAW:     hunger = 150; if ((rng()&1)) p->infection += 10; break;
    case ITEM_FISH_COOKED:  hunger = 260; break;
    case ITEM_FISH_RAW:     hunger = 120; break;
    case ITEM_CANNED_FOOD:  hunger = 350; thirst = 40; break;
    case ITEM_CROP:         hunger = 120; thirst = 40; break;
    case ITEM_WATER_CLEAN:  thirst = 400; break;
    case ITEM_WATER_DIRTY:  thirst = 250; if ((rng()%3)==0) p->infection += 15; break;
    case ITEM_BANDAGE:      clears_bleed = 1; heal = 30; break;
    case ITEM_MEDKIT:       clears_bleed = 1; heal = 250; break;
    case ITEM_ANTIBIOTIC:   clears_infect = 1; break;
    case ITEM_PAINKILLER:   heal = 60; break;
    default: return 0;
    }

    if (!inv_remove(&p->inv, item, 1)) return 0;

    if (hunger) { p->hunger += hunger; if (p->hunger > 1000) p->hunger = 1000; }
    if (thirst) { p->thirst += thirst; if (p->thirst > 1000) p->thirst = 1000; }
    if (heal)   { p->hp += heal; if (p->hp > 1000) p->hp = 1000; }
    if (clears_bleed)  p->bleeding = 0;
    if (clears_infect) p->infection = 0;
    return 1;
}

int mech_repair(player_t *p)
{
    item_id_t eq = inv_equipped(&p->inv);
    if (!ITEM_IS_TOOL(eq)) return 0;
    if (!inv_remove(&p->inv, ITEM_REPAIR_KIT, 1)) return 0;
    p->tool_dura = g_item_defs[eq].durability;
    return 1;
}

int mech_wear_tool(player_t *p, u8 amount)
{
    item_id_t eq = inv_equipped(&p->inv);
    if (!ITEM_IS_TOOL(eq) || g_item_defs[eq].durability == 0) return 0;
    if (p->tool_dura == 0) p->tool_dura = g_item_defs[eq].durability;
    if (p->tool_dura > amount) { p->tool_dura -= amount; return 0; }
    /* quebrou */
    p->tool_dura = 0;
    inv_remove(&p->inv, eq, 1);
    return 1;
}
