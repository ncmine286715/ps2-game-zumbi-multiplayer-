# Arquitetura — pipeline de um frame

```
                   +-------------------+
   pad input  ---->| EE: input_tick    |
                   +---------+---------+
                             |
            (estado de jogo) v
   +-----------+    +-------------------+    +------------------+
   | net recv  +--->| EE: simulate tick +--->| spatial_grid     |
   +-----------+    | (player/zombie/   |    +------------------+
                    |  world/inventory) |
                    +---------+---------+
                              |
                              v
                    +-------------------+
                    | EE: render_begin  |
                    | frustum + cull    |
                    +---------+---------+
                              |
                              v   draw_cmd_t[] sorted by (model, lod)
                    +-------------------+
                    | EE: render_flush  |---> dma_chain ---+
                    +-------------------+                  |
                                                           v
                                              +------------------------+
                                              | VIF1 -> VU1 (microcode)|
                                              |  transform + lighting  |
                                              |  emit GIF packet       |
                                              +-----------+------------+
                                                          |
                                                          v
                                              +------------------------+
                                              | GS: triangle raster +Z |
                                              +-----------+------------+
                                                          |
                                                          v vsync
                                                  framebuffer swap
```

## Caminhos de DMA

| canal | uso |
|---|---|
| toSPR / fromSPR | EE <-> Scratchpad para hot data e GIF packets |
| VIF1            | EE -> VU1 (microcode + matrizes + vertices) |
| GIF             | VU1 -> GS (raster commands); ou EE -> GS (UI) |
| SIF             | EE <-> IOP (audio, controle, rede) |

## Por que VU1 e a alma do PS2

A EE sozinha empurra ~ 200k triangulos/s. Movendo transform+lighting
para o VU1 (rodando em paralelo com o GS) chegamos perto de 1.5M
triangulos/s mesmo com lighting basico. As "ondulacoes" do God of War
e a multidao de Shadow of the Colossus saem dai.

## Memoria por subsistema

Ver `docs/MEMORY.md` para o orcamento detalhado em main RAM, VRAM e
SPR.
