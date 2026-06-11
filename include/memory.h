#ifndef ZUMBI_MEMORY_H
#define ZUMBI_MEMORY_H

#include "types.h"

/* Zona linear: alocacoes sequenciais, free apenas em massa por frame
 * (parecido com bump allocator). Evita fragmentacao em 32 MB. */
typedef struct {
    u8  *base;
    u32  size;
    u32  used;
    const char *tag;
} zone_t;

void  zone_init  (zone_t *z, void *base, u32 size, const char *tag);
void *zone_alloc (zone_t *z, u32 size, u32 align);
void  zone_reset (zone_t *z);
u32   zone_high  (const zone_t *z);

/* Pool de objetos de tamanho fixo. Usado para zumbis, projeteis, itens. */
typedef struct {
    u8  *base;
    u32  obj_size;
    u32  capacity;
    u32 *free_list;   /* indices livres */
    u32  free_count;
} pool_t;

void  pool_init   (pool_t *p, void *base, u32 obj_size, u32 capacity);
void *pool_get    (pool_t *p);
void  pool_release(pool_t *p, void *obj);
int   pool_index  (const pool_t *p, const void *obj);

/* Layout das zonas globais. Total deve caber em < 28 MB
 * (resto = stack + EE kernel + heap libc). */
extern zone_t g_zone_static;   /* assets carregados no boot */
extern zone_t g_zone_level;    /* assets do mapa atual */
extern zone_t g_zone_frame;    /* lixo descartavel por frame */

void memory_boot(void);

#endif
