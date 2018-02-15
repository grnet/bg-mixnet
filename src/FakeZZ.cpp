#include "FakeZZ.h"

#include <assert.h>

#if USE_NTL

# include <NTL/ZZ.h>

#else

// independent of ZZ

# include <iostream>
# include <math.h>
# include <stdexcept>

using namespace std;

# include <string.h>
# include "edgamal.h"

// dependent on ZZ

# include <NTL/ZZ.h>

# ifdef NTL_CLIENT
#  undef NTL_CLIENT
# endif
# define NTL_CLIENT

// TODO refactor this into better place

int GROUPMOD = 1;
int SUBGROUPMOD = 0;

NTL::ZZ subgroup_order = NTL::conv<NTL::ZZ>("7237005577332262213973186563042994240857116359379907606001950938285454250989");
// old one:         1257206741114416297422800737364843764556936223541
// other old one:   2093940378184301311653365957372856779274958817946641127345598909177821235333110899157852449358735758089191470831461169154289110965924549400975552759536367817772197222736877807377880197200409316970791234520514702977005806082978079032920444679504632247059010175405894645810064101337094360118559702814823284408560044493630320638017495213077621340331881796467607713650957219938583

ZZ old_gen = ZZ(NTL::conv<NTL::ZZ>("1929181099559129674691211513194785872536670409492790905276619913671396722443243145931673445424440902236760877484211441680348197072495215150053603001343967365713940597148603897520835948403066356627154482171157913975934174689003578096019980791028264452409955094293631742810957258379488668086855090084223965396993821991583550151470397960480522495500106360092070361350077271147228"));
ZZ old_ord = ZZ(NTL::conv<NTL::ZZ>("1257206741114416297422800737364843764556936223541"));
ZZ old_mod = ZZ(NTL::conv<NTL::ZZ>("2093940378184301311653365957372856779274958817946641127345598909177821235333110899157852449358735758089191470831461169154289110965924549400975552759536367817772197222736877807377880197200409316970791234520514702977005806082978079032920444679504632247059010175405894645810064101337094360118559702814823284408560044493630320638017495213077621340331881796467607713650957219938583"));


// class stuff

// either
ZZ::ZZ() {
  is_initialized = false;
}

// TODO self-implemented; should never be publicly called
ZZ::ZZ(NTL::ZZ xzz) {
  this->zz = xzz;
  is_initialized = true;
  is_scalar = true;
}

// TODO self-implemented; should never be publicly called
NTL::ZZ ZZ::get() const {
  return this->zz;
}

// TODO self-implemented; should never be publicly called
void ZZ::set(NTL::ZZ xzz) {
  this->zz = xzz;
  is_initialized = true;
  this->is_scalar = true;
}

ZZ::~ZZ() {}

// both: Cipher_elg constructor uses point, Mod_p * and != uses number, Verifier_toom.check_challenge uses number
bool ZZ::operator !=(const ZZ& b) const {
  assert(is_initialized);
  assert(b.is_initialized);

  assert(is_scalar && b.is_scalar);

  return zz != b.zz;
}

// both: Cipher_elg constructor uses point, Mod_p * and != uses number, Verifier_toom.check_challenge uses number
bool ZZ::operator !=(const long b) const {
  assert(is_initialized);
  assert(is_scalar);

  return zz != b;
}

bool ZZ::operator ==(const ZZ& b) const {
  assert(is_initialized);
  assert(b.is_initialized);

  assert(is_scalar && b.is_scalar);

  return zz == b.zz;
}

// number: Mod_p uses to compare with 0
bool ZZ::operator ==(const long b) const {
  assert(is_initialized);
  assert(is_scalar);

  return zz == b;
}

// either? func_pro uses to initialize matrix to 1, 2, 3, ..., N and initialize to 0, func_ver.check_Delta_op uses for some arithmetic t_3 = n*(i-1)+j and initialize to 1, Functions.bilinearMap uses to initialize to 0, multi_expo to initialize to 1, ...
void ZZ::operator =(const long c) {
  zz = c;
  is_initialized = true;
  is_scalar = true;
}
void ZZ::operator =(const ZZ& c) {
  zz = c.zz;
  is_initialized = true;

  assert(c.is_scalar);
  if (!c.is_initialized) {
    this->is_scalar = true;
  } else {
    this->is_scalar = c.is_scalar;
  }
}

ostream& operator <<(ostream& os, const ZZ a) {
  assert(a.is_initialized);
  assert(a.is_scalar); // point serialization should go through point_serialize and point_deserialize

  return (os << a.zz);
}
istream& operator >>(istream& is, ZZ& x) {
  x.is_initialized = true;
  x.is_scalar = true;

  return (is >> x.zz);
}

// data format and representation

ZZ to_ZZ(long val) {
  return ZZ(NTL::to_ZZ(val));
}
// number: called on everything (including 0, 1, 60, x, p, v, ...)
ZZ to_ZZ(const ZZ& a) {
  assert(a.is_initialized);
  assert(a.is_scalar);

  return ZZ(NTL::to_ZZ(a.get()));   // TODO this is just the identity function
}

// number: (other than print statement for config) only ever called on order
long NumBits(const ZZ& a) {
  assert(a.is_initialized);
  assert(a.is_scalar);

  return NTL::NumBits(a.get());
}
long NumBits(long a) {
  return NTL::NumBits(a);
}

// number: Utils.cpp uses point (not true?), Verifier_toom.cpp uses number
ZZ ZZFromBytes(const unsigned char *p, long n) {
  return ZZ(NTL::ZZFromBytes(p, n));
}

// number: only used in multi_expo
long bit(const ZZ& a, long k) {
  assert(a.is_initialized);
  assert(a.is_scalar);

  return NTL::bit(a.get(), k);
}

long bit(long a, long k) {
  return NTL::bit(a, k);
}


// arithmetic

ZZ prevmod[30];
int nextprevmod = 0;

void CMPME(ZZ n) {
  return;
  //   int yes = 1;
  //   for (int i = 0; i < nextprevmod; i++) {
  //     if (prevmod[i].zz == n.zz) {
  //       yes = 0;
  //     }
  //   }
  //   if (yes) {
  //     prevmod[nextprevmod].zz = n.zz;
  //     nextprevmod++;
  //     cout << "[";
  //     for (int j = 0; j < nextprevmod; j++) {
  //       cout << prevmod[j].zz << ", ";
  //     }
  //     cout << "]" << endl;
  //   }
}


#include <assert.h>

// number
void AddMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(b.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar && b.is_scalar && n.is_scalar);
  assert(n.zz == old_ord.zz);

  NTL::ZZ xzz;
  NTL::AddMod(xzz, a.get(), b.get(), n.get());
  x.set(xzz);
}
void SubMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(b.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar && b.is_scalar && n.is_scalar);
  assert(n.zz == old_ord.zz);

  NTL::ZZ xzz;
  NTL::SubMod(xzz, a.get(), b.get(), n.get());
  x.set(xzz);
}
void NegateMod(ZZ& x, const ZZ& a, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar && n.is_scalar);
  assert(n.zz == old_ord.zz);

  NTL::ZZ xzz;
  NTL::NegateMod(xzz, a.get(), n.get());
  x.set(xzz);
}

// both
void MulMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(b.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar && b.is_scalar);
  if (a.is_scalar) {
    assert(n.zz == old_ord.zz);
  } else {
    assert(n.zz == old_mod.zz);
  }

  bool scalar_flag = a.is_scalar;
  NTL::ZZ xzz;
  NTL::MulMod(xzz, a.get(), b.get(), n.get());
  x.set(xzz);
  x.is_scalar = scalar_flag;
}
// number
void MulMod(ZZ& x, const ZZ& a, long b, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar);
  if (a.is_scalar) {
    assert(n.zz == old_ord.zz);
  } else {
    assert(n.zz == old_mod.zz);
  }

  NTL::ZZ xzz;
  NTL::MulMod(xzz, a.get(), b, n.get());
  x.set(xzz);
}
// both (but point only used during verification while checking E)
ZZ MulMod(const ZZ& a, const ZZ& b, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(b.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar && b.is_scalar);
  if (a.is_scalar) {
    assert(n.zz == old_ord.zz);
  } else {
    assert(n.zz == old_mod.zz);
  }

  ZZ ret = ZZ(NTL::MulMod(a.get(), b.get(), n.get()));
  ret.is_scalar = a.is_scalar;
  return ret;
}

// point: only used in multi_expo
void SqrMod(ZZ& x, const ZZ& a, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar);
  assert(n.zz == old_mod.zz);

  bool scalar_flag = a.is_scalar;
  NTL::ZZ xzz;
  NTL::SqrMod(xzz, a.get(), n.get());
  x.set(xzz);
  x.is_scalar = scalar_flag;
}

// point: only used in Pedersen precomp
ZZ sqr(const ZZ& a) {
  assert(a.is_initialized);

  ZZ ret = ZZ(NTL::sqr(a.get()));
  ret.is_scalar = a.is_scalar;
  return ret;
}

// point
void PowerMod(ZZ& x, const ZZ& a, const ZZ& e, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(e.is_initialized);
  assert(n.is_initialized);

  assert(e.is_scalar);

  assert(a.is_scalar);
  assert(n.zz == old_mod.zz);

  bool scalar_flag = a.is_scalar;
  NTL::ZZ xzz;
  NTL::PowerMod(xzz, a.get(), e.get(), n.get());
  x.set(xzz);
  x.is_scalar = scalar_flag;
}
// point
void PowerMod(ZZ& x, const ZZ& a, long e, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar);
  assert(n.zz == old_mod.zz);

  bool scalar_flag = a.is_scalar;
  NTL::ZZ xzz;
  NTL::PowerMod(xzz, a.get(), e, n.get());
  x.set(xzz);
  x.is_scalar = scalar_flag;
}
// point: only used in G_q and ElGammal
ZZ PowerMod(const ZZ& a, const ZZ& e, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(e.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar);
  assert(e.is_scalar);

  assert(n.zz == old_mod.zz);

  ZZ ret = ZZ(NTL::PowerMod(a.get(), e.get(), n.get()));
  ret.is_scalar = a.is_scalar;
  return ret;
}

// both
void InvMod(ZZ& x, const ZZ& a, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(n.is_initialized);

  if (a.is_scalar) {
    assert(n.zz == old_ord.zz);
  } else {
    assert(n.zz == old_mod.zz);
  }

  bool scalar_flag = a.is_scalar;
  NTL::ZZ xzz;
  NTL::InvMod(xzz, a.get(), n.get());
  x.set(xzz);
  x.is_scalar = scalar_flag;
}

// point: only used in ElGammal
ZZ InvMod(const ZZ& a, const ZZ& n) { CMPME(n);
  assert(a.is_initialized);
  assert(n.is_initialized);

  assert(a.is_scalar);

  assert(n.zz == old_mod.zz);

  ZZ ret = ZZ(NTL::InvMod(a.get(), n.get()));
  ret.is_scalar = a.is_scalar;
  return ret;
}

// number: only used in Verifier_toom
ZZ operator%(const ZZ& a, const ZZ& b) {
  assert(a.is_initialized);
  assert(b.is_initialized);

  assert(a.is_scalar && b.is_scalar);

  return ZZ(a.get() % b.get());
}

// randomness

// number: only ever called on ord (and for permutation)
ZZ RandomBnd(const ZZ& n) { // CMPME(n);
  assert(n.is_initialized);

  assert(n.is_scalar);

  ZZ ret = ZZ(NTL::RandomBnd(n.get()));
  return ret;
}

// number: only called to get permutation
long RandomBnd(long n) {
  return NTL::RandomBnd(n);
}

#endif
