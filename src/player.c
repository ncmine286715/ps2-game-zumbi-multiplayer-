#include "player.h"
#include "input.h"
#include "render.h"
#include "spatial.h"
#include "world.h"
#include "crafting.h"
#include "structures.h"
#include "mechanics.h"
#include "gs.h"
#include "audio.h"
#include <string.h>
#include <math.h>

player_t g_players[MAX_PLAYERS];
u8       g_local_player_idx   = 0;
u8       g_local_player_count = 1;

void player_boot(void)
{
    memset(g_players, 0, sizeof g_players);
    player_t *p = &g_players[0];
    p->active = 1; p->local = 1; p->net_id = 0; p->pad_index = 0;
    p->pos.x = 0; p->pos.y = 0; p->pos.z = 0;
    p->hp = 1000; p->hunger = 1000; p->thirst = 1000;
    p->stamina = 1000; p->temperature = 500;
    p->bleeding = p->infection = p->wetness = 0;
    p->armor_item = ITEM_NONE; p->level = 1; p->xp = 0;
    inv_clear(&p->inv);
    /* kit inicial — machado + faca para comecar a coletar. */
    inv_add(&p->inv, ITEM_AXE_WOOD, 1);
    inv_add(&p->inv, ITEM_KNIFE_STONE, 1);
    p->tool_dura = g_item_defs[ITEM_AXE_WOOD].durability;
    g_local_player_count = 1;
}

int player_add_local(u8 pad_index)
{
    if (g_local_player_count >= MAX_LOCAL_PLAYERS) return -1;
    for (int i = 1; i < MAX_PLAYERS; ++i) {
        player_t *p = &g_players[i];
        if (p->active) continue;
        memset(p, 0, sizeof *p);
        p->active = 1; p->local = 1; p->net_id = (u8)i; p->pad_index = pad_index;
        /* spawn proximo do player 0 com um pequeno offset. */
        p->pos.x = FX_FROM_INT(2 * i);
        p->pos.z = FX_FROM_INT(2 * i);
        p->hp = 1000; p->hunger = 1000; p->thirst = 1000;
        p->stamina = 1000; p->temperature = 500;
        p->armor_item = ITEM_NONE; p->level = 1;
        inv_clear(&p->inv);
        inv_add(&p->inv, ITEM_AXE_WOOD, 1);
        p->tool_dura = g_item_defs[ITEM_AXE_WOOD].durability;
        g_local_player_count++;
        return i;
    }
    return -1;
}

void player_grant_xp(player_t *p, u16 amount)
{
    p->xp += amount;
    u16 need = (u16)(p->level * 100);
    while (p->xp >= need && p->level < 50) {
        p->xp -= need;
        p->level++;
        need = (u16)(p->level * 100);
    }
}

static fx_t apply_friction(fx_t v)
{
    /* Atrito ~ 0.85 por tick. */
    return FX_MUL(v, FX_FROM_FLOAT(0.85f));
}

/* Distancia de interacao a frente do player (usada por acoes). */
static void player_aim_point(const player_t *p, fx_t reach, fx_t *rx, fx_t *rz)
{
    float yaw = (float)p->yaw * (2.0f * PI_F / 65536.0f);
    *rx = p->pos.x + FX_MUL(reach, FX_FROM_FLOAT(sinf(yaw)));
    *rz = p->pos.z + FX_MUL(reach, FX_FROM_FLOAT(cosf(yaw)));
}

/* Tenta abrir bau de loot proximo. */
static int player_open_loot(player_t *p)
{
    for (u16 i = 0; i < g_world.loot_count; ++i) {
        loot_t *l = &g_world.loot[i];
        if (!l->active || l->opened) continue;
        fx_t dx = l->pos.x - p->pos.x, dz = l->pos.z - p->pos.z;
        fx_t d2 = FX_MUL(dx, dx) + FX_MUL(dz, dz);
        if (d2 <= FX_MUL(FX_FROM_INT(2), FX_FROM_INT(2))) {
            inv_add(&p->inv, (item_id_t)l->item, l->count);
            l->opened = 1;
            audio_play(SFX_PICKUP, p->pos.x, p->pos.z);
            player_grant_xp(p, 15);
            return 1;
        }
    }
    return 0;
}

/* Coloca a estrutura do item equipado a frente do player. */
static int player_try_build(player_t *p)
{
    item_id_t eq = inv_equipped(&p->inv);
    if (structure_type_of_item(eq) == ST_NONE) return 0;
    fx_t bx, bz; player_aim_point(p, FX_FROM_INT(3), &bx, &bz);
    if (structures_place(eq, bx, bz, p->yaw, p->net_id) >= 0) {
        inv_remove(&p->inv, eq, 1);
        audio_play(SFX_CRAFT_DONE, p->pos.x, p->pos.z);
        player_grant_xp(p, 10);
        return 1;
    }
    return 0;
}

void player_tick(u32 dt_ms)
{
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        player_t *p = &g_players[i];
        if (!p->active) continue;

        if (p->local) {
            const input_state_t *in = input_for_pad(p->pad_index);

            float yaw = (float)p->yaw * (2.0f * PI_F / 65536.0f);
            float c = cosf(yaw), s = sinf(yaw);
            fx_t  mx = FX_FROM_FLOAT(in->lstick_x / 127.0f);
            fx_t  my = FX_FROM_FLOAT(in->lstick_y / 127.0f);

            /* Correr custa stamina (mechanics). */
            int want_run = (in->buttons & BTN_L1) && p->stamina > 0;
            fx_t speed = want_run ? FX_FROM_FLOAT(0.18f) : FX_FROM_FLOAT(0.10f);
            p->vel.x = FX_MUL(my, FX_MUL(speed, FX_FROM_FLOAT(s))) +
                       FX_MUL(mx, FX_MUL(speed, FX_FROM_FLOAT(c)));
            p->vel.z = FX_MUL(my, FX_MUL(speed, FX_FROM_FLOAT(c))) -
                       FX_MUL(mx, FX_MUL(speed, FX_FROM_FLOAT(s)));

            p->yaw += (s16)(in->rstick_x * 16);

            if (in->pressed & BTN_CROSS)    player_action_primary(p);
            if (in->pressed & BTN_TRIANGLE) player_action_secondary(p);
            if (in->pressed & BTN_CIRCLE)   mech_consume(p, inv_equipped(&p->inv));
            if (in->pressed & BTN_SQUARE) {
                if (!player_try_build(p)) player_open_loot(p);
            }
            if (in->pressed & BTN_L2)
                p->inv.hotbar = (u8)((p->inv.hotbar + 1) % MAX_INV_SLOTS);
            if (in->pressed & BTN_R2)   /* trocar para tras no hotbar */
                p->inv.hotbar = (u8)((p->inv.hotbar + MAX_INV_SLOTS - 1) % MAX_INV_SLOTS);

            int moving = (p->vel.x | p->vel.z) != 0;
            p->state = !moving ? PSTATE_IDLE : (want_run ? PSTATE_RUN : PSTATE_WALK);
        } else {
            /* remoto — posicao vem de net_apply_snapshot */
        }

        p->pos.x += p->vel.x;
        p->pos.z += p->vel.z;
        p->vel.x = apply_friction(p->vel.x);
        p->vel.z = apply_friction(p->vel.z);

        /* Clamp aos limites do mundo expandido. */
        if (p->pos.x < g_world.world_min_x) p->pos.x = g_world.world_min_x;
        if (p->pos.x > g_world.world_max_x) p->pos.x = g_world.world_max_x;
        if (p->pos.z < g_world.world_min_z) p->pos.z = g_world.world_min_z;
        if (p->pos.z > g_world.world_max_z) p->pos.z = g_world.world_max_z;

        spatial_insert(ENT_PLAYER, p->net_id, p->pos.x, p->pos.z);

        /* Hunger/thirst tick (~ 1 ponto a cada 5 s). */
        static u32 acc = 0;
        acc += dt_ms;
        if (acc >= 5000) {
            if (p->hunger > 0) p->hunger--;
            if (p->thirst > 0) p->thirst--;
            if (!p->hunger || !p->thirst) p->hp--;
            acc = 0;
        }
        if (p->hp <= 0) p->state = PSTATE_DEAD;
    }
}

void player_render(void)
{
    draw_cmd_t c = {0};
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        const player_t *p = &g_players[i];
        if (!p->active) continue;
        /* nao desenha o proprio player local 0 (camera em 1a/3a pessoa).
         * Co-op locais (idx > 0) sao desenhados. */
        if (p->local && i == g_local_player_idx) continue;
        c.model = MDL_PLAYER;
        c.pos   = p->pos;
        c.rot_y = p->yaw;
        render_submit(&c);
    }
}

void player_camera(mat4_t *view, mat4_t *proj)
{
    const player_t *p = PLAYER_LOCAL;
    float yaw = (float)p->yaw * (2.0f * PI_F / 65536.0f);
    float px = FX_TO_FLOAT(p->pos.x);
    float pz = FX_TO_FLOAT(p->pos.z);
    vec4_t eye = { px - 4.0f * sinf(yaw), 2.5f, pz - 4.0f * cosf(yaw), 1 };
    vec4_t at  = { px,                    1.5f, pz,                    1 };
    vec4_t up  = { 0, 1, 0, 0 };
    mat4_lookat(view, &eye, &at, &up);
    mat4_perspective(proj, 60.0f * PI_F / 180.0f,
                     (float)GS_WIDTH / (float)GS_HEIGHT, 0.5f, 256.0f);
}

void player_action_primary(player_t *p)
{
    /* Raycast curto a frente do player no grid. */
    fx_t rx, rz; player_aim_point(p, FX_FROM_INT(2), &rx, &rz);

    grid_entry_t hits[8];
    int n = spatial_query(rx, rz, FX_FROM_INT(2), hits, 8);
    for (int i = 0; i < n; ++i) {
        item_id_t eq = inv_equipped(&p->inv);

        if (hits[i].kind == ENT_TREE) {
            /* ids >= MAX_TREES sao pedras (ver world_tick). */
            if (hits[i].id >= MAX_TREES) {
                u16 rid = (u16)(hits[i].id - MAX_TREES);
                if (rid >= g_world.rock_count) continue;
                rock_t *rk = &g_world.rocks[rid];
                if (!rk->hp) continue;
                u8 dmg = (eq == ITEM_PICK_IRON)  ? 32 :
                         (eq == ITEM_PICK_STONE) ? 22 :
                         (eq == ITEM_PICK_WOOD)  ? 15 : 4;
                world_damage_rock(rid, dmg, p->net_id);
                mech_wear_tool(p, 1);
                mech_on_noise(p->pos.x, p->pos.z, 6);
                audio_play(SFX_HIT_WOOD, p->pos.x, p->pos.z);
                if (!rk->hp) {
                    inv_add(&p->inv, ITEM_STONE, 3);
                    if (rk->kind == 1) inv_add(&p->inv, ITEM_IRON_ORE, 2);
                    if (rk->kind == 2) inv_add(&p->inv, ITEM_COAL, 2);
                    if ((p->net_id ^ rid) & 1) inv_add(&p->inv, ITEM_FLINT, 1);
                    audio_play(SFX_PICKUP, p->pos.x, p->pos.z);
                    player_grant_xp(p, 12);
                }
                return;
            }

            /* arvore */
            tree_t *t = &g_world.trees[hits[i].id];
            if (!t->hp) continue;
            u8 dmg = (eq == ITEM_AXE_IRON)  ? 40 :
                     (eq == ITEM_AXE_STONE) ? 28 :
                     (eq == ITEM_AXE_WOOD)  ? 20 : 5;
            world_damage_tree(hits[i].id, dmg, p->net_id);
            mech_wear_tool(p, 1);
            mech_on_noise(p->pos.x, p->pos.z, 5);
            audio_play(SFX_HIT_WOOD, p->pos.x, p->pos.z);
            if (!t->hp) {
                inv_add(&p->inv, ITEM_WOOD, 3);
                inv_add(&p->inv, ITEM_STICK, 2);
                inv_add(&p->inv, ITEM_FIBER, 1);
                if (t->species == 2 /*BUSH*/) inv_add(&p->inv, ITEM_BERRY, 2);
                audio_play(SFX_PICKUP, p->pos.x, p->pos.z);
                player_grant_xp(p, 8);
            }
            return;
        }

        if (hits[i].kind == ENT_ZOMBIE) {
            extern zombie_t g_zombies[MAX_ZOMBIES];
            zombie_t *z = &g_zombies[hits[i].id];
            u8 dmg = g_item_defs[eq].is_tool ? g_item_defs[eq].damage : 10;
            z->hp -= dmg;
            z->state = ZSTATE_STUNNED;
            mech_wear_tool(p, 2);
            mech_on_noise(p->pos.x, p->pos.z, 4);
            audio_play(SFX_HIT_FLESH, p->pos.x, p->pos.z);
            if (z->hp <= 0) {
                z->state  = ZSTATE_DEAD;
                z->active = 0;
                inv_add(&p->inv, ITEM_MEAT_RAW, 1);
                inv_add(&p->inv, ITEM_BONE, 1);
                player_grant_xp(p, 20);
            }
            return;
        }
    }
}

void player_action_secondary(player_t *p)
{
    /* Triangulo: craft do item equipado se for receita conhecida.
     * Receitas que exigem bancada/forja sao validadas por proximidade. */
    item_id_t eq = inv_equipped(&p->inv);
    extern const recipe_t g_recipes[];
    extern const u32      g_recipe_count;
    for (u32 i = 0; i < g_recipe_count; ++i) {
        if (g_recipes[i].output.id != eq) continue;
        if (g_recipes[i].needs_workbench &&
            structures_near(ST_WORKBENCH, p->pos.x, p->pos.z, FX_FROM_INT(4)) < 0 &&
            structures_near(ST_FORGE,     p->pos.x, p->pos.z, FX_FROM_INT(4)) < 0)
            return;     /* sem estacao perto */
        if (craft_try(&p->inv, i)) {
            audio_play(SFX_CRAFT_DONE, p->pos.x, p->pos.z);
            player_grant_xp(p, 5);
        }
        return;
    }
}
