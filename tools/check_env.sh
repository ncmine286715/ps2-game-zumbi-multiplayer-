#!/usr/bin/env bash
# check_env.sh — diagnóstico do ambiente de build PS2 (ps2dev/PS2SDK/GSKIT).
#
# Detecta: variáveis ausentes, toolchain fora do PATH, PS2SDK incompleto
# (sem linkfile/libs) e flags globais perigosas no ambiente (ex: -mlong64
# vazado por CFLAGS/EE_CFLAGS exportadas) que quebram o build com o erro
# "-mabi=n32 is incompatible with -mlong64".
#
# Uso: tools/check_env.sh   (sai !=0 se algo crítico estiver errado)
set -uo pipefail

ok()   { printf '  [ok]   %s\n' "$*"; }
warn() { printf '  [warn] %s\n' "$*"; WARN=$((WARN+1)); }
bad()  { printf '  [FAIL] %s\n' "$*"; FAIL=$((FAIL+1)); }

WARN=0; FAIL=0
echo "== Diagnóstico do ambiente PS2 =="

# --- Variáveis de ambiente -------------------------------------------------
echo "-- Variáveis --"
for v in PS2DEV PS2SDK GSKIT; do
    val="${!v:-}"
    if [[ -z "$val" ]]; then
        bad "$v não definido"
    elif [[ ! -d "$val" ]]; then
        bad "$v=$val não é um diretório existente"
    else
        ok "$v=$val"
    fi
done

# --- Integridade do PS2SDK -------------------------------------------------
echo "-- PS2SDK --"
if [[ -n "${PS2SDK:-}" && -d "${PS2SDK:-/nonexistent}" ]]; then
    [[ -f "$PS2SDK/ee/startup/linkfile" ]] \
        && ok "linkfile presente" || bad "linkfile ausente (PS2SDK corrompido/incompleto)"
    [[ -d "$PS2SDK/ee/lib" ]] \
        && ok "ee/lib presente" || bad "ee/lib ausente"
    [[ -f "$PS2SDK/ee/lib/libkernel.a" ]] \
        && ok "libkernel.a presente" || warn "libkernel.a ausente (libs não buildadas?)"
else
    warn "pulando checagem de integridade (PS2SDK não acessível)"
fi

# --- gsKit -----------------------------------------------------------------
echo "-- gsKit --"
if [[ -n "${GSKIT:-}" && -d "${GSKIT:-/nonexistent}" ]]; then
    [[ -f "$GSKIT/lib/libgskit.a" ]] \
        && ok "libgskit.a presente" || warn "libgskit.a ausente"
fi

# --- Toolchain no PATH -----------------------------------------------------
echo "-- Toolchain --"
for tool in mips64r5900el-ps2-elf-gcc dvp-as; do
    if command -v "$tool" >/dev/null 2>&1; then
        ok "$tool -> $(command -v "$tool")"
    elif [[ "$tool" == mips64r5900el-ps2-elf-gcc ]] && command -v ee-gcc >/dev/null 2>&1; then
        warn "toolchain antiga (ee-gcc). Use 'make EE_PREFIX=ee-'"
    else
        bad "$tool não encontrado no PATH"
    fi
done

# --- Flags globais perigosas vazadas no ambiente ---------------------------
echo "-- Flags de ambiente --"
for f in CFLAGS CXXFLAGS EE_CFLAGS EE_CXXFLAGS; do
    val="${!f:-}"
    if [[ "$val" == *"-mlong64"* ]]; then
        bad "$f exporta -mlong64 — INCOMPATÍVEL com a ABI n32 da EE. Faça: unset $f"
    elif [[ -n "$val" ]]; then
        warn "$f está exportado ('$val') — pode sobrescrever o build. Considere unset."
    fi
done

echo "== Resumo: $FAIL falha(s), $WARN aviso(s) =="
[[ "$FAIL" -eq 0 ]]
