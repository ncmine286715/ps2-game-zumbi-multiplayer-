# PS2 Zumbi Multiplayer

Base de um jogo de sobrevivencia/zumbi multiplayer para PlayStation 2.
Primeiro cenario: floresta. Sistemas: render 3D via GS+VU1, AI de zumbis,
inventario, crafting e netcode UDP delta sobre o adaptador de rede.

## Hardware alvo

| Componente | Spec |
|---|---|
| EE (CPU) | MIPS R5900 @ 294 MHz, 32 MB RAM |
| VU0 | Co-CPU vetorial (macro / micro) |
| VU1 | Pipeline paralelo ao GS (16 KB micro + 16 KB data) |
| GS | 4 MB eDRAM, 2.4 Gpix/s fill |
| IOP | R3000 @ 36 MHz, 2 MB RAM, I/O e som |
| DEV9/SMAP | Adaptador de rede 100 Mbit |
| Scratchpad | 16 KB SRAM dentro do EE (1 ciclo) |

## Toolchain

Necessario [ps2dev](https://github.com/ps2dev/ps2dev) instalado, com as
variaveis `PS2SDK`, `PS2DEV` e `GSKIT` no ambiente.

```
make            # gera bin/zumbi.elf
make iso        # empacota como ISO bootavel
make run        # roda em PCSX2 (se PCSX2= no env)
```

## Layout do projeto

```
include/    headers publicos de cada subsistema
src/        codigo do EE (jogo + render + net)
vu1/        microcode VU1 (.vsm) para transform & lighting
iop/        modulos IOP (rede, controle, som) carregados em runtime
assets/     texturas (CLUT 4/8-bit), modelos (.mdl quantizado), sons
docs/       ARCHITECTURE.md, PROTOCOL.md, MEMORY.md
```

## Subsistemas

- `memory.c`   pools + zonas + uso explicito da scratchpad
- `math3d.c`   fixed-point Q16.16, matrizes 4x4, frustum
- `gs.c`       init do GS, framebuffer duplo, DMA chains GIF
- `render.c`   pipeline EE -> VU1 -> GS, culling, batching
- `world.c`    grid de floresta, geracao de arvores, LOD
- `player.c`   movimento, camera, raycast de interacao
- `zombie.c`   pool de zumbis, FSM, flowfield de pathfinding
- `inventory.c` slots, stacks, recipes de craft
- `network.c`  UDP, snapshots delta, dead reckoning, lockstep input
- `input.c`    leitura pad via libpad
- `audio.c`    submissao para o IOP via SIF RPC

Veja `docs/ARCHITECTURE.md` para o pipeline completo de um frame.
