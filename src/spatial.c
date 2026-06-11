#include "spatial.h"
#include <string.h>

spatial_grid_t g_grid;

static inline int cell_of(fx_t v)
{
    /* world em metros, centralizado em (0,0). Celula = 8 m. */
    int c = (FX_TO_INT(v) + (GRID_DIM * 8 / 2)) >> 3;
    if (c < 0) c = 0;
    if (c >= GRID_DIM) c = GRID_DIM - 1;
    return c;
}

void spatial_reset(void)
{
    memset(g_grid.count, 0, sizeof g_grid.count);
}

void spatial_insert(ent_kind_t kind, u16 id, fx_t x, fx_t z)
{
    int cx = cell_of(x), cz = cell_of(z);
    u8 *n = &g_grid.count[cz][cx];
    if (*n >= GRID_BUCKET) return;     /* descartar — overflow defensivo */
    g_grid.cells[cz][cx][*n].kind = (u16)kind;
    g_grid.cells[cz][cx][*n].id   = id;
    (*n)++;
}

int spatial_query(fx_t x, fx_t z, fx_t radius, grid_entry_t *out, int max)
{
    int cx = cell_of(x), cz = cell_of(z);
    int r  = (FX_TO_INT(radius) + 7) >> 3;
    int written = 0;
    for (int dz = -r; dz <= r && written < max; ++dz)
    for (int dx = -r; dx <= r && written < max; ++dx) {
        int gx = cx + dx, gz = cz + dz;
        if (gx < 0 || gx >= GRID_DIM || gz < 0 || gz >= GRID_DIM) continue;
        u8 n = g_grid.count[gz][gx];
        for (u8 i = 0; i < n && written < max; ++i)
            out[written++] = g_grid.cells[gz][gx][i];
    }
    return written;
}
