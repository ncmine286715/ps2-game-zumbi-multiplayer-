#ifndef ZUMBI_MATH3D_H
#define ZUMBI_MATH3D_H

#include "types.h"

#define PI_F  3.14159265358979f

void  mat4_identity (mat4_t *m);
void  mat4_mul      (mat4_t *out, const mat4_t *a, const mat4_t *b);
void  mat4_translate(mat4_t *m, float x, float y, float z);
void  mat4_rotate_y (mat4_t *m, float rad);
void  mat4_perspective(mat4_t *m, float fov, float aspect, float zn, float zf);
void  mat4_lookat   (mat4_t *m, const vec4_t *eye, const vec4_t *at,
                     const vec4_t *up);

/* Plano de frustum: a*x + b*y + c*z + d = 0.  Lado positivo = dentro. */
typedef struct { float a, b, c, d; } QALIGN plane_t;
typedef struct { plane_t p[6]; }     QALIGN frustum_t;

void  frustum_extract(frustum_t *f, const mat4_t *view_proj);
int   frustum_test_sphere(const frustum_t *f, float x, float y, float z, float r);

/* Quat encode/decode para netcode (10/10/10 + sinal). */
quat32_t quat_pack  (float x, float y, float z, float w);
void     quat_unpack(quat32_t q, float *x, float *y, float *z, float *w);

/* Atalhos usando VU0 macro (registrador $vf*) — chamados a partir
 * de codigo que compila com -mvu0=macro. Em CI sem VU usa fallback C. */
void  vu0_mat_mul   (mat4_t *out, const mat4_t *a, const mat4_t *b);
void  vu0_vec_xform (vec4_t *out, const mat4_t *m, const vec4_t *v);

#endif
