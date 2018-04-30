#ifndef CURVE_POINT_H
#define CURVE_POINT_H

#include "edgamal.h"
#include "FakeZZ.h"
NTL_CLIENT

/* Use elliptic curve points? Compiler macro definition */
//#define USE_REAL_POINTS 1

/* Size of those curve points points when serialized */
/* Either 128 or 32. 128 does cheap serialization while 32 is expensive */
// TODO changing this also requires changing CurvePoint.h
#define CURVE_POINT_BYTESIZE 32

class CurvePoint {
 public:
  CurvePoint();
  CurvePoint(const CurvePoint &other);
  ~CurvePoint();

  bool operator !=(const CurvePoint& b) const;
  bool operator ==(const CurvePoint& b) const;
  void operator =(const CurvePoint& c);

  friend ostream& operator <<(ostream& os, const CurvePoint a);
  friend istream& operator >>(istream& is, CurvePoint& x);

  void serialize_canonical(char *str); // TODO 32 bytes, inverse is raw_curve_pt
  void serialize(char *str); // 128 bytes
  void deserialize(const char *str); // 128 bytes

  static const int bytesize = CURVE_POINT_BYTESIZE;

#if USE_REAL_POINTS
  edgamal_curve_point P;
#else
  NTL::ZZ zz;
#endif
};

// note: this takes the packed form as an argument
CurvePoint raw_curve_pt(const uint8_t p[32]);
CurvePoint curve_zeropoint();
CurvePoint curve_basepoint();

void MulMod(CurvePoint& x, const CurvePoint& a, const CurvePoint& b, const ZZ& n);
CurvePoint MulMod(const CurvePoint& a, const CurvePoint& b, const ZZ& n);

void SqrMod(CurvePoint& x, const CurvePoint& a, const ZZ& n);
CurvePoint sqr(const CurvePoint& a);

void PowerMod(CurvePoint& x, const CurvePoint& a, const ZZ& e, const ZZ& n);
void PowerMod(CurvePoint& x, const CurvePoint& a, long e, const ZZ& n);
CurvePoint PowerMod(const CurvePoint& a, const ZZ& e, const ZZ& n);

// fast scalar multiplication over basepoint
void basepoint_scalarmult(CurvePoint& x, const ZZ& e);

void InvMod(CurvePoint& x, const CurvePoint& a, const ZZ& n);
CurvePoint InvMod(const CurvePoint& a, const ZZ& n);

// for compatibility
CurvePoint zz_to_curve_pt(NTL::ZZ a);

#endif
