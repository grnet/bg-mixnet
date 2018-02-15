#include "CurvePoint.h"

#include "edgamal.h"
#include <string.h>

#include "assert.h"

#include "FakeZZ.h"
NTL_CLIENT

const ZZ ord = ZZ(NTL::conv<NTL::ZZ>("7237005577332262213973186563042994240857116359379907606001950938285454250989"));

// TODO: ensure memory management is OK with switch to pointers

// helpers

void ScalarFromZZ(edgamal_curve_scalar s, const ZZ& x) {
  // normalize first
  ZZ n = x;
  BytesFromZZ(s, n, EDGAMAL_CURVE_SCALAR_SIZE);
}

// interface fns

CurvePoint::CurvePoint() {
#if USE_REAL_POINTS
  // TODO what should we do? leave uninitialized?
  // memset(P, 0, EDGAMAL_CURVE_POINT_SIZE);
#else
  // pass
#endif
}

CurvePoint::CurvePoint(const CurvePoint &c)
#if !USE_REAL_POINTS
  : zz(c.zz) {}
#else
  {edgamal_copy_point(&P, &c.P);}
#endif

CurvePoint::~CurvePoint() {}

// both: Cipher_elg constructor uses point, Mod_p * and != uses number, Verifier_toom.check_challenge uses number
bool CurvePoint::operator !=(const CurvePoint& b) const {
#if USE_REAL_POINTS
  return edgamal_compare_points(&P, &b.P) != 0;
#else
  return zz != b.zz;
#endif
}

bool CurvePoint::operator ==(const CurvePoint& b) const {
#if USE_REAL_POINTS
  return edgamal_compare_points(&P, &b.P) == 0;
#else
  return zz == b.zz;
#endif
}

void CurvePoint::operator =(const CurvePoint& c) {
#if USE_REAL_POINTS
  edgamal_copy_point(&P, &c.P);
#else
  zz = c.zz;
#endif
}

// these tend to be used in the NIZK proof,
// while (de)serialize tend to be used in encryption/decryption
ostream& operator <<(ostream& os, const CurvePoint point) {
#if USE_REAL_POINTS
  ZZ x;
  uint8_t t[128];
  edgamal_serialize_point(t, &point.P);
  ZZFromBytes(x, t, 128);
  os << x;
#else
  os << point.zz;
#endif
  return os;
}
istream& operator >>(istream& is, CurvePoint& point) {
#if USE_REAL_POINTS
  ZZ x;
  uint8_t t[128];
  is >> x;
  BytesFromZZ(t, x, 128);
  edgamal_deserialize_point(&point.P, t);
#else
  is >> point.zz;
#endif
  return is;
}

void CurvePoint::serialize_canonical(char* p) {
#if USE_REAL_POINTS
  edgamal_compress_point((uint8_t*) p, &P);
#else
  assert(false);
#endif
}
void CurvePoint::serialize(char *str) {
#if USE_REAL_POINTS
# if (CURVE_POINT_BYTESIZE == 128)
  edgamal_serialize_point((uint8_t*) str, &P);
# elif (CURVE_POINT_BYTESIZE == 32)
  edgamal_compress_point((uint8_t*) str, &P);
# else
  assert(false);
# endif
#else
  assert(false);
#endif
}
void CurvePoint::deserialize(const char *str) {
#if USE_REAL_POINTS
# if (CURVE_POINT_BYTESIZE == 128)
  edgamal_deserialize_point(&P, (uint8_t*) str);
# elif (CURVE_POINT_BYTESIZE == 32)
  edgamal_decompress_point(&P, (uint8_t*) str);
# else
  assert(false);
# endif
#else
  assert(false);
#endif
}

void MulMod(CurvePoint& x, const CurvePoint& a, const CurvePoint& b, const ZZ& n) {
#if USE_REAL_POINTS
  edgamal_add_points(&x.P, &a.P, &b.P);
#else
# if USE_NTL
  NTL::MulMod(x.zz, a.zz, b.zz, n);
# else
  NTL::MulMod(x.zz, a.zz, b.zz, n.zz);
# endif
#endif
}
CurvePoint MulMod(const CurvePoint& a, const CurvePoint& b, const ZZ& n) {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_add_points(&x.P, &a.P, &b.P);
#else
# if USE_NTL
  NTL::MulMod(x.zz, a.zz, b.zz, n);
# else
  NTL::MulMod(x.zz, a.zz, b.zz, n.zz);
# endif
#endif
  return x;
}

void SqrMod(CurvePoint& x, const CurvePoint& a, const ZZ& n) {
#if USE_REAL_POINTS
  edgamal_add_points(&x.P, &a.P, &a.P);
#else
# if USE_NTL
  NTL::SqrMod(x.zz, a.zz, n);
# else
  NTL::SqrMod(x.zz, a.zz, n.zz);
# endif
#endif
}
CurvePoint sqr(const CurvePoint& a) {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_add_points(&x.P, &a.P, &a.P);
#else
  NTL::sqr(a.zz);
#endif
  return x;
}

void PowerMod(CurvePoint& x, const CurvePoint& a, const ZZ& e, const ZZ& n) {
#if USE_REAL_POINTS
  edgamal_curve_scalar s = {0};
  ScalarFromZZ(s, e);
  edgamal_scalar_multiply_point(&x.P, &a.P, s);
#else
# if USE_NTL
  NTL::PowerMod(x.zz, a.zz, e, n);
# else
  NTL::PowerMod(x.zz, a.zz, e.zz, n.zz);
# endif
#endif
}
void PowerMod(CurvePoint& x, const CurvePoint& a, long e, const ZZ& n) {
#if USE_REAL_POINTS
  edgamal_curve_scalar s = {0};
  ScalarFromZZ(s, to_ZZ(e));
  edgamal_scalar_multiply_point(&x.P, &a.P, s);
#else
# if USE_NTL
  NTL::PowerMod(x.zz, a.zz, e, n);
# else
  NTL::PowerMod(x.zz, a.zz, e, n.zz);
# endif
#endif
}
CurvePoint PowerMod(const CurvePoint& a, const ZZ& e, const ZZ& n) {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_curve_scalar s = {0};
  ScalarFromZZ(s, e);
  edgamal_scalar_multiply_point(&x.P, &a.P, s);
#else
# if USE_NTL
  NTL::PowerMod(x.zz, a.zz, e, n);
# else
  NTL::PowerMod(x.zz, a.zz, e.zz, n.zz);
# endif
#endif
  return x;
}

void basepoint_scalarmult(CurvePoint& x, const ZZ& e) {
#if USE_REAL_POINTS
  edgamal_curve_scalar s = {0};
  ScalarFromZZ(s, e);
  edgamal_scalar_multiply_basepoint(&x.P, s);
#else
  assert(false);
#endif
}

void InvMod(CurvePoint& x, const CurvePoint& a, const ZZ& n) {
#if USE_REAL_POINTS
  edgamal_negate_point(&x.P, &a.P);
#else
# if USE_NTL
  NTL::InvMod(x.zz, a.zz, n);
# else
  NTL::InvMod(x.zz, a.zz, n.zz);
# endif
#endif
}
CurvePoint InvMod(const CurvePoint& a, const ZZ& n) {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_negate_point(&x.P, &a.P);
#else
# if USE_NTL
  NTL::InvMod(x.zz, a.zz, n);
# else
  NTL::InvMod(x.zz, a.zz, n.zz);
# endif
#endif
  return x;
}

// fixes
CurvePoint zz_to_curve_pt(NTL::ZZ a) {
  CurvePoint x;
#if USE_REAL_POINTS
  assert(false);
#else
  x.zz = NTL::ZZ(a);
#endif
  return x;
}

CurvePoint curve_zeropoint() {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_copy_point(&x.P, &edgamal_zeropoint);
#else
  x.zz = NTL::ZZ(1);
#endif
  return x;
}
CurvePoint curve_basepoint() {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_copy_point(&x.P, &edgamal_basepoint);
#else
  assert(false);
#endif
  return x;
}
CurvePoint raw_curve_pt(const uint8_t p[32]) {
  CurvePoint x;
#if USE_REAL_POINTS
  edgamal_decompress_point(&x.P, p);
#else
  assert(false);
#endif
  return x;
}
