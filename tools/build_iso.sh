#!/usr/bin/env bash
# build_iso.sh — compila o ELF e empacota a ISO bootável do PS2.
#
# Pipeline em 3 fases bem separadas:
#   1. BUILD   -> garante bin/zumbi.elf (make), a menos que --no-build
#   2. STAGING -> monta uma árvore validada em build/iso_root com os nomes
#                 EXATOS que o PS2 espera (SYSTEM.CNF, ZUMBI.ELF, ASSETS/)
#   3. ISO     -> mkps2iso (UDF+ISO9660 bootável) ou fallback genisoimage
#
# Uso:
#   tools/build_iso.sh             # build + staging + iso
#   tools/build_iso.sh --no-build  # só staging + iso (ELF já compilado)
set -Eeuo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

ELF="bin/zumbi.elf"
ISO="bin/zumbi.iso"
STAGE="build/iso_root"
VOLID="ZUMBI"

log()  { printf '>> %s\n' "$*"; }
err()  { printf '!! %s\n' "$*" >&2; }
die()  { err "$*"; exit 1; }

# Limpeza garantida do staging mesmo em erro (set -e).
cleanup() { rm -rf "$STAGE"; }
trap cleanup EXIT

# --- Fase 1: BUILD ---------------------------------------------------------
if [[ "${1:-}" != "--no-build" ]]; then
    log "Compilando ELF (make)..."
    make
fi

# --- Validação do ELF ------------------------------------------------------
[[ -f "$ELF" ]] || die "$ELF não existe. Rode 'make' com o ps2dev no ambiente."
[[ -s "$ELF" ]] || die "$ELF está vazio (link falhou?)."

# Confere magic ELF (\x7fELF) — não empacota um arquivo inválido.
magic=$(head -c 4 "$ELF" | od -An -tx1 | tr -d ' \n')
[[ "$magic" == "7f454c46" ]] || die "$ELF não é um ELF válido (magic=$magic)."

# Se houver readelf da toolchain, confirma que é MIPS (alvo EE).
RE="$(command -v mips64r5900el-ps2-elf-readelf || command -v ee-readelf \
      || command -v readelf || true)"
if [[ -n "$RE" ]]; then
    if ! "$RE" -h "$ELF" 2>/dev/null | grep -qi 'mips'; then
        die "$ELF não aparenta ser MIPS/EE — toolchain errada no build?"
    fi
fi
log "ELF validado: $ELF"

# --- SYSTEM.CNF ------------------------------------------------------------
[[ -f SYSTEM.CNF ]] || die "SYSTEM.CNF ausente na raiz do projeto."
if ! grep -qi 'ZUMBI\.ELF' SYSTEM.CNF; then
    die "SYSTEM.CNF não aponta BOOT2 para ZUMBI.ELF — boot quebraria."
fi

# --- Fase 2: STAGING -------------------------------------------------------
# Nomes em MAIÚSCULO no root: o PS2 (ISO9660) é case-sensitive no boot.
log "Montando staging em $STAGE ..."
rm -rf "$STAGE"
mkdir -p "$STAGE"
cp SYSTEM.CNF "$STAGE/SYSTEM.CNF"
cp "$ELF"     "$STAGE/ZUMBI.ELF"
if [[ -d assets ]]; then
    mkdir -p "$STAGE/ASSETS"
    # copia conteúdo de assets/ (não a pasta) para ASSETS/
    cp -r assets/. "$STAGE/ASSETS/"
fi

# Confere a árvore mínima de boot antes de gerar a imagem.
[[ -f "$STAGE/SYSTEM.CNF" ]] || die "staging sem SYSTEM.CNF."
[[ -s "$STAGE/ZUMBI.ELF"  ]] || die "staging sem ZUMBI.ELF."
log "Árvore de boot validada (SYSTEM.CNF + ZUMBI.ELF)."

mkdir -p bin

# --- Fase 3: ISO -----------------------------------------------------------
if command -v mkps2iso >/dev/null 2>&1; then
    log "Empacotando com mkps2iso (UDF+ISO9660 bootável)..."
    # mkps2iso aceita diretório de origem -> imagem de saída.
    mkps2iso "$STAGE" "$ISO"
elif command -v genisoimage >/dev/null 2>&1 || command -v mkisofs >/dev/null 2>&1; then
    GEN="$(command -v genisoimage || command -v mkisofs)"
    err "mkps2iso ausente — usando $GEN (fallback ISO9660)."
    err "Recomendado: instale mkps2iso (https://github.com/N4gtan/mkps2iso)."
    # iso-level 2: mantém nomes de assets (até 31 chars) e os nomes de boot
    # SYSTEM.CNF/ZUMBI.ELF em maiúsculo com versão ;1, como o PS2 exige.
    "$GEN" -iso-level 2 -sysid PLAYSTATION2 -V "$VOLID" \
           -o "$ISO" "$STAGE"
else
    die "Nem mkps2iso nem genisoimage/mkisofs encontrados. Instale um deles."
fi

[[ -s "$ISO" ]] || die "ISO não foi gerada."
log "Pronto: $ISO ($(du -h "$ISO" | cut -f1))"
