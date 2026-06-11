#ifndef ZUMBI_RENDER_H
#define ZUMBI_RENDER_H

#include "types.h"
#include "math3d.h"

/* Modelo: vertices quantizados em 16 bits (cabe em half-precision
 * suficiente para escala da floresta — bbox local *  escala). */
typedef struct {
    s16 x, y, z;
    u16 _pad;
} QALIGN vtx16_t;

typedef struct {
    u16 a, b, c;
    u16 _pad;
} QALIGN tri16_t;

typedef struct {
    vtx16_t *verts;
    tri16_t *tris;
    u16      nverts;
    u16      ntris;
    float    scale;       /* desquantizacao: pos = vtx * scale */
    u32      tex_id;      /* indice em texture_atlas */
    float    radius;      /* bounding sphere para culling */
} model_t;

typedef enum {
    MDL_TREE_PINE = 0,
    MDL_TREE_OAK,
    MDL_TREE_BUSH,
    MDL_PLAYER,
    MDL_ZOMBIE,
    MDL_ROCK,
    MDL_CRATE,
    MDL_COUNT
} model_id_t;

typedef struct {
    model_id_t model;
    vec3_t     pos;
    u16        rot_y;    /* 0..65535 = 0..2pi */
    u8         lod;
    u8         flags;
} draw_cmd_t;

void render_boot(void);

/* Por frame: */
void render_begin       (const mat4_t *view, const mat4_t *proj);
void render_submit      (const draw_cmd_t *cmd);
void render_flush       (void);

/* Pipeline interno:
 *   EE  : frustum cull + LOD + batching por modelo/textura
 *   VU1 : recebe lista de matrizes e blocos de vertices, transforma,
 *         clipa contra near plane, emite GIF packet PRIM=triangle
 *   GS  : raster com Z + texture mapping
 */

/* Stats por frame (debug overlay). */
typedef struct {
    u32 draw_calls;
    u32 vertices;
    u32 triangles;
    u32 culled;
    u32 dma_us;
} render_stats_t;
extern render_stats_t g_rstats;

#endif
