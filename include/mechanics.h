#ifndef ZUMBI_MECHANICS_H
#define ZUMBI_MECHANICS_H

#include "types.h"
#include "inventory.h"
#include "player.h"

/* mechanics.c reune os sistemas de jogo (sobrevivencia, ambiente, combate
 * de suporte). As 40 mecanicas abaixo formam o registro consultavel em
 * runtime (nome + categoria) e documentado em docs/MECHANICS.md. As mais
 * "ativas" (1..24) sao simuladas a cada tick; as demais sao habilitadas
 * por estruturas/itens e disparadas por eventos. */

typedef enum {
    MECH_MOVE = 0,        /* 1  andar */
    MECH_SPRINT,          /* 2  correr (gasta stamina) */
    MECH_STAMINA,         /* 3  stamina/regen */
    MECH_HUNGER,          /* 4  fome */
    MECH_THIRST,          /* 5  sede */
    MECH_HEALTH_REGEN,    /* 6  regeneracao quando saciado */
    MECH_BLEEDING,        /* 7  sangramento */
    MECH_INFECTION,       /* 8  infeccao por mordida */
    MECH_TEMPERATURE,     /* 9  temperatura corporal */
    MECH_DAYNIGHT,        /* 10 ciclo dia/noite */
    MECH_WEATHER,         /* 11 clima (chuva/neblina/tempestade) */
    MECH_WETNESS,         /* 12 molhar (chuva/agua) */
    MECH_FIRE,            /* 13 fogo e calor de fogueira */
    MECH_COOKING,         /* 14 cozinhar carne/peixe */
    MECH_FISHING,         /* 15 pescar */
    MECH_FARMING,         /* 16 plantar/colher */
    MECH_FORAGING,        /* 17 coletar frutas/cogumelos */
    MECH_HUNTING,         /* 18 cacar animais */
    MECH_CHOPPING,        /* 19 cortar arvores */
    MECH_MINING,          /* 20 minerar pedras/minerio */
    MECH_CRAFTING,        /* 21 criar itens */
    MECH_BUILDING,        /* 22 construir estruturas */
    MECH_DECAY,           /* 23 decaimento de estruturas */
    MECH_DURABILITY,      /* 24 desgaste de ferramentas */
    MECH_REPAIR,          /* 25 reparar itens/estruturas */
    MECH_ARMOR,           /* 26 armadura reduz dano */
    MECH_RANGED,          /* 27 combate a distancia */
    MECH_MELEE,           /* 28 combate corpo a corpo */
    MECH_STEALTH,         /* 29 furtividade/ruido */
    MECH_AGGRO,           /* 30 atracao de zumbis por ruido */
    MECH_NIGHT_HORDE,     /* 31 horda noturna */
    MECH_SPAWN_WAVES,     /* 32 ondas de spawn */
    MECH_LOOT,            /* 33 baus/loot espalhado */
    MECH_TRADING,         /* 34 troca entre jogadores */
    MECH_RESPAWN,         /* 35 renascer */
    MECH_SLEEP,           /* 36 dormir/pular noite (cama) */
    MECH_LEVELING,        /* 37 XP/nivel */
    MECH_STATUS_FX,       /* 38 efeitos de status (veneno/buff) */
    MECH_COOP_LOCAL,      /* 39 co-op local (multiplos pads) */
    MECH_MINIMAP,         /* 40 minimapa/bussola/waypoints */
    MECH_COUNT
} mechanic_id_t;

typedef enum {
    MCAT_SURVIVAL = 0,
    MCAT_ENVIRONMENT,
    MCAT_GATHERING,
    MCAT_CRAFTBUILD,
    MCAT_COMBAT,
    MCAT_PROGRESSION,
    MCAT_MULTIPLAYER,
} mech_category_t;

typedef struct {
    const char     *name;
    mech_category_t cat;
    u8              active_sim;   /* 1 = simulado todo tick */
} mech_def_t;

extern const mech_def_t g_mechanics[MECH_COUNT];

/* ---- estado de ambiente (clima, hora) ---- */
typedef enum {
    WX_CLEAR = 0,
    WX_RAIN,
    WX_FOG,
    WX_STORM,
    WX_COUNT
} weather_t;

#define DAY_TICKS    12000      /* 1 dia ~ 4 min a 50 Hz */

typedef struct {
    u32 time_of_day;   /* 0..DAY_TICKS */
    u16 day_count;
    u8  weather;       /* weather_t */
    u8  is_night;
    s16 ambient_temp;  /* 0..1000, 500 = ameno */
    u32 weather_timer;
} env_t;

extern env_t g_env;

void mechanics_boot(void);
void mechanics_tick(u32 dt_ms);

/* Consultas de ambiente. */
int  env_is_night(void);
int  env_light_level(void);     /* 0 (escuro) .. 255 (dia claro) */

/* Eventos vindos de outros subsistemas. */
void mech_on_bite(player_t *p, u8 zombie_dmg);   /* mordida: dano+sangra+infecta */
void mech_on_noise(fx_t x, fx_t z, u8 loudness);  /* tiro/quebra atrai zumbis */
int  mech_consume(player_t *p, item_id_t item);   /* comer/beber/curar; 1 se usou */
int  mech_repair(player_t *p);                    /* repara ferramenta ativa */

/* Desgaste de ferramenta no slot ativo. Retorna 1 se quebrou. */
int  mech_wear_tool(player_t *p, u8 amount);

#endif
