#!/usr/bin/env bash
# build_iso.sh — compila o ELF e empacota a ISO bootavel do PS2.
#
# Preferimos mkps2iso (gera UDF+ISO9660 que o PCSX2 e o console real
# bootam direto). Se nao estiver instalado, cai para mkisofs/genisoimage.
#
# Uso:
#   tools/build_iso.sh            # make + ISO
#   tools/build_iso.sh --no-build # so empacota (ELF ja compilado)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

ELF="bin/zumbi.elf"
ISO="bin/zumbi.iso"

if [[ "${1:-}" != "--no-build" ]]; then
    echo ">> Compilando ELF (make)..."
    make
fi

if [[ ! -f "$ELF" ]]; then
    echo "!! $ELF nao existe. Rode 'make' com o ps2dev no ambiente." >&2
    exit 1
fi

mkdir -p bin

if command -v mkps2iso >/dev/null 2>&1; then
    echo ">> Empacotando com mkps2iso (UDF bootavel)..."
    mkps2iso zumbi.xml
elif command -v genisoimage >/dev/null 2>&1 || command -v mkisofs >/dev/null 2>&1; then
    GEN="$(command -v genisoimage || command -v mkisofs)"
    echo ">> mkps2iso ausente — usando $GEN (fallback ISO9660)..."
    # Monta uma arvore temporaria com os nomes em maiusculo que o PS2 espera.
    STAGE="$(mktemp -d)"
    cp SYSTEM.CNF "$STAGE/SYSTEM.CNF"
    cp "$ELF"     "$STAGE/ZUMBI.ELF"
    [[ -d assets ]] && cp -r assets "$STAGE/ASSETS"
    "$GEN" -l -o "$ISO" -V ZUMBI -sysid PLAYSTATION "$STAGE"
    rm -rf "$STAGE"
else
    echo "!! Nem mkps2iso nem genisoimage/mkisofs encontrados." >&2
    echo "   Instale um deles ou use o ImgBurn no Windows (modo Build, UDF)." >&2
    exit 1
fi

echo ">> Pronto: $ISO"
