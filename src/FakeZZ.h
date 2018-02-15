/* Use limited-size elliptic curve point scalars? */
#define USE_NTL 1

#if USE_NTL

# include <NTL/ZZ.h>

#else

# ifndef FAKE_ZZ_H
# define FAKE_ZZ_H

// independent of ZZ

# include <iostream>
# include <math.h>
# include <stdexcept>

using namespace std;

long bit(long a, long k);

# include "edgamal.h"

// dependent on ZZ

# include <NTL/ZZ.h>
# ifdef NTL_CLIENT
#  undef NTL_CLIENT
# endif
# define NTL_CLIENT

// class stuff

// either
class ZZ { // TODO need to fix public NTL members either via pointer or via clever include
 public:
  ZZ();
  ~ZZ();

  bool operator !=(const ZZ& b) const;
  bool operator !=(const long b) const;

  bool operator ==(const ZZ& b) const;
  bool operator ==(const long b) const;

  void operator =(const long c);
  void operator =(const ZZ& c);

  friend ostream& operator <<(ostream& os, const ZZ a);
  friend istream& operator >>(istream& is, ZZ& x);


  // TODO self-implemented; should never be publicly called
  ZZ(NTL::ZZ zz);
  NTL::ZZ get() const;
  void set(NTL::ZZ zz);

  bool is_scalar;
  bool is_initialized;

  NTL::ZZ zz;
  curve_point P;
 //   private:
 //    NTL::ZZ zz;
};

// data format and representation

ZZ to_ZZ(long val);
ZZ to_ZZ(const ZZ& a); // TODO this is just the identity function

long NumBits(const ZZ& a);
long NumBits(long a);

ZZ ZZFromBytes(const unsigned char *p, long n);

long bit(const ZZ& a, long k); // TODO this is used for multi_expo -- need to know if replaceable

// arithmetic

void AddMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& n); // x = (a+b)%n
void SubMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& n); // x = (a-b)%n
void NegateMod(ZZ& x, const ZZ& a, const ZZ& n); // x = -a % n

void MulMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& n); // x = (a*b)%n
void MulMod(ZZ& x, const ZZ& a, long b, const ZZ& n); // TODO beware of long to point conversion
ZZ MulMod(const ZZ& a, const ZZ& b, const ZZ& n);

void SqrMod(ZZ& x, const ZZ& a, const ZZ& n); // x = a^2 % n
ZZ sqr(const ZZ& a);

void PowerMod(ZZ& x, const ZZ& a, const ZZ& e, const ZZ& n);
void PowerMod(ZZ& x, const ZZ& a, long e, const ZZ& n); // TODO beware of long to point conversion
ZZ PowerMod(const ZZ& a, const ZZ& e, const ZZ& n);

void InvMod(ZZ& x, const ZZ& a, const ZZ& n);
ZZ InvMod(const ZZ& a, const ZZ& n);

ZZ operator%(const ZZ& a, const ZZ& b);

// randomness

ZZ RandomBnd(const ZZ& n);

long RandomBnd(long n);

# endif

#endif
