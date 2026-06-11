#include "gs.h"
#include "scratchpad.h"
#include <gsKit.h>
#include <dmaKit.h>
#include <gsToolkit.h>

GSGLOBAL *gs_global = 0;

void gs_boot(int interlace)
{
    gs_global = gsKit_init_global();

    gs_global->Mode      = GS_MODE_NTSC;
    gs_global->Interlace = interlace;
    gs_global->Field     = GS_FIELD;
    gs_global->Width     = GS_WIDTH;
    gs_global->Height    = GS_HEIGHT;
    gs_global->PSM       = GS_PSM;
    gs_global->PSMZ      = GS_PSMZ;
    gs_global->ZBuffering = GS_SETTING_ON;
    gs_global->DoubleBuffering = GS_SETTING_ON;
    gs_global->PrimAlphaEnable = GS_SETTING_ON;
    gs_global->PrimAAEnable    = GS_SETTING_OFF;
    gs_global->Dithering       = GS_SETTING_ON;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_OFF,
                D_CTRL_STD_OFF,  D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_fromSPR);
    dmaKit_chan_init(DMA_CHANNEL_toSPR);

    gsKit_init_screen(gs_global);
    gsKit_mode_switch(gs_global, GS_PERSISTENT);
    gsKit_set_clamp(gs_global, GS_CMODE_REPEAT);
    gsKit_clear(gs_global, GS_SETREG_RGBAQ(0x08, 0x10, 0x18, 0x80, 0x00));
}

void gs_frame_begin(void)
{
    gsKit_clear(gs_global, GS_SETREG_RGBAQ(0x10, 0x20, 0x18, 0x80, 0x00));
}

void gs_frame_end(void)
{
    gsKit_queue_exec(gs_global);
    gsKit_sync_flip(gs_global);
}

/* ---- GIF packet builder ---- */

void gif_pkt_open(gif_pkt_t *p, void *spr_ofs, u32 qwc_capacity)
{
    p->base = (qword_t *)spr_ofs;
    p->cur  = p->base;
    p->end  = p->base + qwc_capacity;
}

void gif_pkt_giftag(gif_pkt_t *p, u32 nloop, u32 eop, u64 prim, u64 regs, u32 nreg)
{
    if (p->cur >= p->end) return;
    u64 tag_lo = ((u64)nloop & 0x7FFF) | ((u64)(eop & 1) << 15) |
                 ((u64)prim  << 47);
    u64 tag_hi = ((u64)nreg & 0xF) << 28 | regs;
    p->cur->q  = ((u128)tag_hi << 64) | tag_lo;
    p->cur++;
}

void gif_pkt_qword(gif_pkt_t *p, u64 lo, u64 hi)
{
    if (p->cur >= p->end) return;
    p->cur->q = ((u128)hi << 64) | lo;
    p->cur++;
}

void gif_pkt_send(gif_pkt_t *p)
{
    u32 qwc = (u32)(p->cur - p->base);
    if (!qwc) return;
    /* SPR -> GIF via cadeia simples (sem CNT/REF chains aqui — usuario
     * usa este path para uploads pequenos; geometria em massa segue
     * por DMA chain dedicada no render.c). */
    dma_channel_send_normal(DMA_CHANNEL_GIF, p->base, qwc, 0, 0);
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
}
