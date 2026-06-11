#include "player.h"
#include "input.h"
#include "render.h"
#include "spatial.h"
#include "world.h"
#include "crafting.h"
#include "audio.h"
#include <string.h>
#include <math.h>

player_t g_players[MAX_PLAYERS];
u8       g_local_player_idx = 0;

void player_boot(void)
{
    memset(g_players, 0, sizeof g_players);
    player_t *p = &g_players[0];
    p->active = 1; p->local = 1; p->net_id = 0;
    p->pos.x = 0; p->pos.y = 0; p->pos.z = 0;
    p->hp = 1000; p->hunger = 1000; p->thirst = 1000;
    inv_clear(&p->inv);
    /* kit inicial — 1 machado de madeira para comecar a coletar. */
    inv_add(&p->inv, ITEM_AXE_WOOD, 1);
}

static fx_t apply_friction(fx_t v)
{
    /* Atrito ~ 0.85 por tick. */
    return FX_MUL(v, FX_FROM_FLOAT(0.85f));
}

void player_tick(u32 dt_ms)
{
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        player_t *p = &g_players[i];
        if (!p->active) continue;

        if (p->local) {
            float yaw = (float)p->yaw * (2.0f * PI_F / 65536.0f);
            float c = cosf(yaw), s = sinf(yaw);
            fx_t  mx = FX_FROM_FLOAT(g_input.lstick_x / 127.0f);
            fx_t  my = FX_FROM_FLOAT(g_input.lstick_y / 127.0f);
            /* Movimento no plano do mundo. */
            fx_t  speed = (g_input.buttons & BTN_L1)
                          ? FX_FROM_FLOAT(0.18f)
                          : FX_FROM_FLOAT(0.10f);
            p->vel.x = FX_MUL(my, FX_MUL(speed, FX_FROM_FLOAT(s))) +
                       FX_MUL(mx, FX_MUL(speed, FX_FROM_FLOAT(c)));
            p->vel.z = FX_MUL(my, FX_MUL(speed, FX_FROM_FLOAT(c))) -
                       FX_MUL(mx, FX_MUL(speed, FX_FROM_FLOAT(s)));

            /* Camera: stick direito controla yaw. */
            p->yaw += (s16)(g_input.rstick_x * 16);

            if (g_input.pressed & BTN_CROSS)    player_action_primary(p);
            if (g_input.pressed & BTN_TRIANGLE) player_action_secondary(p);
            if (g_input.pressed & BTN_L2)
                p->inv.hotbar = (p->inv.hotbar + 1) & (MAX_INV_SLOTS - 1);

            p->state = (p->vel.x | p->vel.z) ? PSTATE_WALK : PSTATE_IDLE;
        } else {
            /* remoto — posicao vem de net_apply_snapshot */
        }

        p->pos.x += p->vel.x;
        p->pos.z += p->vel.z;
        p->vel.x = apply_friction(p->vel.x);
        p->vel.z = apply_friction(p->vel.z);

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
        if (!p->active || p->local) continue;       /* nao desenha proprio */
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
    float yaw = (float)p->yaw * (2.0f * PI_F / 65536.0f);
    fx_t  rx  = p->pos.x + FX_FROM_FLOAT(2.0f * sinf(yaw));
    fx_t  rz  = p->pos.z + FX_FROM_FLOAT(2.0f * cosf(yaw));

    grid_entry_t hits[8];
    int n = spatial_query(rx, rz, FX_FROM_INT(2), hits, 8);
    for (int i = 0; i < n; ++i) {
        if (hits[i].kind == ENT_TREE) {
            item_id_t eq = inv_equipped(&p->inv);
            u8 dmg = (eq == ITEM_AXE_WOOD) ? 20 : 5;
            world_damage_tree(hits[i].id, dmg, p->net_id);
            audio_play(SFX_HIT_WOOD, p->pos.x, p->pos.z);
            tree_t *t = &g_world.trees[hits[i].id];
            if (!t->hp) {
                inv_add(&p->inv, ITEM_WOOD, 3);
                inv_add(&p->inv, ITEM_STICK, 2);
                if (t->species == MDL_TREE_BUSH)
                    inv_add(&p->inv, ITEM_BERRY, 2);
                audio_play(SFX_PICKUP, p->pos.x, p->pos.z);
            }
            return;
        }
        if (hits[i].kind == ENT_ZOMBIE) {
            extern zombie_t g_zombies[MAX_ZOMBIES];
            zombie_t *z = &g_zombies[hits[i].id];
            item_id_t eq = inv_equipped(&p->inv);
            u8 dmg = (eq == ITEM_SPEAR)    ? 35 :
                     (eq == ITEM_AXE_WOOD) ? 25 : 10;
            z->hp -= dmg;
            audio_play(SFX_HIT_FLESH, p->pos.x, p->pos.z);
            if (z->hp <= 0) {
                z->state  = ZSTATE_DEAD;
                z->active = 0;
                inv_add(&p->inv, ITEM_MEAT_RAW, 1);
            }
            return;
        }
    }
}

void player_action_secondary(player_t *p)
{
    /* Triangulo: tenta craft do item equipado se for receita conhecida.
     * UI real expandiria menus — versao base craft direto. */
    item_id_t eq = inv_equipped(&p->inv);
    extern const recipe_t g_recipes[];
    extern const u32      g_recipe_count;
    for (u32 i = 0; i < g_recipe_count; ++i) {
        if (g_recipes[i].output.id == eq) {
            if (craft_try(&p->inv, i))
                audio_play(SFX_CRAFT_DONE, p->pos.x, p->pos.z);
            return;
        }
    }
}
