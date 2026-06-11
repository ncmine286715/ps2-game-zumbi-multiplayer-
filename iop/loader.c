/* iop/loader.c — carrega modulos IRX no IOP durante o boot do EE.
 *
 * Modulos necessarios:
 *   SIO2MAN   — gerencia o slot dos controles
 *   PADMAN    — leitura do pad
 *   DEV9      — driver de PCI/I/O para o adaptador de rede
 *   NETMAN    — gateway EE<->IOP para sockets
 *   SMAP      — driver Ethernet
 *   IOMANX    — filesystem extendido (memcard, hdd)
 *   LIBSD     — som
 *
 * Em ELF de producao esses modulos sao linkados como blobs no .data
 * com `bin2o` e carregados via SifExecModuleBuffer.
 */

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>

extern unsigned char SIO2MAN_irx[];   extern int size_SIO2MAN_irx;
extern unsigned char PADMAN_irx[];    extern int size_PADMAN_irx;
extern unsigned char DEV9_irx[];      extern int size_DEV9_irx;
extern unsigned char NETMAN_irx[];    extern int size_NETMAN_irx;
extern unsigned char SMAP_irx[];      extern int size_SMAP_irx;
extern unsigned char LIBSD_irx[];     extern int size_LIBSD_irx;

static int load(const char *name, void *buf, int sz, const char *args, int argc)
{
    int ret = 0, r;
    r = SifExecModuleBuffer(buf, sz, argc, (char *)args, &ret);
    printf("iop_load %-10s -> r=%d ret=%d\n", name, r, ret);
    return r;
}

void iop_boot_modules(void)
{
    SifInitRpc(0);
    /* Reseta o IOP para um estado conhecido. */
    while (!SifIopReset("", 0)) {}
    while (!SifIopSync())       {}
    SifInitRpc(0);

    load("sio2man", SIO2MAN_irx, size_SIO2MAN_irx, 0, 0);
    load("padman",  PADMAN_irx,  size_PADMAN_irx,  0, 0);
    load("dev9",    DEV9_irx,    size_DEV9_irx,    0, 0);
    load("netman",  NETMAN_irx,  size_NETMAN_irx,  0, 0);
    load("smap",    SMAP_irx,    size_SMAP_irx,    0, 0);
    load("libsd",   LIBSD_irx,   size_LIBSD_irx,   0, 0);
}
