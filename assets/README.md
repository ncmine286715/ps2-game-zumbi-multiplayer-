# Assets

Formato dos modelos: `.mdl` quantizado, gerado por ferramenta externa
a partir de OBJ/FBX. Layout little-endian:

```
struct mdl_header {
    char magic[4];      // "ZMDL"
    u16  nverts;
    u16  ntris;
    f32  scale;         // pos = vtx_s16 * scale
    u32  tex_id;
    f32  radius;        // bounding sphere
};

struct vtx16 { s16 x, y, z; u16 _pad; };
struct tri16 { u16 a, b, c; u16 _pad; };
```

Texturas: PNG decodificado para CT32 (32 bpp) ou PSMT4 (4 bpp + CLUT
para arvores/zumbi onde 16 cores ja servem). Conversao em build time
via `tools/png_to_gs.py`.

Sons: VAG/ADPCM proprietario do SPU2 (formato ja documentado por
Sony libs). 22 kHz mono para SFX, 44 kHz stereo para musica.

## Lista para o primeiro cenario

| arquivo | descricao |
|---|---|
| tree_pine.mdl  | pinheiro alto, ~120 tris no LOD0 |
| tree_oak.mdl   | carvalho copa larga, ~180 tris |
| bush.mdl       | arbusto, ~24 tris |
| player.mdl     | personagem com skinning manual (4 joints) |
| zombie.mdl     | zumbi com 3 animacoes (walk/attack/death) |
| rock.mdl       | rocha decorativa |
| crate.mdl      | container craftavel |
| ground_grass.tex | atlas 256x256 chao |
| bark.tex        | atlas 128x128 troncos |
| campfire.vag    | loop de fogueira |
| groan.vag       | gemido de zumbi |
| hit_wood.vag    | corte de machado |
| pickup.vag      | confirmacao de coleta |
