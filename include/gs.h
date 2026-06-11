#ifndef ZUMBI_GS_H
#define ZUMBI_GS_H

#include "types.h"
#include <gsKit.h>
#include <dmaKit.h>

/* Configuracao do framebuffer. NTSC 512x448 32bpp dobra cabe nos 4 MB
 * de eDRAM com Z buffer 24 bit e algumas paginas para textura. */
#define GS_WIDTH      512
#define GS_HEIGHT     448
#define GS_PSM        GS_PSM_CT32
#define GS_PSMZ       GS_PSMZ_24

extern GSGLOBAL *gs_global;

void gs_boot(int interlace);     /* GS_INTERLACED / GS_NONINTERLACED */
void gs_frame_begin(void);
void gs_frame_end(void);         /* VSYNC + swap */

/* Helpers para construir GIF packets em scratchpad. */
typedef struct gif_pkt {
    qword_t *cur;
    qword_t *base;
    qword_t *end;
} gif_pkt_t;

void gif_pkt_open (gif_pkt_t *p, void *spr_ofs, u32 qwc_capacity);
void gif_pkt_giftag(gif_pkt_t *p, u32 nloop, u32 eop, u64 prim, u64 regs, u32 nreg);
void gif_pkt_qword(gif_pkt_t *p, u64 lo, u64 hi);
void gif_pkt_send (gif_pkt_t *p);  /* DMA chain SPR -> GIF */

#endif
