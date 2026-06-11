# Orcamento de memoria

## Main RAM (32 MB)

| zona | tamanho | conteudo |
|---|---:|---|
| EE kernel + libc + drivers | ~4 MB | reservado |
| zone_static                | 4 MB  | modelos, texturas decodificadas residentes |
| zone_level                 | 16 MB | floresta atual, sons cacheados, IRX |
| zone_frame                 | 2 MB  | matrizes, draw lists, lixo descartavel |
| stacks + heap libc         | ~4 MB | restante |

## GS eDRAM (4 MB)

| pagina | uso |
|---|---|
| 0    | framebuffer 0 (512x448x32, ~900 KB) |
| 14   | framebuffer 1 |
| 28   | Z buffer 24-bit |
| 42+  | texturas residentes (atlas de 512x512 CT32 = 1 MB) |

Texturas grandes sao recarregadas por TBP via DMA EE->GIF a cada frame
quando necessario; texturas comuns (terreno, casca de arvore) ficam
residentes para evitar trafego.

## Scratchpad (16 KB)

Layout fixo em `include/scratchpad.h`. Cada hot path tem seu
sub-buffer — overflow detectado em debug com canary words.

## VU1 (16 KB micro + 16 KB data)

| segmento | uso |
|---|---|
| micro    | transform.vsm + lighting.vsm (~2 KB) |
| data 0-3 | MVP matrix |
| data 4   | counters |
| data 5+  | dual buffer de vertices (4 KB cada metade) |
| data 8K+ | GIF packet em construcao |

Double buffering classico: enquanto o VU emite GIF da metade A,
DMA enche metade B com os vertices do proximo lote.

## Rede

| campo | bytes |
|---|---:|
| net_input_t   | 16 |
| net_entity_t  | 24 |
| MTU usavel    | 1200 |
| snapshot tipico | 4 + 2 + bitmap + 24 * changed |

Com 8 players + 64 zumbis e ~30% de mudancas por tick,
um snapshot delta cabe em ~700 bytes — folga confortavel pra 100 Mbit.
