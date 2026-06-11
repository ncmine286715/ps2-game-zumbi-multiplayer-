#ifndef ZUMBI_SCRATCHPAD_H
#define ZUMBI_SCRATCHPAD_H

#include "types.h"

/* O EE expoe 16 KB de SRAM mapeada em 0x70000000 com latencia de 1 ciclo.
 * Reservamos slots fixos para hot paths (math 3D, packet building,
 * snapshot delta). Nunca alocamos dinamico aqui — overflow = crash. */

#define SPR_BASE          0x70000000u
#define SPR_SIZE          (16 * 1024)

/* Layout (offsets em bytes): */
#define SPR_OFS_MATRIX    0x0000  /* 256 B  — mat4 + scratch */
#define SPR_OFS_PACKET    0x0100  /* 4 KB   — GIF packet em construcao */
#define SPR_OFS_VTXBUF    0x1100  /* 4 KB   — vertices transformados */
#define SPR_OFS_NETBUF    0x2100  /* 4 KB   — pacote de rede */
#define SPR_OFS_USER      0x3100  /* 3.75 KB — uso curto livre */

#define SPR(ofs)          ((void *)(SPR_BASE + (ofs)))

/* DMA do main RAM -> SPR usa canal SPR_FROM_MEM. */
void spr_from_mem(void *spr_ofs, const void *src, u32 size);
void spr_to_mem  (void *dst, void *spr_ofs, u32 size);
void spr_wait    (void);

#endif
