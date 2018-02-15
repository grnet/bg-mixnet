#ifndef EDGAMAL_H
#define EDGAMAL_H

// everything is static so the idea is to inline everything (or at least keep it module-private)

// forward declarations (subject to change)

#include "edgamal_internal_decl.h"

// API

#define EDGAMAL_CURVE_SCALAR_SIZE 32
typedef uint8_t edgamal_curve_scalar[EDGAMAL_CURVE_SCALAR_SIZE];

// typedef <OPAQUE> edgamal_curve_point;
#define EDGAMAL_CURVE_POINT_SIZE sizeof(edgamal_curve_point)

// these are defined in edgamal_internal_impl.h
// static const edgamal_curve_point edgamal_basepoint;
// static const edgamal_curve_point edgamal_zeropoint;

static inline void edgamal_copy_point(edgamal_curve_point *out, const edgamal_curve_point *in);

static inline int edgamal_compare_scalars(const edgamal_curve_scalar a, const edgamal_curve_scalar b);
static inline int edgamal_compare_points(const edgamal_curve_point *a, const edgamal_curve_point *b);

static inline void edgamal_compress_point(uint8_t out[32], const edgamal_curve_point *in);
static inline void edgamal_decompress_point(edgamal_curve_point *out, const uint8_t in[32]);

static inline void edgamal_serialize_point(uint8_t out[128], const edgamal_curve_point *in);
static inline void edgamal_deserialize_point(edgamal_curve_point *out, const uint8_t in[128]);

static inline void edgamal_random_scalar(edgamal_curve_scalar s);
static inline void edgamal_random_point(edgamal_curve_point *p);
static inline void edgamal_random_pair(edgamal_curve_scalar s, edgamal_curve_point *p);

static inline void edgamal_add_scalars(edgamal_curve_scalar out, const edgamal_curve_scalar a, const edgamal_curve_scalar b);
static inline void edgamal_multiply_scalars(edgamal_curve_scalar out, const edgamal_curve_scalar a, const edgamal_curve_scalar b);

static inline void edgamal_add_points(edgamal_curve_point *out, const edgamal_curve_point *a, const edgamal_curve_point *b);
static inline void edgamal_double_point(edgamal_curve_point *out, const edgamal_curve_point *in);
static inline void edgamal_negate_point(edgamal_curve_point *out, const edgamal_curve_point *in);
static inline void edgamal_scalar_multiply_point(edgamal_curve_point *out, const edgamal_curve_point *base, const edgamal_curve_scalar exponent);
static inline void edgamal_scalar_multiply_basepoint(edgamal_curve_point *out, const edgamal_curve_scalar exponent);

// debug (subject to change)

// static inline void edgamal_print_point(const edgamal_curve_point *p);
// static inline void edgamal_print_point_packed(const edgamal_curve_point *p);

// internals (very subject to change)

#include "edgamal_internal_impl.h"

#endif
