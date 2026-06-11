# PS2 Zumbi Multiplayer

Jogo de sobrevivencia/zumbi multiplayer para PlayStation 2.
Mapa procedural de **512 x 512 m** com 5 biomas, render 3D via GS+VU1,
AI de zumbis com horda noturna, inventario, **40 mecanicas de
sobrevivencia**, **40 ferramentas/armas**, sistema de **construcao de
bases**, **co-op local (ate 4 controles)** e netcode UDP delta.

Veja **[docs/MECHANICS.md](docs/MECHANICS.md)** para o catalogo completo
de mecanicas/ferramentas e **[docs/BUILD.md](docs/BUILD.md)** para gerar
a ISO bootavel.

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
make                 # gera bin/zumbi.elf
make iso             # empacota como ISO bootavel (mkps2iso/SYSTEM.CNF)
make run             # roda em PCSX2 (se PCSX2= no env)
tools/build_iso.sh   # make + ISO num passo so
```

Arquivos de empacotamento na raiz: `SYSTEM.CNF` (boot) e `zumbi.xml`
(projeto mkps2iso). Detalhes em [docs/BUILD.md](docs/BUILD.md).

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

- `memory.c`    pools + zonas + uso explicito da scratchpad
- `math3d.c`    fixed-point Q16.16, matrizes 4x4, frustum
- `gs.c`        init do GS, framebuffer duplo, DMA chains GIF
- `render.c`    pipeline EE -> VU1 -> GS, culling, batching
- `world.c`     mapa 512m, biomas, arvores/pedras/loot, geracao por seed
- `player.c`    movimento, camera, coleta/mineracao, build, co-op local
- `zombie.c`    pool de zumbis, FSM, flowfield, buff noturno
- `inventory.c` slots, stacks, 100+ itens (40 ferramentas)
- `crafting.c`  receitas (refino, ferramentas, estruturas, armadura)
- `structures.c` construcoes colocaveis e estacoes de craft
- `mechanics.c` 40 mecanicas: sobrevivencia, ambiente, combate de suporte
- `network.c`   UDP, snapshots delta, dead reckoning, lockstep input
- `input.c`     leitura de ate 4 pads via libpad (co-op local)
- `audio.c`     submissao para o IOP via SIF RPC

Veja `docs/ARCHITECTURE.md` para o pipeline completo de um frame.
