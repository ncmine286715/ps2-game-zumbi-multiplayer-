#ifndef ZUMBI_AUDIO_H
#define ZUMBI_AUDIO_H

#include "types.h"

typedef enum {
    SFX_HIT_WOOD = 0,
    SFX_HIT_FLESH,
    SFX_ZOMBIE_GROAN,
    SFX_PICKUP,
    SFX_CRAFT_DONE,
    SFX_FOOTSTEP,
    SFX_COUNT
} sfx_id_t;

void audio_boot(void);
void audio_play(sfx_id_t id, fx_t world_x, fx_t world_z);
void audio_set_listener(fx_t x, fx_t z, u16 yaw);

#endif
