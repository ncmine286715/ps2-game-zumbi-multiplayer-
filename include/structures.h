#ifndef ZUMBI_STRUCTURES_H
#define ZUMBI_STRUCTURES_H

#include "types.h"
#include "inventory.h"

/* Estruturas colocaveis pelo jogador. O mapa expandido permite bases
 * grandes — ate MAX_STRUCTURES pecas no mundo. Cada peca tem hp e pode
 * decair (mechanics.c). Algumas sao "estacoes" (workbench/forge/campfire)
 * que habilitam receitas e mecanicas quando o jogador esta perto. */

typedef enum {
    ST_NONE = 0,
    ST_CAMPFIRE,
    ST_FOUNDATION,
    ST_FLOOR,
    ST_WALL_WOOD,
    ST_WALL_STONE,
    ST_DOOR,
    ST_WINDOW,
    ST_ROOF,
    ST_STAIRS,
    ST_FENCE,
    ST_GATE,
    ST_WORKBENCH,
    ST_FORGE,
    ST_FURNACE,
    ST_STORAGE_BOX,
    ST_BED,
    ST_WATER_BARREL,
    ST_FARM_PLOT,
    ST_TRAP_SPIKE,
    ST_WATCHTOWER,
    ST_GENERATOR,
    ST_LAMP_POST,
    ST_TENT,
    ST_COUNT
} structure_type_t;

typedef struct {
    u8     active;
    u8     type;        /* structure_type_t */
    u8     owner;       /* net_id de quem colocou */
    u8     flags;       /* ST_FLAG_* */
    s16    hp;
    u16    rot_y;
    vec3_t pos;
    /* Estado especifico: fogueira acesa, canteiro com crescimento, bau. */
    u8     lit;         /* fogueira/forja/lampada acesa */
    u8     growth;      /* 0..255 progresso de plantacao (ST_FARM_PLOT) */
    u8     fuel;        /* combustivel restante (fogueira/gerador) */
    u8     store_id;    /* item guardado (ST_STORAGE_BOX) */
    u16    store_count;
} structure_t;

#define ST_FLAG_OPEN    (1 << 0)    /* porta/portao aberto */
#define ST_FLAG_POWERED (1 << 1)    /* recebe energia do gerador */

extern structure_t g_structures[MAX_STRUCTURES];
extern u16         g_structure_count;

/* Mapeia um item de construcao para o tipo de estrutura (ST_NONE se o
 * item nao for colocavel). */
structure_type_t structure_type_of_item(item_id_t item);

void structures_boot(void);
void structures_tick(u32 dt_ms);
void structures_render(void);

/* Coloca uma estrutura a frente do jogador. Retorna o indice ou -1. */
int  structures_place(item_id_t item, fx_t x, fx_t z, u16 rot_y, u8 owner);
void structures_damage(u16 idx, s16 dmg);

/* Retorna 1 se existe uma estrutura do tipo dado dentro do raio (em
 * metros) da posicao — usado para "perto da bancada/forja/fogueira". */
int  structures_near(structure_type_t type, fx_t x, fx_t z, fx_t radius_m);

/* Custo de materiais para construir um item (em FOUNDATION etc o proprio
 * item ja e o resultado do craft; aqui devolve a peca consumida). */

#endif
