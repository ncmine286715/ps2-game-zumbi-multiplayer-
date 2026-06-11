#ifndef ZUMBI_TYPES_H
#define ZUMBI_TYPES_H

#include <tamtypes.h>

/* Fixed point Q16.16 — usado no logico de jogo para evitar a FPU
 * relativamente lenta do EE em comparacao com as VUs. */
typedef s32 fx_t;

#define FX_SHIFT     16
#define FX_ONE       (1 << FX_SHIFT)
#define FX_HALF      (FX_ONE >> 1)
#define FX_FROM_INT(i)   ((fx_t)((i) << FX_SHIFT))
#define FX_TO_INT(f)     ((s32)((f) >> FX_SHIFT))
#define FX_FROM_FLOAT(f) ((fx_t)((f) * (float)FX_ONE))
#define FX_TO_FLOAT(f)   ((float)(f) / (float)FX_ONE)
#define FX_MUL(a, b)     ((fx_t)(((s64)(a) * (s64)(b)) >> FX_SHIFT))
#define FX_DIV(a, b)     ((fx_t)(((s64)(a) << FX_SHIFT) / (s64)(b)))

/* Alinhamento de 128 bits exigido pelo DMA / QWC do PS2. */
#define QALIGN __attribute__((aligned(16)))

/* Quadword: 4 floats ou 4 s32. As DMAs operam em multiplos de qwords. */
typedef union {
    u128 q;
    float  f[4];
    s32    i[4];
    u32    u[4];
} QALIGN qword_t;

typedef struct { fx_t x, y, z; }        vec3_t;
typedef struct { float x, y, z, w; }    vec4_t;
typedef struct { float m[16]; }         QALIGN mat4_t;

/* Quaternion comprimido em 32 bits para netcode (xyz 10/10/10 + sinal). */
typedef u32 quat32_t;

#define MAX_PLAYERS         8
#define MAX_LOCAL_PLAYERS   4      /* co-op local de tela compartilhada */
#define MAX_ZOMBIES         128    /* hordas maiores no mapa expandido */
#define MAX_TREES           1024
#define MAX_ROCKS           256
#define MAX_ANIMALS         48
#define MAX_LOOT            96     /* baus/caixas espalhados pelo mundo */
#define MAX_STRUCTURES      256    /* construcoes colocadas pelos jogadores */
#define MAX_PROJECTILES     64     /* flechas, balas, molotovs */
#define MAX_ITEMS_GROUND    256
#define MAX_INV_SLOTS       24
#define INV_STACK_MAX       64

/* Mapa expandido: 512 x 512 m (antes 256 x 256). Continua dentro da
 * cobertura do spatial grid (64 celulas * 8 m = 512 m). */
#define WORLD_HALF_M        256
#define WORLD_SIZE_M        (WORLD_HALF_M * 2)

#endif
