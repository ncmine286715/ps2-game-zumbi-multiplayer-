#ifndef ZUMBI_ZOMBIE_H
#define ZUMBI_ZOMBIE_H

#include "types.h"

typedef enum {
    ZSTATE_IDLE = 0,
    ZSTATE_WANDER,
    ZSTATE_CHASE,
    ZSTATE_ATTACK,
    ZSTATE_STUNNED,
    ZSTATE_DEAD,
} zstate_t;

typedef struct {
    u8     active;
    u8     state;
    u8     target_player;   /* 0xFF = nenhum */
    u8     anim_frame;
    vec3_t pos;
    vec3_t vel;
    u16    yaw;
    s16    hp;
    u32    state_enter_tick;
    fx_t   sense_radius;
    fx_t   attack_radius;
} zombie_t;

extern zombie_t g_zombies[MAX_ZOMBIES];

void zombie_boot(void);
void zombie_spawn(fx_t x, fx_t z);
void zombie_tick(u32 dt_ms);
void zombie_render(void);

/* Flowfield: campo de direcoes 32x32 atualizado de 4 em 4 frames,
 * direcionando zumbis aos players sem A* por entidade. */
void zombie_flowfield_update(void);

#endif
