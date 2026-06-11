# Makefile para Zumbi Multiplayer (PS2)
# Requer ps2sdk + ps2dev toolchain instalados.

EE_BIN          = bin/zumbi.elf
EE_OBJS         = src/main.o src/memory.o src/math3d.o src/gs.o src/render.o \
                  src/world.o src/player.o src/zombie.o src/inventory.o \
                  src/network.o src/input.o src/audio.o src/spatial.o \
                  src/scratchpad.o src/crafting.o

# Microcode VU1 linkado como blob binario no ELF.
VU1_OBJS        = vu1/transform.o vu1/lighting.o

EE_INCS         = -Iinclude -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include \
                  -I$(GSKIT)/include

EE_CFLAGS       = -O2 -G0 -Wall -Werror -fno-strict-aliasing \
                  -mno-gpopt -mlong64 $(EE_INCS)

EE_LDFLAGS      = -L$(PS2SDK)/ee/lib -L$(GSKIT)/lib

EE_LIBS         = -lgskit -ldmakit -lpacket -lpad -lkernel -ldebug \
                  -lps2ip -lnetman -lc -lpthreadglue

DVP_AS          = dvp-as
EE_AS           = ee-as

all: $(EE_BIN)

$(EE_BIN): $(EE_OBJS) $(VU1_OBJS) | bin
	$(EE_CC) -T$(PS2SDK)/ee/startup/linkfile $(EE_LDFLAGS) \
	    -o $@ $^ $(EE_LIBS)

bin:
	mkdir -p bin

# Compila microcode VU1 e empacota como objeto .o
vu1/%.o: vu1/%.vsm
	$(DVP_AS) -o $(@:.o=.vo) $<
	$(EE_AS) --defsym vsm=1 -o $@ $(@:.o=.vo)

iso: $(EE_BIN)
	mkisofs -l -o bin/zumbi.iso -V ZUMBI -sysid PLAYSTATION $(EE_BIN)

run: $(EE_BIN)
	$(PCSX2) --elf $(EE_BIN)

clean:
	rm -f $(EE_OBJS) $(VU1_OBJS) vu1/*.vo
	rm -rf bin

.PHONY: all iso run clean

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
