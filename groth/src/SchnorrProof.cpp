#include "SchnorrProof.h"

#include "CurvePoint.h"
#include "FakeZZ.h"
#include "Functions.h"
#include <string.h>
#include <openssl/sha.h>
NTL_CLIENT

const ZZ ord = ZZ(NTL::conv<NTL::ZZ>("7237005577332262213973186563042994240857116359379907606001950938285454250989"));

// TODO: small proof sizes are inefficient (serialized and deserialized twice)
ZZ SchnorrProof::fiat_shamir() {
  // this can be preprocessed in SHA but probably cheap
  char temp[CurvePoint::bytesize + 32];
  CurvePoint bp = curve_basepoint();
  bp.serialize(temp); // don't need canonical repr: basepoint is a constant

  memcpy(&temp[CurvePoint::bytesize], this->a_canonical, 32);
  string s = string(temp, CurvePoint::bytesize+32);
  unsigned char hash[SHA256_DIGEST_LENGTH];
  Functions::sha256(s, hash);

  ZZ c;
  ZZFromBytes(c, hash, CurvePoint::bytesize+32);
  return c % ord;
}

// prove and serialize

SchnorrProof::SchnorrProof(const ZZ& alpha) {
  ZZ c, w;
  w = RandomBnd(ord);
  basepoint_scalarmult(this->a, w);
  this->a.serialize_canonical(this->a_canonical);
  c = fiat_shamir();

  ZZ t1;
  MulMod(t1, alpha, c, ord);
  AddMod(this->r, t1, w, ord);
}

void SchnorrProof::serialize(char *output) {
  unsigned char* o = (unsigned char*) output;
  this->a.serialize(&output[0]);
  memcpy(&output[CurvePoint::bytesize], this->a_canonical, 32);
  BytesFromZZ(&o[CurvePoint::bytesize+32], this->r, 32);
}

// deserialize and verify

SchnorrProof::SchnorrProof(const char *serialized) {
  unsigned char* s = (unsigned char*) serialized;
  this->a.deserialize(&serialized[0]);
  memcpy(this->a_canonical, &serialized[CurvePoint::bytesize], 32);
  ZZFromBytes(this->r, &s[CurvePoint::bytesize+32], 32);
}

// given x = g^alpha, g, and proof (r, a)
int SchnorrProof::verify(const CurvePoint& x) {
  ZZ c;
  c = fiat_shamir();

  CurvePoint ta0, tg, ta;
  basepoint_scalarmult(tg, this->r);
  PowerMod(ta0, x, c, ZZ(0)); // these don't work with fake ZZs
  MulMod(ta, this->a, ta0, ZZ(0));

  return ta == tg;
}
