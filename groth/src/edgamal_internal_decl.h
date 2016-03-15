#include <stdint.h>

// internal declarations here
typedef uint64_t bignum25519[5];
typedef uint64_t bignum256modm[5];

typedef struct ge25519_t {
 bignum25519 x, y, z, t;
} ge25519;

// links to API
typedef ge25519 edgamal_curve_point;
