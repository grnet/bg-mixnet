#include "edgamal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void fiat_shamir(edgamal_curve_scalar c,
                 const edgamal_curve_point *g, const edgamal_curve_point *h,
                 const edgamal_curve_point *a, const edgamal_curve_point *b) {
  edgamal_curve_scalar C = {3};
  memcpy(c, C, 32);
}

void prove(edgamal_curve_point *a, edgamal_curve_point *b, edgamal_curve_scalar r,
           const edgamal_curve_point *g, const edgamal_curve_point *h,
           const edgamal_curve_scalar alpha, const edgamal_curve_scalar w) {
  edgamal_curve_scalar c;

  edgamal_scalar_multiply_point(a, g, w);
  edgamal_scalar_multiply_point(b, h, w);

  fiat_shamir(c, g, h, a, b);

  edgamal_curve_scalar t1;
  edgamal_multiply_scalars(t1, alpha, c);
  edgamal_add_scalars(r, t1, w);
}

int verify(const edgamal_curve_point *a, const edgamal_curve_point *b, const edgamal_curve_scalar r,
           const edgamal_curve_point *g, const edgamal_curve_point *h,
           const edgamal_curve_point *x, const edgamal_curve_point *y) {
  edgamal_curve_scalar c;
  fiat_shamir(c, g, h, a, b);

  edgamal_curve_point ta0, tg, ta, tb0, th, tb;
  edgamal_scalar_multiply_point(&tg, g, r);
  edgamal_scalar_multiply_point(&th, h, r);
  edgamal_scalar_multiply_point(&ta0, x, c);
  edgamal_scalar_multiply_point(&tb0, y, c);
  edgamal_add_points(&ta, a, &ta0);
  edgamal_add_points(&tb, b, &tb0);

  return (edgamal_compare_points(&ta, &tg) == 0) && (edgamal_compare_points(&tb, &th) == 0);
}

int main() {
  edgamal_curve_scalar alpha, w, r;
  edgamal_curve_point h, g, x, y, a, b;
  edgamal_random_point(&h);
  edgamal_copy_point(&g, &edgamal_basepoint);
  edgamal_random_scalar(w);

  edgamal_random_scalar(alpha);
  edgamal_scalar_multiply_point(&x, &g, alpha);
  edgamal_scalar_multiply_point(&y, &h, alpha);

  prove(&a, &b, r, &g, &h, alpha, w);
  int result = verify(&a, &b, r, &g, &h, &x, &y);
  assert(result == 1);
  printf("result is %d\n", result);

  return 0;
}
