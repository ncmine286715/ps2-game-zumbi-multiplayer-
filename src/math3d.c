#include "math3d.h"
#include <math.h>
#include <string.h>

void mat4_identity(mat4_t *m)
{
    memset(m, 0, sizeof *m);
    m->m[0] = m->m[5] = m->m[10] = m->m[15] = 1.0f;
}

void mat4_mul(mat4_t *out, const mat4_t *a, const mat4_t *b)
{
    /* Fallback C. Versao VU0 macro vem em vu0_mat_mul (inline asm). */
    mat4_t r;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
                s += a->m[i*4+k] * b->m[k*4+j];
            r.m[i*4+j] = s;
        }
    *out = r;
}

void mat4_translate(mat4_t *m, float x, float y, float z)
{
    mat4_identity(m);
    m->m[12] = x; m->m[13] = y; m->m[14] = z;
}

void mat4_rotate_y(mat4_t *m, float rad)
{
    float c = cosf(rad), s = sinf(rad);
    mat4_identity(m);
    m->m[0] =  c; m->m[2]  = s;
    m->m[8] = -s; m->m[10] = c;
}

void mat4_perspective(mat4_t *m, float fov, float aspect, float zn, float zf)
{
    float f = 1.0f / tanf(fov * 0.5f);
    memset(m, 0, sizeof *m);
    m->m[0]  = f / aspect;
    m->m[5]  = f;
    m->m[10] = (zf + zn) / (zn - zf);
    m->m[11] = -1.0f;
    m->m[14] = (2.0f * zf * zn) / (zn - zf);
}

static void v_sub(float *o, const float *a, const float *b)
{ o[0]=a[0]-b[0]; o[1]=a[1]-b[1]; o[2]=a[2]-b[2]; }
static void v_cross(float *o, const float *a, const float *b)
{ o[0]=a[1]*b[2]-a[2]*b[1]; o[1]=a[2]*b[0]-a[0]*b[2]; o[2]=a[0]*b[1]-a[1]*b[0]; }
static float v_dot(const float *a, const float *b)
{ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
static void v_norm(float *v)
{ float l = sqrtf(v_dot(v,v)); if (l>0){v[0]/=l;v[1]/=l;v[2]/=l;} }

void mat4_lookat(mat4_t *m, const vec4_t *eye, const vec4_t *at, const vec4_t *up)
{
    float f[3], s[3], u[3];
    float e[3] = {eye->x, eye->y, eye->z};
    float a[3] = {at->x,  at->y,  at->z};
    float U[3] = {up->x,  up->y,  up->z};
    v_sub(f, a, e); v_norm(f);
    v_cross(s, f, U); v_norm(s);
    v_cross(u, s, f);
    mat4_identity(m);
    m->m[0]=s[0]; m->m[4]=s[1]; m->m[8]=s[2];
    m->m[1]=u[0]; m->m[5]=u[1]; m->m[9]=u[2];
    m->m[2]=-f[0];m->m[6]=-f[1];m->m[10]=-f[2];
    m->m[12]=-v_dot(s,e); m->m[13]=-v_dot(u,e); m->m[14]=v_dot(f,e);
}

void frustum_extract(frustum_t *fr, const mat4_t *vp)
{
    const float *m = vp->m;
    #define PL(i, a, b, c, d) fr->p[i].a=a; fr->p[i].b=b; fr->p[i].c=c; fr->p[i].d=d
    /* Linha 3 = vp[12,13,14,15] */
    fr->p[0].a = m[3]+m[0]; fr->p[0].b = m[7]+m[4];
    fr->p[0].c = m[11]+m[8]; fr->p[0].d = m[15]+m[12];   /* left */
    fr->p[1].a = m[3]-m[0]; fr->p[1].b = m[7]-m[4];
    fr->p[1].c = m[11]-m[8]; fr->p[1].d = m[15]-m[12];   /* right */
    fr->p[2].a = m[3]+m[1]; fr->p[2].b = m[7]+m[5];
    fr->p[2].c = m[11]+m[9]; fr->p[2].d = m[15]+m[13];   /* bottom */
    fr->p[3].a = m[3]-m[1]; fr->p[3].b = m[7]-m[5];
    fr->p[3].c = m[11]-m[9]; fr->p[3].d = m[15]-m[13];   /* top */
    fr->p[4].a = m[3]+m[2]; fr->p[4].b = m[7]+m[6];
    fr->p[4].c = m[11]+m[10];fr->p[4].d = m[15]+m[14];   /* near */
    fr->p[5].a = m[3]-m[2]; fr->p[5].b = m[7]-m[6];
    fr->p[5].c = m[11]-m[10];fr->p[5].d = m[15]-m[14];   /* far */
    for (int i = 0; i < 6; ++i) {
        float l = sqrtf(fr->p[i].a*fr->p[i].a +
                        fr->p[i].b*fr->p[i].b +
                        fr->p[i].c*fr->p[i].c);
        if (l > 0) {
            fr->p[i].a /= l; fr->p[i].b /= l;
            fr->p[i].c /= l; fr->p[i].d /= l;
        }
    }
    (void)PL;
}

int frustum_test_sphere(const frustum_t *fr, float x, float y, float z, float r)
{
    for (int i = 0; i < 6; ++i) {
        float d = fr->p[i].a*x + fr->p[i].b*y + fr->p[i].c*z + fr->p[i].d;
        if (d < -r) return 0;
    }
    return 1;
}

/* ---- quat 32 bits ---- */

quat32_t quat_pack(float x, float y, float z, float w)
{
    /* Smallest-three: descarta a maior componente e marca o indice. */
    float a[4] = {x, y, z, w};
    int   imax = 0;
    float vmax = fabsf(a[0]);
    for (int i = 1; i < 4; ++i)
        if (fabsf(a[i]) > vmax) { vmax = fabsf(a[i]); imax = i; }
    if (a[imax] < 0) { a[0]=-a[0]; a[1]=-a[1]; a[2]=-a[2]; a[3]=-a[3]; }
    int o = 0; u32 r = (u32)imax << 30;
    for (int i = 0; i < 4; ++i) {
        if (i == imax) continue;
        s32 v = (s32)(a[i] * 511.0f);
        if (v < -511) v = -511; if (v > 511) v = 511;
        r |= ((u32)(v & 0x3FF)) << (o * 10);
        o++;
    }
    return r;
}

void quat_unpack(quat32_t q, float *x, float *y, float *z, float *w)
{
    int imax = (q >> 30) & 3;
    float c[3];
    for (int i = 0; i < 3; ++i) {
        s32 v = (q >> (i*10)) & 0x3FF;
        if (v & 0x200) v |= ~0x3FF;       /* sign extend 10b */
        c[i] = (float)v / 511.0f;
    }
    float sum = c[0]*c[0] + c[1]*c[1] + c[2]*c[2];
    float m   = sqrtf(1.0f - sum);
    float out[4];
    int o = 0;
    for (int i = 0; i < 4; ++i) out[i] = (i == imax) ? m : c[o++];
    *x = out[0]; *y = out[1]; *z = out[2]; *w = out[3];
}

/* ---- VU0 macro mode placeholders ---- */
void vu0_mat_mul  (mat4_t *o, const mat4_t *a, const mat4_t *b) { mat4_mul(o, a, b); }
void vu0_vec_xform(vec4_t *o, const mat4_t *m, const vec4_t *v)
{
    float x=v->x, y=v->y, z=v->z, w=v->w;
    o->x = m->m[0]*x + m->m[4]*y + m->m[8] *z + m->m[12]*w;
    o->y = m->m[1]*x + m->m[5]*y + m->m[9] *z + m->m[13]*w;
    o->z = m->m[2]*x + m->m[6]*y + m->m[10]*z + m->m[14]*w;
    o->w = m->m[3]*x + m->m[7]*y + m->m[11]*z + m->m[15]*w;
}
