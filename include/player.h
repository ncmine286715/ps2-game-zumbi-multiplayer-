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
    vec3_t pos;
    vec3_t vel;
    u16    yaw;           /* 0..65535 */
    u16    pitch;
    s16    hp;            /* 0..1000 */
    s16    hunger;        /* 0..1000 */
    s16    thirst;
    inventory_t inv;
    u32    last_input_tick;
} player_t;

extern player_t g_players[MAX_PLAYERS];
extern u8       g_local_player_idx;

#define PLAYER_LOCAL (&g_players[g_local_player_idx])

void player_boot(void);
void player_tick(u32 dt_ms);
void player_render(void);

/* Acao primaria: ataque/coletar com item equipado. */
void player_action_primary(player_t *p);
void player_action_secondary(player_t *p);

#endif
