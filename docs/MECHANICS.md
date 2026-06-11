# Mecanicas e conteudo

Catalogo das **40 mecanicas** e das **40 ferramentas/armas** adicionadas,
alem do mapa expandido e do sistema de construcao.

## Mapa expandido (512 x 512 m)

Mundo procedural deterministico por seed, dividido em **biomas**
(`world_biome_at`):

| Bioma | Conteudo |
|---|---|
| `BIOME_FOREST` | pinheiros/carvalhos densos, muita madeira |
| `BIOME_PLAINS` | campo aberto, poucos recursos, bom p/ base |
| `BIOME_SWAMP`  | arvores mortas, mais perigoso |
| `BIOME_BEACH`  | palmeiras, areia, agua (bordas do mapa) |
| `BIOME_ROCKY`  | pedra, **minerio de ferro** e **carvao** |

Recursos no mundo: ate `MAX_TREES` (1024) arvores, `MAX_ROCKS` (256)
pedras/veios, `MAX_LOOT` (96) baus espalhados.

## As 40 mecanicas (`mechanic_id_t` / `g_mechanics[]`)

Sobrevivencia: **1** movimento · **2** corrida · **3** stamina ·
**4** fome · **5** sede · **6** regen. de vida · **7** sangramento ·
**8** infeccao (mordida) · **9** temperatura corporal.

Ambiente: **10** ciclo dia/noite · **11** clima (limpo/chuva/neblina/
tempestade) · **12** umidade · **13** fogo/fogueira.

Coleta: **14** cozinhar · **15** pescar · **16** plantar · **17** coletar ·
**18** cacar · **19** cortar arvore · **20** minerar.

Craft/Construcao: **21** crafting · **22** construir · **23** decaimento ·
**24** durabilidade · **25** reparar.

Combate: **26** armadura · **27** combate a distancia · **28** corpo a
corpo · **29** furtividade · **30** atracao de zumbis por ruido ·
**31** horda noturna · **32** ondas de spawn.

Mundo/Progressao: **33** loot · **34** troca · **35** renascer ·
**36** dormir · **37** XP/nivel · **38** efeitos de status ·
**39** **co-op local** (ate 4 controles) · **40** minimapa/bussola.

As mecanicas marcadas como `active_sim` rodam em `mechanics_tick()` a cada
tick fixo; as demais sao disparadas por eventos (acoes, estruturas).

### Destaques de implementacao

- **Dia/noite + horda**: `env_light_level()` controla a luz; na borda de
  subida da noite, `mechanics_tick` invoca `zombie_spawn` em volta de cada
  jogador (escala com `day_count`). Zumbis correm mais rapido a noite.
- **Temperatura**: tende ao ambiente; fogueira/forja aquecem num raio de
  6 m; chuva/umidade esfriam; extremos drenam vida.
- **Mordida** (`mech_on_bite`): dano + chance de sangramento e infeccao,
  mitigado por armadura. Antibiotico cura infeccao; bandagem/medkit
  estancam sangramento.
- **Durabilidade** (`mech_wear_tool`): cada uso gasta a ferramenta ativa;
  ao zerar, ela quebra. `mech_repair` usa um **Kit de reparo**.
- **Ruido** (`mech_on_noise`): minerar/cortar/atacar/atirar acorda zumbis
  proximos (raio em metros = "loudness").

## As 40 ferramentas/armas (`ITEM_TOOL_FIRST..ITEM_TOOL_LAST`)

Machados (madeira/pedra/ferro), picaretas (madeira/pedra/ferro), facas
(pedra/ferro), lancas (madeira/ferro), martelo, pas (madeira/ferro),
enxada, vara de pesca, arco, besta, estilingue, facao, porrete, taco com
pregos, espada de ferro, marreta, chave inglesa, tesoura, agulha, serra,
formao, foice, isqueiro, tocha, lanterna, binoculo, bussola, mapa, kit de
reparo, pistola, rifle, escopeta e molotov.

Cada uma tem `damage` e `durability` em `g_item_defs[]`. Use
`ITEM_IS_TOOL(id)` para checar a faixa.

## Construcao (`structures.h` / 23 tipos)

Itens de construcao viram `structure_t` no mundo via `structures_place`
(botao **Quadrado** mira a frente do jogador). Estacoes habilitam
receitas e mecanicas:

- **Fogueira/Forja/Fornalha**: cozinhar, fundir minerio, aquecer.
- **Bancada (Workbench)**: receitas que exigem `needs_workbench`.
- **Bau**: guardar itens. **Cama**: dormir. **Canteiro**: plantar.
- **Parede/Piso/Porta/Janela/Telhado/Cerca/Portao/Torre**: base/defesa.
- **Armadilha de espinhos**, **gerador**, **poste de luz**, **barraca**.

Estruturas tem HP e podem ser destruidas (`structures_damage`).

## Controles (em jogo)

| Botao | Acao |
|---|---|
| Stick L | mover · Stick R | girar camera |
| L1 | correr (gasta stamina) |
| X | acao primaria (cortar/minerar/atacar) |
| Triangulo | craftar item equipado (perto da estacao se exigir) |
| Quadrado | construir item equipado / abrir bau proximo |
| Circulo | consumir item equipado (comer/beber/curar) |
| L2 / R2 | trocar slot do hotbar (frente/tras) |

## Co-op local

`player_add_local(pad_index)` registra um jogador extra por **controle
conectado** (porta 2 ou multitap), ate `MAX_LOCAL_PLAYERS` (4). Cada um
le seu proprio pad via `input_for_pad()`. A camera segue o jogador 0
(tela compartilhada). O netcode online continua disponivel em paralelo.
