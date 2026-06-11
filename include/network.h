#ifndef ZUMBI_NETWORK_H
#define ZUMBI_NETWORK_H

#include "types.h"

/* Protocolo UDP sobre ps2ip. Topologia: 1 host autoritativo + ate
 * 7 clientes. Tick fixo 20 Hz para snapshots, input 30 Hz.
 *
 * Pacotes:
 *   0x10 INPUT       client -> host    (botoes + mira)
 *   0x20 SNAPSHOT    host -> client    (delta vs ultimo ack)
 *   0x30 EVENT       host -> client    (craft, damage, drop)
 *   0x40 HELLO       client -> host
 *   0x41 WELCOME     host -> client
 *   0x50 PING/PONG   bilateral
 */

#define NET_PORT_HOST     27015
#define NET_TICK_HZ       20
#define NET_INPUT_HZ      30
#define NET_SNAP_BUF      32        /* historico para delta */
#define NET_MTU           1200
#define TICK_HZ_INPUT_DIV 2         /* 50 Hz / 2 ~ 25 Hz input */

typedef enum {
    PKT_INPUT    = 0x10,
    PKT_SNAPSHOT = 0x20,
    PKT_EVENT    = 0x30,
    PKT_HELLO    = 0x40,
    PKT_WELCOME  = 0x41,
    PKT_PING     = 0x50,
    PKT_PONG     = 0x51,
} pkt_kind_t;

typedef struct {
    u32  buttons;       /* bitmask PAD_* */
    s16  move_x;
    s16  move_y;
    u16  yaw;
    u16  pitch;
    u32  tick;
} QALIGN net_input_t;

/* Estado por entidade dentro de um snapshot. */
typedef struct {
    u8  kind;        /* ent_kind_t */
    u8  id;
    u16 yaw;
    s16 x, y, z;     /* posicao quantizada em decimetros */
    s16 vx, vy, vz;
    u16 hp;
    u8  state;
    u8  anim;
} QALIGN net_entity_t;

typedef struct {
    u8  kind;        /* PKT_SNAPSHOT */
    u8  base_tick;   /* tick contra o qual e o delta (0 = full) */
    u16 tick;
    u8  nent;
    u8  _pad[3];
    net_entity_t ent[1];
} net_snapshot_t;

void net_boot(int as_host, const char *host_ip);
void net_tick(u32 frame_tick);
int  net_is_host(void);
int  net_is_connected(void);

/* Compactacao delta: XOR contra snapshot anterior + run-length. */
u32  net_delta_encode(void *dst, u32 dst_cap,
                      const net_entity_t *base, u32 nbase,
                      const net_entity_t *cur,  u32 ncur);
u32  net_delta_decode(net_entity_t *out, u32 out_cap,
                      const void *src, u32 src_len,
                      const net_entity_t *base, u32 nbase);

/* Dead reckoning aplicado no cliente quando snapshots atrasam. */
void net_predict(net_entity_t *e, u32 dt_ms);

#endif
