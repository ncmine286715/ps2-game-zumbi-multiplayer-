# Makefile — Zumbi Multiplayer (PlayStation 2 / EE + VU1)
#
# Build autossuficiente e determinístico. NÃO inclui os samples do PS2SDK
# (Makefile.pref / Makefile.eeglobal) de propósito: eles redefinem a regra
# de $(EE_BIN) a partir de $(EE_OBJS) e sobrescrevem o link, causando
# "overriding recipe"/regra duplicada e ignorando o microcode VU1.
#
# Requisitos no ambiente:
#   PS2SDK  -> raiz do ps2sdk (headers, libs, ee/startup/linkfile)
#   GSKIT   -> raiz do gsKit (gskit/dmakit)
#   toolchain ps2dev no PATH (mips64r5900el-ps2-elf-* e dvp-as)
#
# Uso:
#   make            gera bin/zumbi.elf
#   make iso        gera bin/zumbi.iso (via tools/build_iso.sh)
#   make run        roda no PCSX2 (defina PCSX2= no ambiente)
#   make clean      remove objetos e bin/
# ---------------------------------------------------------------------------

# --- Toolchain (tudo overridável por env/linha de comando) -----------------
# Prefixo do ps2dev moderno; toolchains antigas: faça EE_PREFIX=ee-
EE_PREFIX      ?= mips64r5900el-ps2-elf-
EE_CC          ?= $(EE_PREFIX)gcc
EE_LD          ?= $(EE_PREFIX)gcc
DVP_AS         ?= dvp-as
PCSX2          ?= pcsx2

# --- Sanidade de ambiente --------------------------------------------------
ifeq ($(strip $(PS2SDK)),)
$(error PS2SDK nao definido. Exporte PS2SDK (ex: export PS2SDK=$$PS2DEV/ps2sdk))
endif
ifeq ($(strip $(GSKIT)),)
$(error GSKIT nao definido. Exporte GSKIT (ex: export GSKIT=$$PS2DEV/gsKit))
endif

LINKFILE       := $(PS2SDK)/ee/startup/linkfile

# --- Saída -----------------------------------------------------------------
BINDIR         := bin
EE_BIN         := $(BINDIR)/zumbi.elf

# --- Objetos ---------------------------------------------------------------
EE_OBJS         = src/main.o src/memory.o src/math3d.o src/gs.o src/render.o \
                  src/world.o src/player.o src/zombie.o src/inventory.o \
                  src/network.o src/input.o src/audio.o src/spatial.o \
                  src/scratchpad.o src/crafting.o src/structures.o \
                  src/mechanics.o

# Microcode VU1 montado com dvp-as e linkado como blob no ELF.
VU1_OBJS        = vu1/transform.o vu1/lighting.o

OBJS           := $(EE_OBJS) $(VU1_OBJS)
DEPS           := $(EE_OBJS:.o=.dep)

# --- Includes / flags ------------------------------------------------------
EE_INCS         = -Iinclude -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include \
                  -I$(GSKIT)/include

# IMPORTANTE: NADA de -mlong64 aqui. A ABI da EE e n32 (long de 32 bits) e
# -mlong64 quebra com "-mabi=n32 is incompatible with -mlong64".
EE_CFLAGS       = -D_EE -O2 -G0 -Wall -fno-strict-aliasing -mno-gpopt \
                  $(EE_INCS) -MMD -MP -MF $(@:.o=.dep)

EE_LDFLAGS      = -T$(LINKFILE) -L$(PS2SDK)/ee/lib -L$(GSKIT)/lib

EE_LIBS         = -lgskit -ldmakit -lpacket -lpad -lkernel -ldebug \
                  -lps2ip -lnetman -lc -lpthreadglue

# ---------------------------------------------------------------------------
.PHONY: all iso run clean
.DEFAULT_GOAL := all

all: $(EE_BIN)

# Link único e explícito (inclui os blobs VU1). bin/ garantido via order-only.
$(EE_BIN): $(OBJS) | $(BINDIR)
	$(EE_LD) $(EE_LDFLAGS) -o $@ $(OBJS) $(EE_LIBS)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compilação C com geração de dependências de headers.
src/%.o: src/%.c
	$(EE_CC) $(EE_CFLAGS) -c $< -o $@

# Microcode VU1: dvp-as gera objeto EE-linkável diretamente (exporta os
# símbolos VU1*_CodeStart/End usados em src/render.c).
vu1/%.o: vu1/%.vsm
	$(DVP_AS) -o $@ $<

iso: $(EE_BIN)
	tools/build_iso.sh --no-build

run: $(EE_BIN)
	$(PCSX2) --elf $(EE_BIN)

clean:
	rm -f $(OBJS) $(DEPS) vu1/*.vo
	rm -rf $(BINDIR)

-include $(DEPS)
