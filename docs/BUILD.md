# Build & empacotamento da ISO (PS2)

Este guia cobre como compilar o `zumbi.elf` e gerar uma **ISO bootavel**
para rodar no **PCSX2** ou no **PS2 real**.

## 1. Toolchain (ps2dev)

Instale o [ps2dev](https://github.com/ps2dev/ps2dev) e exporte as variaveis:

```sh
export PS2DEV=/usr/local/ps2dev
export PS2SDK=$PS2DEV/ps2sdk
export GSKIT=$PS2DEV/gsKit
export PATH=$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2SDK/bin:$PATH
```

No Windows, o caminho mais simples e usar o **WSL2 (Ubuntu)** ou o
instalador do ps2dev. Sem a toolchain o `make` nao compila o ELF.

## 2. Compilar o ELF

```sh
make            # gera bin/zumbi.elf
make clean      # limpa objetos e bin/
```

## 3. Arquivos obrigatorios na raiz

| Arquivo        | Para que serve |
|----------------|----------------|
| `SYSTEM.CNF`   | aponta o boot para `cdrom0:\ZUMBI.ELF;1` |
| `bin/zumbi.elf`| executavel compilado (vira `ZUMBI.ELF` na ISO) |
| `assets/`      | texturas, modelos, sons |
| `zumbi.xml`    | projeto do mkps2iso |

O `SYSTEM.CNF` ja esta versionado:

```
BOOT2 = cdrom0:\ZUMBI.ELF;1
VER = 1.00
VMODE = NTSC
HDDUNITPOWER = NOCHECK
```

## 4. Gerar a ISO

### Opcao A — mkps2iso (recomendado)

Feito especificamente para homebrew de PS2 (UDF + ISO9660):

```sh
# baixe/instale: https://github.com/N4gtan/mkps2iso
make iso             # chama tools/build_iso.sh
# ou direto:
mkps2iso zumbi.xml   # gera bin/zumbi.iso
```

### Opcao B — script automatico

```sh
tools/build_iso.sh           # make + ISO (usa mkps2iso, ou cai p/ genisoimage)
tools/build_iso.sh --no-build  # so empacota (ELF ja compilado)
```

### Opcao C — Windows sem linha de comando (ImgBurn)

1. Abra o **ImgBurn** em modo **Build**.
2. Saida: **UDF + ISO9660**.
3. Coloque na raiz: `SYSTEM.CNF`, `ZUMBI.ELF` e a pasta `ASSETS`.
4. Volume label: `ZUMBI`. Gere a `.iso`.

## 5. Rodar

```sh
make run                      # PCSX2 via ELF (defina PCSX2= no ambiente)
# ou abra bin/zumbi.iso no PCSX2 (System -> Boot ISO)
```

No console real, grave a ISO ou rode via **OPL** (USB/HDD/SMB).

## Solucao de problemas

- **"Cannot find SYSTEM.CNF"**: o arquivo precisa estar na raiz da ISO e
  o `BOOT2` apontar para o nome exato do ELF (`ZUMBI.ELF;1`).
- **Tela preta no PCSX2**: confira o `VMODE` (NTSC/PAL) e se o ELF foi
  compilado com o mesmo `PS2SDK` do emulador.
- **ELF nao linka**: verifique `PS2SDK`/`GSKIT` no ambiente e se todos os
  `.o` (incluindo `structures.o` e `mechanics.o`) foram compilados.
