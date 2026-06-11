/* network.c — netcode UDP via ps2ip + SMAP. */

#include "network.h"
#include "player.h"
#include "zombie.h"
#include "input.h"
#include "scratchpad.h"
#include <string.h>
#include <stdio.h>

#include <ps2ip.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int             s_sock        = -1;
static int             s_is_host     = 0;
static int             s_connected   = 0;
static struct sockaddr_in s_host_addr;
static u32             s_tick_local  = 0;

/* Historico para delta. Mantemos 32 ticks recentes. */
static net_entity_t    s_snap_hist[NET_SNAP_BUF][MAX_PLAYERS + MAX_ZOMBIES] QALIGN;
static u8              s_snap_count[NET_SNAP_BUF];
static u32             s_snap_ticks[NET_SNAP_BUF];
static u8              s_last_ack_idx = 0;

static void load_ps2ip_modules(void)
{
    /* Em runtime real: LoadStartModule de SIO2MAN, NETMAN, SMAP_INGAME
     * e ps2ip via SifLoadModule. Aqui assumimos pre-carregados pelo IRX
     * loader (iop/loader.c) — interface idempotente. */
}

void net_boot(int as_host, const char *host_ip)
{
    s_is_host = as_host;
    load_ps2ip_modules();

    s_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock < 0) { printf("net: socket failed\n"); return; }

    struct sockaddr_in bind_addr;
    bind_addr.sin_family      = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port        = htons(as_host ? NET_PORT_HOST : 0);
    if (bind(s_sock, (struct sockaddr *)&bind_addr, sizeof bind_addr) < 0) {
        printf("net: bind failed\n");
        s_sock = -1; return;
    }

    if (!as_host) {
        memset(&s_host_addr, 0, sizeof s_host_addr);
        s_host_addr.sin_family = AF_INET;
        s_host_addr.sin_port   = htons(NET_PORT_HOST);
        s_host_addr.sin_addr.s_addr = inet_addr(host_ip);
        u8 hello = PKT_HELLO;
        sendto(s_sock, &hello, 1, 0,
               (struct sockaddr *)&s_host_addr, sizeof s_host_addr);
    }
    s_connected = 1;
}

int net_is_host(void)      { return s_is_host;   }
int net_is_connected(void) { return s_connected; }

/* ---- gather: extrai net_entity_t a partir do estado autoritativo ---- */
static u32 snapshot_gather(net_entity_t *out)
{
    u32 n = 0;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (!g_players[i].active) continue;
        net_entity_t *e = &out[n++];
        e->kind  = ENT_PLAYER;
        e->id    = g_players[i].net_id;
        e->yaw   = g_players[i].yaw;
        e->x     = (s16)(FX_TO_INT(g_players[i].pos.x) * 10);
        e->y     = 0;
        e->z     = (s16)(FX_TO_INT(g_players[i].pos.z) * 10);
        e->vx    = (s16)(FX_TO_INT(g_players[i].vel.x) * 10);
        e->vz    = (s16)(FX_TO_INT(g_players[i].vel.z) * 10);
        e->vy    = 0;
        e->hp    = (u16)g_players[i].hp;
        e->state = g_players[i].state;
        e->anim  = 0;
    }
    for (int i = 0; i < MAX_ZOMBIES; ++i) {
        if (!g_zombies[i].active) continue;
        net_entity_t *e = &out[n++];
        e->kind  = ENT_ZOMBIE;
        e->id    = (u8)i;
        e->yaw   = g_zombies[i].yaw;
        e->x     = (s16)(FX_TO_INT(g_zombies[i].pos.x) * 10);
        e->y     = 0;
        e->z     = (s16)(FX_TO_INT(g_zombies[i].pos.z) * 10);
        e->vx    = (s16)(FX_TO_INT(g_zombies[i].vel.x) * 10);
        e->vz    = (s16)(FX_TO_INT(g_zombies[i].vel.z) * 10);
        e->vy    = 0;
        e->hp    = (u16)g_zombies[i].hp;
        e->state = g_zombies[i].state;
        e->anim  = g_zombies[i].anim_frame;
    }
    return n;
}

/* Delta: para cada entidade do snapshot atual, procura mesma id+kind
 * no baseline. Se igual, marca um bitmap; senao serializa completo.
 * Layout (byte-stream):
 *   u16 ncur
 *   bitmap[ceil(ncur/8)]  — 1 = changed
 *   net_entity_t[changed_count] em ordem
 *
 * O receptor reconstroi mantendo identicos os nao-marcados. */
u32 net_delta_encode(void *dst, u32 dst_cap,
                     const net_entity_t *base, u32 nbase,
                     const net_entity_t *cur,  u32 ncur)
{
    u8 *p = (u8 *)dst;
    if (dst_cap < 2 + ((ncur + 7) >> 3)) return 0;
    *(u16 *)p = (u16)ncur; p += 2;
    u8 *bm = p; u32 bm_len = (ncur + 7) >> 3;
    memset(bm, 0, bm_len); p += bm_len;

    for (u32 i = 0; i < ncur; ++i) {
        const net_entity_t *b = 0;
        for (u32 j = 0; j < nbase; ++j) {
            if (base[j].kind == cur[i].kind && base[j].id == cur[i].id) {
                b = &base[j]; break;
            }
        }
        int changed = !b || memcmp(b, &cur[i], sizeof *b) != 0;
        if (changed) {
            if ((u32)((p + sizeof(net_entity_t)) - (u8 *)dst) > dst_cap)
                return 0;
            bm[i >> 3] |= (u8)(1 << (i & 7));
            memcpy(p, &cur[i], sizeof *b);
            p += sizeof *b;
        }
    }
    return (u32)(p - (u8 *)dst);
}

u32 net_delta_decode(net_entity_t *out, u32 out_cap,
                     const void *src, u32 src_len,
                     const net_entity_t *base, u32 nbase)
{
    const u8 *p = (const u8 *)src;
    if (src_len < 2) return 0;
    u16 ncur = *(const u16 *)p; p += 2;
    if (ncur > out_cap) return 0;
    u32 bm_len = (ncur + 7) >> 3;
    const u8 *bm = p; p += bm_len;
    for (u16 i = 0; i < ncur; ++i) {
        int changed = (bm[i >> 3] >> (i & 7)) & 1;
        if (changed) {
            memcpy(&out[i], p, sizeof out[0]);
            p += sizeof out[0];
        } else {
            /* mantem baseline (mesma posicao se for ordenado igual). */
            if (i < nbase) out[i] = base[i];
            else memset(&out[i], 0, sizeof out[0]);
        }
    }
    (void)src_len;
    return ncur;
}

/* Cliente: extrapola posicao quando atrasa snapshot. */
void net_predict(net_entity_t *e, u32 dt_ms)
{
    e->x += (s16)((s32)e->vx * (s32)dt_ms / 1000);
    e->z += (s16)((s32)e->vz * (s32)dt_ms / 1000);
}

static void send_input(void)
{
    net_input_t in;
    in.buttons = g_input.buttons;
    in.move_x  = g_input.lstick_x;
    in.move_y  = g_input.lstick_y;
    in.yaw     = PLAYER_LOCAL->yaw;
    in.pitch   = PLAYER_LOCAL->pitch;
    in.tick    = s_tick_local;
    u8 buf[1 + sizeof in];
    buf[0] = PKT_INPUT;
    memcpy(buf + 1, &in, sizeof in);
    sendto(s_sock, buf, sizeof buf, 0,
           (struct sockaddr *)&s_host_addr, sizeof s_host_addr);
}

static void send_snapshot_to(struct sockaddr_in *to)
{
    u8 idx_cur  = (u8)(s_tick_local & (NET_SNAP_BUF - 1));
    u8 idx_base = s_last_ack_idx;

    u32 ncur = snapshot_gather(s_snap_hist[idx_cur]);
    s_snap_count[idx_cur] = (u8)ncur;
    s_snap_ticks[idx_cur] = s_tick_local;

    /* Pacote: header + delta. Usamos scratchpad como buffer. */
    u8 *buf = (u8 *)SPR(SPR_OFS_NETBUF);
    buf[0] = PKT_SNAPSHOT;
    buf[1] = (u8)s_snap_ticks[idx_base];
    *(u16 *)(buf + 2) = (u16)s_tick_local;

    u32 plen = net_delta_encode(buf + 4, NET_MTU - 4,
                                s_snap_hist[idx_base], s_snap_count[idx_base],
                                s_snap_hist[idx_cur],  ncur);
    if (!plen) {
        /* delta nao coube — manda full. */
        buf[1] = 0;
        plen = net_delta_encode(buf + 4, NET_MTU - 4, 0, 0,
                                s_snap_hist[idx_cur], ncur);
    }
    sendto(s_sock, buf, 4 + plen, 0,
           (struct sockaddr *)to, sizeof *to);
}

static void recv_pump(void)
{
    u8  buf[NET_MTU];
    struct sockaddr_in from;
    socklen_t          fl = sizeof from;
    int n;
    while ((n = recvfrom(s_sock, buf, sizeof buf, MSG_DONTWAIT,
                         (struct sockaddr *)&from, &fl)) > 0) {
        switch (buf[0]) {
        case PKT_HELLO:
            if (s_is_host) {
                u8 w[2] = { PKT_WELCOME, 1 };  /* atribui slot 1 ao cliente */
                sendto(s_sock, w, 2, 0,
                       (struct sockaddr *)&from, sizeof from);
            }
            break;
        case PKT_INPUT:
            if (s_is_host && n >= (int)(1 + sizeof(net_input_t))) {
                net_input_t in; memcpy(&in, buf + 1, sizeof in);
                /* Aplica nos players remotos — assumindo net_id em from->port */
                u8 slot = (u8)(ntohs(from.sin_port) & 7);
                if (slot < MAX_PLAYERS && g_players[slot].active) {
                    g_players[slot].yaw   = in.yaw;
                    g_players[slot].pitch = in.pitch;
                    /* movimento aplicado no tick do host com input do cliente */
                }
            }
            break;
        case PKT_SNAPSHOT:
            if (!s_is_host && n >= 4) {
                u8  base_t = buf[1];
                u16 cur_t  = *(u16 *)(buf + 2);
                u8  bidx   = base_t & (NET_SNAP_BUF - 1);
                u8  cidx   = (u8)(cur_t & (NET_SNAP_BUF - 1));
                u32 m = net_delta_decode(s_snap_hist[cidx], MAX_PLAYERS + MAX_ZOMBIES,
                                         buf + 4, n - 4,
                                         s_snap_hist[bidx], s_snap_count[bidx]);
                s_snap_count[cidx] = (u8)m;
                s_snap_ticks[cidx] = cur_t;
                s_last_ack_idx     = cidx;
                /* Aplica nos espelhos locais. */
                for (u32 i = 0; i < m; ++i) {
                    const net_entity_t *e = &s_snap_hist[cidx][i];
                    if (e->kind == ENT_PLAYER && e->id < MAX_PLAYERS) {
                        player_t *p = &g_players[e->id];
                        if (p->local) continue;
                        p->active = 1; p->net_id = e->id;
                        p->pos.x  = FX_FROM_INT(e->x / 10);
                        p->pos.z  = FX_FROM_INT(e->z / 10);
                        p->yaw    = e->yaw; p->hp = e->hp;
                        p->state  = e->state;
                    } else if (e->kind == ENT_ZOMBIE && e->id < MAX_ZOMBIES) {
                        zombie_t *z = &g_zombies[e->id];
                        z->active = (e->hp > 0); z->state = e->state;
                        z->pos.x  = FX_FROM_INT(e->x / 10);
                        z->pos.z  = FX_FROM_INT(e->z / 10);
                        z->yaw    = e->yaw; z->hp = e->hp;
                    }
                }
            }
            break;
        }
    }
}

void net_tick(u32 frame_tick)
{
    if (s_sock < 0) return;
    s_tick_local = frame_tick;
    recv_pump();
    if (s_is_host) {
        /* TODO: iterar lista de peers conectados e enviar snapshot.
         * Para a base inicial, mandamos por broadcast para o ultimo
         * remetente registrado (s_host_addr usado como peer). */
        if (s_host_addr.sin_port)
            send_snapshot_to(&s_host_addr);
    } else {
        if ((frame_tick % (TICK_HZ_INPUT_DIV)) == 0) send_input();
    }
}

