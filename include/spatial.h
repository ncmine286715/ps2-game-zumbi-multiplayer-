#ifndef ZUMBI_SPATIAL_H
#define ZUMBI_SPATIAL_H

#include "types.h"

/* Grid uniforme para queries de vizinhanca (zumbis vs players, raycast
 * de mira em arvores). 64x64 celulas com 4 entradas estaticas cada
 * cabe em 64*64*4*2 = 32 KB — ok para a main RAM. */

#define GRID_DIM      64
#define GRID_CELL_FX  FX_FROM_INT(8)   /* 8 m por celula */
#define GRID_BUCKET   4

typedef enum {
    ENT_PLAYER = 1,
    ENT_ZOMBIE = 2,
    ENT_TREE   = 3,
    ENT_ITEM   = 4,
} ent_kind_t;

typedef struct {
    u16 kind;
    u16 id;
} grid_entry_t;

typedef struct {
    grid_entry_t cells[GRID_DIM][GRID_DIM][GRID_BUCKET];
    u8  count[GRID_DIM][GRID_DIM];
} spatial_grid_t;

extern spatial_grid_t g_grid;

void spatial_reset(void);
void spatial_insert(ent_kind_t kind, u16 id, fx_t x, fx_t z);
int  spatial_query (fx_t x, fx_t z, fx_t radius,
                    grid_entry_t *out, int max);

#endif
