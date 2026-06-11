#ifndef ZUMBI_PLAYER_H
#define ZUMBI_PLAYER_H

#include "types.h"
#include "inventory.h"

typedef enum {
    PSTATE_IDLE = 0,
    PSTATE_WALK,
    PSTATE_RUN,
    PSTATE_ATTACK,
    PSTATE_CRAFT,
    PSTATE_DEAD,
} pstate_t;

typedef struct {
    u8     active;        /* slot ocupado */
    u8     local;         /* 1 se for o player local */
    u8     state;
    u8     net_id;        /* id na sessao multiplayer */
    u8     pad_index;     /* qual controle (co-op local 0..3) */
    vec3_t pos;
    vec3_t vel;
    u16    yaw;           /* 0..65535 */
    u16    pitch;
    s16    hp;            /* 0..1000 */
    s16    hunger;        /* 0..1000 */
    s16    thirst;

    /* ---- atributos de sobrevivencia (mechanics.c) ---- */
    s16    stamina;       /* 0..1000 — corrida/ataque consomem */
    s16    temperature;   /* 0..1000 — 500 = normal, frio/calor afasta */
    u8     bleeding;      /* 0..255 — perde hp por tick enquanto > 0 */
    u8     infection;     /* 0..255 — mordida de zumbi; cura c/ antibiotico */
    u8     wetness;       /* 0..255 — chuva/agua; aumenta perda de calor */
    u8     armor_item;    /* item_id_t equipado como armadura (0 = nenhum) */
    u8     armor_dura;    /* durabilidade restante da armadura */
    u8     level;         /* nivel de sobrevivencia */
    u16    xp;            /* experiencia acumulada */
    u16    tool_dura;     /* durabilidade da ferramenta no slot ativo */

    inventory_t inv;
    u32    last_input_tick;
} player_t;

extern player_t g_players[MAX_PLAYERS];
extern u8       g_local_player_idx;
extern u8       g_local_player_count;   /* co-op local: 1..MAX_LOCAL_PLAYERS */

#define PLAYER_LOCAL (&g_players[g_local_player_idx])

void player_boot(void);
void player_tick(u32 dt_ms);
void player_render(void);

/* Co-op local: registra um jogador extra controlado por outro pad. */
int  player_add_local(u8 pad_index);

/* Concede XP e sobe de nivel quando passa do limiar. */
void player_grant_xp(player_t *p, u16 amount);

/* Acao primaria: ataque/coletar com item equipado. */
void player_action_primary(player_t *p);
void player_action_secondary(player_t *p);

#endif
