/* DMA EE <-> Scratchpad usando o canal SPR. Em hardware real isto
 * usa registradores SPR_TO/SPR_FROM_MEM em 0x1000D000.
 * Mantemos a interface curta — quem chama controla o ofs/size. */

#include "scratchpad.h"
#include <dma.h>

void spr_from_mem(void *spr_ofs, const void *src, u32 size)
{
    /* Tamanho em qwords, alinhado para cima. */
    u32 qwc = (size + 15) >> 4;
    dma_channel_send_normal(DMA_CHANNEL_fromSPR, (void *)src, qwc, 0, 0);
    (void)spr_ofs;
}

void spr_to_mem(void *dst, void *spr_ofs, u32 size)
{
    u32 qwc = (size + 15) >> 4;
    dma_channel_send_normal(DMA_CHANNEL_toSPR, dst, qwc, 0, 0);
    (void)spr_ofs;
}

void spr_wait(void)
{
    dma_channel_wait(DMA_CHANNEL_toSPR,   0);
    dma_channel_wait(DMA_CHANNEL_fromSPR, 0);
}
