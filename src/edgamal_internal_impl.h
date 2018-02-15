// ed25519 imports
// #include "curve25519-donna-64bit.h"
// #include "ed25519-donna-basepoint-table.h"
// #include "ed25519-donna-impl-base.h"
// #include "modm-donna-64bit.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ed.c"

static const edgamal_curve_point edgamal_zeropoint = {
  {0x0007ffffffffffed,0x0007ffffffffffff,0x0007ffffffffffff,0x0007ffffffffffff,0x0007ffffffffffff},
  {0x0007ffffffffffee,0x0007ffffffffffff,0x0007ffffffffffff,0x0007ffffffffffff,0x0007ffffffffffff},
  {0x0000000000000001,0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000},
  {0x0007ffffffffffed,0x0007ffffffffffff,0x0007ffffffffffff,0x0007ffffffffffff,0x0007ffffffffffff}};

static const edgamal_curve_point edgamal_basepoint = {
  {0x00062d608f25d51a,0x000412a4b4f6592a,0x00075b7171a4b31d,0x0001ff60527118fe,0x000216936d3cd6e5},
  {0x0006666666666658,0x0004cccccccccccc,0x0001999999999999,0x0003333333333333,0x0006666666666666},
  {0x0000000000000001,0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000},
  {0x00068ab3a5b7dda3,0x00000eea2a5eadbb,0x0002af8df483c27e,0x000332b375274732,0x00067875f0fd78b7}};


static inline void edgamal_renormalize_point(edgamal_curve_point *out, const edgamal_curve_point *in) {
  bignum25519 zi, one = {1};

  curve25519_recip(zi, in->z);
  curve25519_mul(out->x, in->x, zi);
  curve25519_mul(out->y, in->y, zi);
  curve25519_mul(out->t, out->x, out->y);
  curve25519_copy(out->z, one);
}

static inline void edgamal_copy_point(edgamal_curve_point *out, const edgamal_curve_point *in) {
  curve25519_copy(out->x, in->x);
  curve25519_copy(out->y, in->y);
  curve25519_copy(out->z, in->z);
  curve25519_copy(out->t, in->t);
}

// TODO should be able to compare by cross-multiplying denominators instead of dividing by z
static inline int edgamal_compare_points(const edgamal_curve_point *a, const edgamal_curve_point *b) {
  uint8_t temp1[32], temp2[32];

  edgamal_compress_point(temp1, a);
  edgamal_compress_point(temp2, b);

  return memcmp(temp1, temp2, 32);
}
static inline int edgamal_compare_scalars(const edgamal_curve_scalar a, const edgamal_curve_scalar b) {
  bignum256modm t_a, t_b;
  edgamal_curve_scalar p_a, p_b;

  // TODO inefficient
  expand256_modm(t_a, a, 32);
  expand256_modm(t_b, b, 32);

  contract256_modm(p_a, t_a);
  contract256_modm(p_b, t_b);

  return memcmp(p_a, p_b, 32);
}

#include <assert.h>
static inline void edgamal_compress_point(uint8_t out[32], const edgamal_curve_point *in) {
  ge25519_pack(out, in);
}
static inline void edgamal_decompress_point(edgamal_curve_point *out, const uint8_t in[32]) {
  int r = ge25519_unpack_vartime(out, in);
  if (r == 0) {
    printf("warning: encountered faulty point\n");
  }
  /*   assert (r != 0);   */
}

static inline void edgamal_serialize_point(uint8_t out[128], const edgamal_curve_point *in) {
  curve25519_contract(&out[ 0], in->x);
  curve25519_contract(&out[32], in->y);
  curve25519_contract(&out[64], in->z);
  curve25519_contract(&out[96], in->t);
}
static inline void edgamal_deserialize_point(edgamal_curve_point *out, const uint8_t in[128]) {
  curve25519_expand(out->x, &in[ 0]);
  curve25519_expand(out->y, &in[32]);
  curve25519_expand(out->z, &in[64]);
  curve25519_expand(out->t, &in[96]);
}

static inline void edgamal_random_scalar(edgamal_curve_scalar s) {
  ed25519_randombytes_unsafe(s, EDGAMAL_CURVE_SCALAR_SIZE);
}
static inline void edgamal_random_point(edgamal_curve_point *p) {
  edgamal_curve_scalar temp;
  edgamal_curve_point raw;

  ed25519_randombytes_unsafe(&temp, EDGAMAL_CURVE_SCALAR_SIZE);
  edgamal_scalar_multiply_basepoint(&raw, temp);
}
static inline void edgamal_random_pair(edgamal_curve_scalar s, edgamal_curve_point *p) {
  edgamal_curve_point raw;

  ed25519_randombytes_unsafe(s, EDGAMAL_CURVE_SCALAR_SIZE);
  edgamal_scalar_multiply_basepoint(&raw, s);
}

static inline void edgamal_add_scalars(edgamal_curve_scalar out, const edgamal_curve_scalar a, const edgamal_curve_scalar b) {
  bignum256modm t_out, t_a, t_b;

  expand256_modm(t_a, a, EDGAMAL_CURVE_SCALAR_SIZE);
  expand256_modm(t_b, b, EDGAMAL_CURVE_SCALAR_SIZE);

  add256_modm(t_out, t_a, t_b);

  contract256_modm(out, t_out);
}
static inline void edgamal_multiply_scalars(edgamal_curve_scalar out, const edgamal_curve_scalar a, const edgamal_curve_scalar b) {
  bignum256modm t_out, t_a, t_b;

  expand256_modm(t_a, a, EDGAMAL_CURVE_SCALAR_SIZE);
  expand256_modm(t_b, b, EDGAMAL_CURVE_SCALAR_SIZE);

  mul256_modm(t_out, t_a, t_b);

  contract256_modm(out, t_out);
}

static inline void edgamal_add_points(edgamal_curve_point *r, const edgamal_curve_point *p, const edgamal_curve_point *q) {
  ge25519_add(r, p, q);
}
static inline void edgamal_double_point(edgamal_curve_point *out, const edgamal_curve_point *in) {
  ge25519_double(out, in);
}
static inline void edgamal_negate_point(edgamal_curve_point *out, const edgamal_curve_point *in) {
  curve25519_neg(out->x, in->x);
  curve25519_copy(out->y, in->y);
  curve25519_copy(out->z, in->z);
  curve25519_neg(out->t, in->t);
}
static inline void edgamal_scalar_multiply_point(edgamal_curve_point *out, const edgamal_curve_point *base, const edgamal_curve_scalar exponent) {
  bignum256modm temp;

  expand256_modm(temp, exponent, EDGAMAL_CURVE_SCALAR_SIZE); // TODO this is vartime

  ge25519_scale_vartime(out, base, temp);
}
static inline void edgamal_scalar_multiply_basepoint(edgamal_curve_point *out, const edgamal_curve_scalar exponent) {
  bignum256modm temp;

  expand256_modm(temp, exponent, EDGAMAL_CURVE_SCALAR_SIZE); // TODO this is vartime

  ge25519_scalarmult_base_niels(out, ge25519_niels_base_multiples, temp);
}

// static inline void edgamal_print_point(const edgamal_curve_point *p)
#define edgamal_print_point(p)                  \
  int i;                                        \
  printf("x:");                                 \
  for (i = 0; i < 5; i++) {                     \
    printf("0x%016llx,", p->x[i]);              \
  }                                             \
                                                \
  printf(" y:");                                \
  for (i = 0; i < 5; i++) {                     \
    printf("0x%016llx,", p->y[i]);              \
  }                                             \
                                                \
  printf(" t:");                                \
  for (i = 0; i < 5; i++) {                     \
    printf("0x%016llx,", p->t[i]);              \
  }                                             \
                                                \
  printf(" z:");                                \
  for (i = 0; i < 5; i++) {                     \
    printf("0x%016llx,", p->z[i]);              \
  }                                             \

// static inline void edgamal_print_point_packed(const edgamal_curve_point *p)
#define edgamal_print_point_packed(p)           \
  uint8_t temp[32];                             \
                                                \
  edgamal_compress_point(temp, p);              \
                                                \
  int i;                                        \
  for (i = 0; i < 32; i++) {                    \
    printf("%02hhx ", temp[i]);                 \
  }                                             \

