# Protocolo de rede

## Camada de transporte

UDP. Ordem nao garantida, perda toleravel — snapshots novos
sobrescrevem antigos. Reliable apenas para eventos discretos
(craft, drop, dano) via re-envio ate ack.

## Topologia

```
   Cliente A ----.
   Cliente B ----+--->  Host autoritativo  --->  Estado canonico
   Cliente C ----'
```

- Host roda a simulacao completa (player + zumbi + mundo).
- Clientes mandam apenas input + intentos (use item, craft).
- Host responde com snapshots delta a cada tick (50 ms).

## Frame de pacote

```
+------+--------+-------------+
| kind | tick   | payload     |
+------+--------+-------------+
| 1 B  | 2 B    | ate 1197 B  |
```

`kind` mapeia para `pkt_kind_t` em `include/network.h`.

## Snapshot delta

1. Host mantem buffer circular de 32 snapshots passados.
2. Cliente envia ack do ultimo snapshot recebido junto com input.
3. Host computa delta contra o snapshot ackeado e envia.
4. Se o ack ficou antigo demais (mais de 1 s), host envia full state.

Payload do delta:

```
u16  ncur                  ; quantas entidades
u8   bitmap[ceil(ncur/8)]  ; 1 = mudou desde o baseline
struct net_entity_t [k]    ; so as marcadas
```

`net_entity_t` (24 B): kind, id, yaw, pos(xyz s16 decimetros),
vel(xyz s16), hp, state, anim.

## Dead reckoning

Quando o cliente fica 2+ ticks sem snapshot novo, ele extrapola
posicao = pos + vel * dt. Ao chegar o proximo snapshot, faz lerp
suave em 100 ms para mascarar o snap-back.

## Eventos confiaveis

Mesmo pacote UDP, mas com `kind = PKT_EVENT` + seq u16. Cliente
guarda `last_event_seq`; host re-envia eventos perdidos junto com
o snapshot ate receber ack explicito.

## Anti-cheat basico

- Servidor reprovado: host valida posicao do client (limite de
  velocidade, sem clip atraves de arvores via spatial grid).
- Input com tick monotonicamente crescente — drop em retrocesso.
