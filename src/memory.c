#include "memory.h"
#include <string.h>
#include <stdio.h>

zone_t g_zone_static;
zone_t g_zone_level;
zone_t g_zone_frame;

/* Layout em main RAM (32 MB total — descontando ~4 MB para EE kernel,
 * stacks, libc, drivers): */
#define STATIC_SIZE    (4  * 1024 * 1024)
#define LEVEL_SIZE     (16 * 1024 * 1024)
#define FRAME_SIZE     (2  * 1024 * 1024)

static u8 s_static[STATIC_SIZE]  QALIGN;
static u8 s_level [LEVEL_SIZE]   QALIGN;
static u8 s_frame [FRAME_SIZE]   QALIGN;

void memory_boot(void)
{
    zone_init(&g_zone_static, s_static, sizeof s_static, "static");
    zone_init(&g_zone_level,  s_level,  sizeof s_level,  "level");
    zone_init(&g_zone_frame,  s_frame,  sizeof s_frame,  "frame");
}

void zone_init(zone_t *z, void *base, u32 size, const char *tag)
{
    z->base = (u8 *)base;
    z->size = size;
    z->used = 0;
    z->tag  = tag;
}

void *zone_alloc(zone_t *z, u32 size, u32 align)
{
    u32 mask  = align - 1;
    u32 start = (z->used + mask) & ~mask;
    if (start + size > z->size) {
        printf("zone[%s] OOM: %u/%u\n", z->tag, start + size, z->size);
        return 0;
    }
    z->used = start + size;
    return z->base + start;
}

void zone_reset(zone_t *z) { z->used = 0; }
u32  zone_high (const zone_t *z) { return z->used; }

void pool_init(pool_t *p, void *base, u32 obj_size, u32 capacity)
{
    /* Aligna obj_size para 16 (quadword). */
    obj_size      = (obj_size + 15) & ~15;
    p->base       = (u8 *)base;
    p->obj_size   = obj_size;
    p->capacity   = capacity;
    p->free_list  = (u32 *)((u8 *)base + obj_size * capacity);
    p->free_count = capacity;
    for (u32 i = 0; i < capacity; ++i)
        p->free_list[i] = capacity - 1 - i;
    memset(base, 0, obj_size * capacity);
}

void *pool_get(pool_t *p)
{
    if (!p->free_count) return 0;
    u32 idx = p->free_list[--p->free_count];
    return p->base + idx * p->obj_size;
}

void pool_release(pool_t *p, void *obj)
{
    u32 idx = pool_index(p, obj);
    if (idx >= p->capacity) return;
    p->free_list[p->free_count++] = idx;
}

int pool_index(const pool_t *p, const void *obj)
{
    return ((const u8 *)obj - p->base) / p->obj_size;
}
