#include <stddef.h>

// interface-needed

typedef uint64_t bignum256modm_element_t;

static void ed25519_randombytes_unsafe (void *p, size_t len);
static void mul256_modm(bignum256modm r, const bignum256modm x, const bignum256modm y);
static void expand_raw256_modm(bignum256modm out, const unsigned char in[32]);
static void contract256_modm(unsigned char out[32], const bignum256modm in);

static int ge25519_unpack_vartime(ge25519 *r, const unsigned char p[32]);
static int ge25519_unpack_negative_vartime(ge25519 *r, const unsigned char p[32]);
static void ge25519_pack(unsigned char r[32], const ge25519 *p);
static void ge25519_add(ge25519 *r, const ge25519 *p, const ge25519 *q);

inline __attribute__((always_inline))
static void curve25519_copy(bignum25519 out, const bignum25519 in);
inline __attribute__((always_inline))
static void curve25519_neg(bignum25519 out, const bignum25519 a);
inline __attribute__((always_inline))
static void curve25519_mul(bignum25519 out, const bignum25519 in2, const bignum25519 in);

// ???
#include <stdlib.h>
#include <string.h>

typedef struct ge25519_pniels_t {
 bignum25519 ysubx, xaddy, z, t2d;
} ge25519_pniels;

typedef struct ge25519_p1p1_t {
 bignum25519 x, y, z, t;
} ge25519_p1p1;

static void contract256_slidingwindow_modm(signed char r[256], const bignum256modm s, int windowsize);

static void ge25519_double(ge25519 *r, const ge25519 *p);
static void ge25519_double_p1p1(ge25519_p1p1 *r, const ge25519 *p);
static void ge25519_p1p1_to_partial(ge25519 *r, const ge25519_p1p1 *p);
static void ge25519_p1p1_to_full(ge25519 *r, const ge25519_p1p1 *p);
static void ge25519_full_to_pniels(ge25519_pniels *p, const ge25519 *r);
static void ge25519_pnielsadd(ge25519_pniels *r, const ge25519 *p, const ge25519_pniels *q);
static void ge25519_pnielsadd_p1p1(ge25519_p1p1 *r, const ge25519 *p, const ge25519_pniels *q, unsigned char signbit);

static void
ge25519_scale_vartime(ge25519 *r, const ge25519 *p1, const bignum256modm s1) {
 signed char slide1[256];
 ge25519_pniels pre1[(1<<(5 -2))];
 ge25519 d1;
 ge25519_p1p1 t;
 int32_t i;

 contract256_slidingwindow_modm(slide1, s1, 5);

 ge25519_double(&d1, p1);
 ge25519_full_to_pniels(pre1, p1);
 for (i = 0; i < (1<<(5 -2)) - 1; i++)
  ge25519_pnielsadd(&pre1[i+1], &d1, &pre1[i]);


 memset(r, 0, sizeof(ge25519));
 r->y[0] = 1;
 r->z[0] = 1;

 i = 255;
 while ((i >= 0) && !(slide1[i]))
  i--;

 for (; i >= 1; i--) {
  ge25519_double_p1p1(&t, r);

  if (slide1[i]) {
   int index = abs(slide1[i]) / 2;
   ge25519_p1p1_to_full(r, &t);
   ge25519_pnielsadd_p1p1(&t, r, &pre1[index], (unsigned char)slide1[i] >> 7);
  }

  ge25519_p1p1_to_partial(r, &t);
 }
 {
  i = 0;
  ge25519_double_p1p1(&t, r);

  if (slide1[i]) {
   int index = abs(slide1[i]) / 2;
   ge25519_p1p1_to_full(r, &t);
   ge25519_pnielsadd_p1p1(&t, r, &pre1[index], (unsigned char)slide1[i] >> 7);
  }

  ge25519_p1p1_to_full(r, &t);
 }
}


// compile-needed

#define ALIGN(x) __attribute__((aligned(x)))

typedef struct ge25519_niels_t {
 bignum25519 ysubx, xaddy, t2d;
} ge25519_niels;

#define ROTL32(a,b) (((a) << (b)) | ((a) >> (32 - b)))
#define ROTR32(a,b) (((a) >> (b)) | ((a) << (32 - b)))
static void U32TO8_LE(unsigned char *p, const uint32_t v);



// include ed25519-donna-64bit-tables.h

static const ge25519 ge25519_basepoint = {
	{0x00062d608f25d51a,0x000412a4b4f6592a,0x00075b7171a4b31d,0x0001ff60527118fe,0x000216936d3cd6e5},
	{0x0006666666666658,0x0004cccccccccccc,0x0001999999999999,0x0003333333333333,0x0006666666666666},
	{0x0000000000000001,0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000},
	{0x00068ab3a5b7dda3,0x00000eea2a5eadbb,0x0002af8df483c27e,0x000332b375274732,0x00067875f0fd78b7}
};

static const bignum25519 ge25519_ecd = {
	0x00034dca135978a3,0x0001a8283b156ebd,0x0005e7a26001c029,0x000739c663a03cbb,0x00052036cee2b6ff
};

static const bignum25519 ge25519_ec2d = {
	0x00069b9426b2f159,0x00035050762add7a,0x0003cf44c0038052,0x0006738cc7407977,0x0002406d9dc56dff
};

static const bignum25519 ge25519_sqrtneg1 = {
	0x00061b274a0ea0b0,0x0000d5a5fc8f189d,0x0007ef5e9cbd0c60,0x00078595a6804c9e,0x0002b8324804fc1d
};

static const ge25519_niels ge25519_niels_sliding_multiples[32] = {
	{{0x00003905d740913e,0x0000ba2817d673a2,0x00023e2827f4e67c,0x000133d2e0c21a34,0x00044fd2f9298f81},{0x000493c6f58c3b85,0x0000df7181c325f7,0x0000f50b0b3e4cb7,0x0005329385a44c32,0x00007cf9d3a33d4b},{0x00011205877aaa68,0x000479955893d579,0x00050d66309b67a0,0x0002d42d0dbee5ee,0x0006f117b689f0c6}},
	{{0x00011fe8a4fcd265,0x0007bcb8374faacc,0x00052f5af4ef4d4f,0x0005314098f98d10,0x0002ab91587555bd},{0x0005b0a84cee9730,0x00061d10c97155e4,0x0004059cc8096a10,0x00047a608da8014f,0x0007a164e1b9a80f},{0x0006933f0dd0d889,0x00044386bb4c4295,0x0003cb6d3162508c,0x00026368b872a2c6,0x0005a2826af12b9b}},
	{{0x000182c3a447d6ba,0x00022964e536eff2,0x000192821f540053,0x0002f9f19e788e5c,0x000154a7e73eb1b5},{0x0002bc4408a5bb33,0x000078ebdda05442,0x0002ffb112354123,0x000375ee8df5862d,0x0002945ccf146e20},{0x0003dbf1812a8285,0x0000fa17ba3f9797,0x0006f69cb49c3820,0x00034d5a0db3858d,0x00043aabe696b3bb}},
	{{0x00072c9aaa3221b1,0x000267774474f74d,0x000064b0e9b28085,0x0003f04ef53b27c9,0x0001d6edd5d2e531},{0x00025cd0944ea3bf,0x00075673b81a4d63,0x000150b925d1c0d4,0x00013f38d9294114,0x000461bea69283c9},{0x00036dc801b8b3a2,0x0000e0a7d4935e30,0x0001deb7cecc0d7d,0x000053a94e20dd2c,0x0007a9fbb1c6a0f9}},
	{{0x0006217e039d8064,0x0006dea408337e6d,0x00057ac112628206,0x000647cb65e30473,0x00049c05a51fadc9},{0x0006678aa6a8632f,0x0005ea3788d8b365,0x00021bd6d6994279,0x0007ace75919e4e3,0x00034b9ed338add7},{0x0004e8bf9045af1b,0x000514e33a45e0d6,0x0007533c5b8bfe0f,0x000583557b7e14c9,0x00073c172021b008}},
	{{0x00075b0249864348,0x00052ee11070262b,0x000237ae54fb5acd,0x0003bfd1d03aaab5,0x00018ab598029d5c},{0x000700848a802ade,0x0001e04605c4e5f7,0x0005c0d01b9767fb,0x0007d7889f42388b,0x0004275aae2546d8},{0x00032cc5fd6089e9,0x000426505c949b05,0x00046a18880c7ad2,0x0004a4221888ccda,0x0003dc65522b53df}},
	{{0x0007013b327fbf93,0x0001336eeded6a0d,0x0002b565a2bbf3af,0x000253ce89591955,0x0000267882d17602},{0x0000c222a2007f6d,0x000356b79bdb77ee,0x00041ee81efe12ce,0x000120a9bd07097d,0x000234fd7eec346f},{0x0000a119732ea378,0x00063bf1ba8e2a6c,0x00069f94cc90df9a,0x000431d1779bfc48,0x000497ba6fdaa097}},
	{{0x0003cd86468ccf0b,0x00048553221ac081,0x0006c9464b4e0a6e,0x00075fba84180403,0x00043b5cd4218d05},{0x0006cc0313cfeaa0,0x0001a313848da499,0x0007cb534219230a,0x00039596dedefd60,0x00061e22917f12de},{0x0002762f9bd0b516,0x0001c6e7fbddcbb3,0x00075909c3ace2bd,0x00042101972d3ec9,0x000511d61210ae4d}},
	{{0x000386484420de87,0x0002d6b25db68102,0x000650b4962873c0,0x0004081cfd271394,0x00071a7fe6fe2482},{0x000676ef950e9d81,0x0001b81ae089f258,0x00063c4922951883,0x0002f1d54d9b3237,0x0006d325924ddb85},{0x000182b8a5c8c854,0x00073fcbe5406d8e,0x0005de3430cff451,0x000554b967ac8c41,0x0004746c4b6559ee}},
	{{0x000546c864741147,0x0003a1df99092690,0x0001ca8cc9f4d6bb,0x00036b7fc9cd3b03,0x000219663497db5e},{0x00077b3c6dc69a2b,0x0004edf13ec2fa6e,0x0004e85ad77beac8,0x0007dba2b28e7bda,0x0005c9a51de34fe9},{0x0000f1cf79f10e67,0x00043ccb0a2b7ea2,0x00005089dfff776a,0x0001dd84e1d38b88,0x0004804503c60822}},
	{{0x000021d23a36d175,0x0004fd3373c6476d,0x00020e291eeed02a,0x00062f2ecf2e7210,0x000771e098858de4},{0x00049ed02ca37fc7,0x000474c2b5957884,0x0005b8388e816683,0x0004b6c454b76be4,0x000553398a516506},{0x0002f5d278451edf,0x000730b133997342,0x0006965420eb6975,0x000308a3bfa516cf,0x0005a5ed1d68ff5a}},
	{{0x0005e0c558527359,0x0003395b73afd75c,0x000072afa4e4b970,0x00062214329e0f6d,0x000019b60135fefd},{0x0005122afe150e83,0x0004afc966bb0232,0x0001c478833c8268,0x00017839c3fc148f,0x00044acb897d8bf9},{0x000068145e134b83,0x0001e4860982c3cc,0x000068fb5f13d799,0x0007c9283744547e,0x000150c49fde6ad2}},
	{{0x0001863c9cdca868,0x0003770e295a1709,0x0000d85a3720fd13,0x0005e0ff1f71ab06,0x00078a6d7791e05f},{0x0003f29509471138,0x000729eeb4ca31cf,0x00069c22b575bfbc,0x0004910857bce212,0x0006b2b5a075bb99},{0x0007704b47a0b976,0x0002ae82e91aab17,0x00050bd6429806cd,0x00068055158fd8ea,0x000725c7ffc4ad55}},
	{{0x00002bf71cd098c0,0x00049dabcc6cd230,0x00040a6533f905b2,0x000573efac2eb8a4,0x0004cd54625f855f},{0x00026715d1cf99b2,0x0002205441a69c88,0x000448427dcd4b54,0x0001d191e88abdc5,0x000794cc9277cb1f},{0x0006c426c2ac5053,0x0005a65ece4b095e,0x0000c44086f26bb6,0x0007429568197885,0x0007008357b6fcc8}},
	{{0x00039fbb82584a34,0x00047a568f257a03,0x00014d88091ead91,0x0002145b18b1ce24,0x00013a92a3669d6d},{0x0000672738773f01,0x000752bf799f6171,0x0006b4a6dae33323,0x0007b54696ead1dc,0x00006ef7e9851ad0},{0x0003771cc0577de5,0x0003ca06bb8b9952,0x00000b81c5d50390,0x00043512340780ec,0x0003c296ddf8a2af}},
	{{0x00034d2ebb1f2541,0x0000e815b723ff9d,0x000286b416e25443,0x0000bdfe38d1bee8,0x0000a892c7007477},{0x000515f9d914a713,0x00073191ff2255d5,0x00054f5cc2a4bdef,0x0003dd57fc118bcf,0x0007a99d393490c7},{0x0002ed2436bda3e8,0x00002afd00f291ea,0x0000be7381dea321,0x0003e952d4b2b193,0x000286762d28302f}},
	{{0x00058e2bce2ef5bd,0x00068ce8f78c6f8a,0x0006ee26e39261b2,0x00033d0aa50bcf9d,0x0007686f2a3d6f17},{0x000036093ce35b25,0x0003b64d7552e9cf,0x00071ee0fe0b8460,0x00069d0660c969e5,0x00032f1da046a9d9},{0x000512a66d597c6a,0x0000609a70a57551,0x000026c08a3c464c,0x0004531fc8ee39e1,0x000561305f8a9ad2}},
	{{0x0002cc28e7b0c0d5,0x00077b60eb8a6ce4,0x0004042985c277a6,0x000636657b46d3eb,0x000030a1aef2c57c},{0x0004978dec92aed1,0x000069adae7ca201,0x00011ee923290f55,0x00069641898d916c,0x00000aaec53e35d4},{0x0001f773003ad2aa,0x000005642cc10f76,0x00003b48f82cfca6,0x0002403c10ee4329,0x00020be9c1c24065}},
	{{0x0000e44ae2025e60,0x0005f97b9727041c,0x0005683472c0ecec,0x000188882eb1ce7c,0x00069764c545067e},{0x000387d8249673a6,0x0005bea8dc927c2a,0x0005bd8ed5650ef0,0x0000ef0e3fcd40e1,0x000750ab3361f0ac},{0x00023283a2f81037,0x000477aff97e23d1,0x0000b8958dbcbb68,0x0000205b97e8add6,0x00054f96b3fb7075}},
	{{0x0005afc616b11ecd,0x00039f4aec8f22ef,0x0003b39e1625d92e,0x0005f85bd4508873,0x00078e6839fbe85d},{0x0005f20429669279,0x00008fafae4941f5,0x00015d83c4eb7688,0x0001cf379eca4146,0x0003d7fe9c52bb75},{0x00032df737b8856b,0x0000608342f14e06,0x0003967889d74175,0x0001211907fba550,0x00070f268f350088}},
	{{0x0004112070dcf355,0x0007dcff9c22e464,0x00054ada60e03325,0x00025cd98eef769a,0x000404e56c039b8c},{0x00064583b1805f47,0x00022c1baf832cd0,0x000132c01bd4d717,0x0004ecf4c3a75b8f,0x0007c0d345cfad88},{0x00071f4b8c78338a,0x00062cfc16bc2b23,0x00017cf51280d9aa,0x0003bbae5e20a95a,0x00020d754762aaec}},
	{{0x0004feb135b9f543,0x00063bd192ad93ae,0x00044e2ea612cdf7,0x000670f4991583ab,0x00038b8ada8790b4},{0x0007c36fc73bb758,0x0004a6c797734bd1,0x0000ef248ab3950e,0x00063154c9a53ec8,0x0002b8f1e46f3cee},{0x00004a9cdf51f95d,0x0005d963fbd596b8,0x00022d9b68ace54a,0x0004a98e8836c599,0x000049aeb32ceba1}},
	{{0x00067d3c63dcfe7e,0x000112f0adc81aee,0x00053df04c827165,0x0002fe5b33b430f0,0x00051c665e0c8d62},{0x00007d0b75fc7931,0x00016f4ce4ba754a,0x0005ace4c03fbe49,0x00027e0ec12a159c,0x000795ee17530f67},{0x00025b0a52ecbd81,0x0005dc0695fce4a9,0x0003b928c575047d,0x00023bf3512686e5,0x0006cd19bf49dc54}},
	{{0x0007619052179ca3,0x0000c16593f0afd0,0x000265c4795c7428,0x00031c40515d5442,0x0007520f3db40b2e},{0x0006612165afc386,0x0001171aa36203ff,0x0002642ea820a8aa,0x0001f3bb7b313f10,0x0005e01b3a7429e4},{0x00050be3d39357a1,0x0003ab33d294a7b6,0x0004c479ba59edb3,0x0004c30d184d326f,0x00071092c9ccef3c}},
	{{0x0000523f0364918c,0x000687f56d638a7b,0x00020796928ad013,0x0005d38405a54f33,0x0000ea15b03d0257},{0x0003d8ac74051dcf,0x00010ab6f543d0ad,0x0005d0f3ac0fda90,0x0005ef1d2573e5e4,0x0004173a5bb7137a},{0x00056e31f0f9218a,0x0005635f88e102f8,0x0002cbc5d969a5b8,0x000533fbc98b347a,0x0005fc565614a4e3}},
	{{0x0006570dc46d7ae5,0x00018a9f1b91e26d,0x000436b6183f42ab,0x000550acaa4f8198,0x00062711c414c454},{0x0002e1e67790988e,0x0001e38b9ae44912,0x000648fbb4075654,0x00028df1d840cd72,0x0003214c7409d466},{0x0001827406651770,0x0004d144f286c265,0x00017488f0ee9281,0x00019e6cdb5c760c,0x0005bea94073ecb8}},
	{{0x0005bf0912c89be4,0x00062fadcaf38c83,0x00025ec196b3ce2c,0x00077655ff4f017b,0x0003aacd5c148f61},{0x0000ce63f343d2f8,0x0001e0a87d1e368e,0x000045edbc019eea,0x0006979aed28d0d1,0x0004ad0785944f1b},{0x00063b34c3318301,0x0000e0e62d04d0b1,0x000676a233726701,0x00029e9a042d9769,0x0003aff0cb1d9028}},
	{{0x0005c7eb3a20405e,0x0005fdb5aad930f8,0x0004a757e63b8c47,0x00028e9492972456,0x000110e7e86f4cd2},{0x0006430bf4c53505,0x000264c3e4507244,0x00074c9f19a39270,0x00073f84f799bc47,0x0002ccf9f732bd99},{0x0000d89ed603f5e4,0x00051e1604018af8,0x0000b8eedc4a2218,0x00051ba98b9384d0,0x00005c557e0b9693}},
	{{0x0001ce311fc97e6f,0x0006023f3fb5db1f,0x0007b49775e8fc98,0x0003ad70adbf5045,0x0006e154c178fe98},{0x0006bbb089c20eb0,0x0006df41fb0b9eee,0x00051087ed87e16f,0x000102db5c9fa731,0x000289fef0841861},{0x00016336fed69abf,0x0004f066b929f9ec,0x0004e9ff9e6c5b93,0x00018c89bc4bb2ba,0x0006afbf642a95ca}},
	{{0x0000de0c62f5d2c1,0x00049601cf734fb5,0x0006b5c38263f0f6,0x0004623ef5b56d06,0x0000db4b851b9503},{0x00055070f913a8cc,0x000765619eac2bbc,0x0003ab5225f47459,0x00076ced14ab5b48,0x00012c093cedb801},{0x00047f9308b8190f,0x000414235c621f82,0x00031f5ff41a5a76,0x0006736773aab96d,0x00033aa8799c6635}},
	{{0x0007f51ebd085cf2,0x00012cfa67e3f5e1,0x0001800cf1e3d46a,0x00054337615ff0a8,0x000233c6f29e8e21},{0x0000f588fc156cb1,0x000363414da4f069,0x0007296ad9b68aea,0x0004d3711316ae43,0x000212cd0c1c8d58},{0x0004d5107f18c781,0x00064a4fd3a51a5e,0x0004f4cd0448bb37,0x000671d38543151e,0x0001db7778911914}},
	{{0x000352397c6bc26f,0x00018a7aa0227bbe,0x0005e68cc1ea5f8b,0x0006fe3e3a7a1d5f,0x00031ad97ad26e2a},{0x00014769dd701ab6,0x00028339f1b4b667,0x0004ab214b8ae37b,0x00025f0aefa0b0fe,0x0007ae2ca8a017d2},{0x000017ed0920b962,0x000187e33b53b6fd,0x00055829907a1463,0x000641f248e0a792,0x0001ed1fc53a6622}}
};

// include ed25519-randombytes.h

/*
	ISAAC+ "variant", the paper is not clear on operator precedence and other
	things. This is the "first in, first out" option!
	Not threadsafe or securely initialized, only for deterministic testing
*/
typedef struct isaacp_state_t {
	uint32_t state[256];
	unsigned char buffer[1024];
	uint32_t a, b, c;
	size_t left;
} isaacp_state;

#define isaacp_step(offset, mix) \
	x = mm[i + offset]; \
	a = (a ^ (mix)) + (mm[(i + offset + 128) & 0xff]); \
	y = (a ^ b) + mm[(x >> 2) & 0xff]; \
	mm[i + offset] = y; \
	b = (x + a) ^ mm[(y >> 10) & 0xff]; \
	U32TO8_LE(out + (i + offset) * 4, b);

static void
isaacp_mix(isaacp_state *st) {
	uint32_t i, x, y;
	uint32_t a = st->a, b = st->b, c = st->c;
	uint32_t *mm = st->state;
	unsigned char *out = st->buffer;

	c = c + 1;
	b = b + c;

	for (i = 0; i < 256; i += 4) {
		isaacp_step(0, ROTL32(a,13))
		isaacp_step(1, ROTR32(a, 6))
		isaacp_step(2, ROTL32(a, 2))
		isaacp_step(3, ROTR32(a,16))
	}

	st->a = a;
	st->b = b;
	st->c = c;
	st->left = 1024;
}

static void
isaacp_random(isaacp_state *st, void *p, size_t len) {
	size_t use;
	unsigned char *c = (unsigned char *)p;
	while (len) {
		use = (len > st->left) ? st->left : len;
		memcpy(c, st->buffer + (sizeof(st->buffer) - st->left), use);

		st->left -= use;
		c += use;
		len -= use;

		if (!st->left)
			isaacp_mix(st);
	}
}

// void
static void ed25519_randombytes_unsafe (void *p, size_t len) {
// ED25519_FN(ed25519_randombytes_unsafe) (void *p, size_t len) {
	static int initialized = 0;
	static isaacp_state rng;

	if (!initialized) {
		memset(&rng, 0, sizeof(rng));
		isaacp_mix(&rng);
		isaacp_mix(&rng);
		initialized = 1;
	}

	isaacp_random(&rng, p, len);
}

// 102 "ed25519-donna-portable-identify.h" 2
// 2 "ed25519-donna-portable.h" 2
// 50 "ed25519-donna-portable.h"
   typedef unsigned __int128 uint128_t;
// 91 "ed25519-donna-portable.h"
static inline void U32TO8_LE(unsigned char *p, const uint32_t v) {
 p[0] = (unsigned char)(v );
 p[1] = (unsigned char)(v >> 8);
 p[2] = (unsigned char)(v >> 16);
 p[3] = (unsigned char)(v >> 24);
}
// 108 "ed25519-donna-portable.h"
static inline uint64_t U8TO64_LE(const unsigned char *p) {
 return
 (((uint64_t)(p[0]) ) |
  ((uint64_t)(p[1]) << 8) |
  ((uint64_t)(p[2]) << 16) |
  ((uint64_t)(p[3]) << 24) |
  ((uint64_t)(p[4]) << 32) |
  ((uint64_t)(p[5]) << 40) |
  ((uint64_t)(p[6]) << 48) |
  ((uint64_t)(p[7]) << 56));
}

static inline void U64TO8_LE(unsigned char *p, const uint64_t v) {
 p[0] = (unsigned char)(v );
 p[1] = (unsigned char)(v >> 8);
 p[2] = (unsigned char)(v >> 16);
 p[3] = (unsigned char)(v >> 24);
 p[4] = (unsigned char)(v >> 32);
 p[5] = (unsigned char)(v >> 40);
 p[6] = (unsigned char)(v >> 48);
 p[7] = (unsigned char)(v >> 56);
}

// 134 "ed25519-donna-portable.h" 2
// 13 "ed25519-donna.h" 2
// 47 "ed25519-donna.h"
// 1 "curve25519-donna-64bit.h" 1
// 9 "curve25519-donna-64bit.h"
/*   typedef uint64_t bignum25519[5];   */

static const uint64_t reduce_mask_40 = ((uint64_t)1 << 40) - 1;
static const uint64_t reduce_mask_51 = ((uint64_t)1 << 51) - 1;
static const uint64_t reduce_mask_56 = ((uint64_t)1 << 56) - 1;


inline __attribute__((always_inline)) static void
curve25519_copy(bignum25519 out, const bignum25519 in) {
 out[0] = in[0];
 out[1] = in[1];
 out[2] = in[2];
 out[3] = in[3];
 out[4] = in[4];
}


inline __attribute__((always_inline)) static void
curve25519_add(bignum25519 out, const bignum25519 a, const bignum25519 b) {
 out[0] = a[0] + b[0];
 out[1] = a[1] + b[1];
 out[2] = a[2] + b[2];
 out[3] = a[3] + b[3];
 out[4] = a[4] + b[4];
}


inline __attribute__((always_inline)) static void
curve25519_add_after_basic(bignum25519 out, const bignum25519 a, const bignum25519 b) {
 out[0] = a[0] + b[0];
 out[1] = a[1] + b[1];
 out[2] = a[2] + b[2];
 out[3] = a[3] + b[3];
 out[4] = a[4] + b[4];
}

inline __attribute__((always_inline)) static void
curve25519_add_reduce(bignum25519 out, const bignum25519 a, const bignum25519 b) {
 uint64_t c;
 out[0] = a[0] + b[0] ; c = (out[0] >> 51); out[0] &= reduce_mask_51;
 out[1] = a[1] + b[1] + c; c = (out[1] >> 51); out[1] &= reduce_mask_51;
 out[2] = a[2] + b[2] + c; c = (out[2] >> 51); out[2] &= reduce_mask_51;
 out[3] = a[3] + b[3] + c; c = (out[3] >> 51); out[3] &= reduce_mask_51;
 out[4] = a[4] + b[4] + c; c = (out[4] >> 51); out[4] &= reduce_mask_51;
 out[0] += c * 19;
}


static const uint64_t twoP0 = 0x0fffffffffffda;
static const uint64_t twoP1234 = 0x0ffffffffffffe;
static const uint64_t fourP0 = 0x1fffffffffffb4;
static const uint64_t fourP1234 = 0x1ffffffffffffc;


inline __attribute__((always_inline)) static void
curve25519_sub(bignum25519 out, const bignum25519 a, const bignum25519 b) {
 out[0] = a[0] + twoP0 - b[0];
 out[1] = a[1] + twoP1234 - b[1];
 out[2] = a[2] + twoP1234 - b[2];
 out[3] = a[3] + twoP1234 - b[3];
 out[4] = a[4] + twoP1234 - b[4];
}


inline __attribute__((always_inline)) static void
curve25519_sub_after_basic(bignum25519 out, const bignum25519 a, const bignum25519 b) {
 out[0] = a[0] + fourP0 - b[0];
 out[1] = a[1] + fourP1234 - b[1];
 out[2] = a[2] + fourP1234 - b[2];
 out[3] = a[3] + fourP1234 - b[3];
 out[4] = a[4] + fourP1234 - b[4];
}

inline __attribute__((always_inline)) static void
curve25519_sub_reduce(bignum25519 out, const bignum25519 a, const bignum25519 b) {
 uint64_t c;
 out[0] = a[0] + fourP0 - b[0] ; c = (out[0] >> 51); out[0] &= reduce_mask_51;
 out[1] = a[1] + fourP1234 - b[1] + c; c = (out[1] >> 51); out[1] &= reduce_mask_51;
 out[2] = a[2] + fourP1234 - b[2] + c; c = (out[2] >> 51); out[2] &= reduce_mask_51;
 out[3] = a[3] + fourP1234 - b[3] + c; c = (out[3] >> 51); out[3] &= reduce_mask_51;
 out[4] = a[4] + fourP1234 - b[4] + c; c = (out[4] >> 51); out[4] &= reduce_mask_51;
 out[0] += c * 19;
}


inline __attribute__((always_inline)) static void
curve25519_neg(bignum25519 out, const bignum25519 a) {
 uint64_t c;
 out[0] = twoP0 - a[0] ; c = (out[0] >> 51); out[0] &= reduce_mask_51;
 out[1] = twoP1234 - a[1] + c; c = (out[1] >> 51); out[1] &= reduce_mask_51;
 out[2] = twoP1234 - a[2] + c; c = (out[2] >> 51); out[2] &= reduce_mask_51;
 out[3] = twoP1234 - a[3] + c; c = (out[3] >> 51); out[3] &= reduce_mask_51;
 out[4] = twoP1234 - a[4] + c; c = (out[4] >> 51); out[4] &= reduce_mask_51;
 out[0] += c * 19;
}


inline __attribute__((always_inline)) static void
curve25519_mul(bignum25519 out, const bignum25519 in2, const bignum25519 in) {



 uint128_t t[5];
 uint64_t r0,r1,r2,r3,r4,s0,s1,s2,s3,s4,c;

 r0 = in[0];
 r1 = in[1];
 r2 = in[2];
 r3 = in[3];
 r4 = in[4];

 s0 = in2[0];
 s1 = in2[1];
 s2 = in2[2];
 s3 = in2[3];
 s4 = in2[4];


 t[0] = ((uint128_t) r0) * s0;
 t[1] = ((uint128_t) r0) * s1 + ((uint128_t) r1) * s0;
 t[2] = ((uint128_t) r0) * s2 + ((uint128_t) r2) * s0 + ((uint128_t) r1) * s1;
 t[3] = ((uint128_t) r0) * s3 + ((uint128_t) r3) * s0 + ((uint128_t) r1) * s2 + ((uint128_t) r2) * s1;
 t[4] = ((uint128_t) r0) * s4 + ((uint128_t) r4) * s0 + ((uint128_t) r3) * s1 + ((uint128_t) r1) * s3 + ((uint128_t) r2) * s2;
// 140 "curve25519-donna-64bit.h"
 r1 *= 19;
 r2 *= 19;
 r3 *= 19;
 r4 *= 19;


 t[0] += ((uint128_t) r4) * s1 + ((uint128_t) r1) * s4 + ((uint128_t) r2) * s3 + ((uint128_t) r3) * s2;
 t[1] += ((uint128_t) r4) * s2 + ((uint128_t) r2) * s4 + ((uint128_t) r3) * s3;
 t[2] += ((uint128_t) r4) * s3 + ((uint128_t) r3) * s4;
 t[3] += ((uint128_t) r4) * s4;
// 158 "curve25519-donna-64bit.h"
                      r0 = ((uint64_t)t[0]) & reduce_mask_51; c = (uint64_t)(t[0] >> (51));;
 t[1] += (uint64_t)c; r1 = ((uint64_t)t[1]) & reduce_mask_51; c = (uint64_t)(t[1] >> (51));;
 t[2] += (uint64_t)c; r2 = ((uint64_t)t[2]) & reduce_mask_51; c = (uint64_t)(t[2] >> (51));;
 t[3] += (uint64_t)c; r3 = ((uint64_t)t[3]) & reduce_mask_51; c = (uint64_t)(t[3] >> (51));;
 t[4] += (uint64_t)c; r4 = ((uint64_t)t[4]) & reduce_mask_51; c = (uint64_t)(t[4] >> (51));;
 r0 += c * 19; c = r0 >> 51; r0 = r0 & reduce_mask_51;
 r1 += c;

 out[0] = r0;
 out[1] = r1;
 out[2] = r2;
 out[3] = r3;
 out[4] = r4;
}

__attribute__((noinline)) static void
curve25519_mul_noinline(bignum25519 out, const bignum25519 in2, const bignum25519 in) {
 curve25519_mul(out, in2, in);
}


__attribute__((noinline)) static void
curve25519_square_times(bignum25519 out, const bignum25519 in, uint64_t count) {



 uint128_t t[5];
 uint64_t r0,r1,r2,r3,r4,c;
 uint64_t d0,d1,d2,d4,d419;

 r0 = in[0];
 r1 = in[1];
 r2 = in[2];
 r3 = in[3];
 r4 = in[4];

 do {
  d0 = r0 * 2;
  d1 = r1 * 2;
  d2 = r2 * 2 * 19;
  d419 = r4 * 19;
  d4 = d419 * 2;


  t[0] = ((uint128_t) r0) * r0 + ((uint128_t) d4) * r1 + (((uint128_t) d2) * (r3 ));
  t[1] = ((uint128_t) d0) * r1 + ((uint128_t) d4) * r2 + (((uint128_t) r3) * (r3 * 19));
  t[2] = ((uint128_t) d0) * r2 + ((uint128_t) r1) * r1 + (((uint128_t) d4) * (r3 ));
  t[3] = ((uint128_t) d0) * r3 + ((uint128_t) d1) * r2 + (((uint128_t) r4) * (d419 ));
  t[4] = ((uint128_t) d0) * r4 + ((uint128_t) d1) * r3 + (((uint128_t) r2) * (r2 ));
// 215 "curve25519-donna-64bit.h"
  r0 = ((uint64_t)t[0]) & reduce_mask_51;
  r1 = ((uint64_t)t[1]) & reduce_mask_51; c = (uint64_t)((t[0] << 13) >> 64);; r1 += c;
  r2 = ((uint64_t)t[2]) & reduce_mask_51; c = (uint64_t)((t[1] << 13) >> 64);; r2 += c;
  r3 = ((uint64_t)t[3]) & reduce_mask_51; c = (uint64_t)((t[2] << 13) >> 64);; r3 += c;
  r4 = ((uint64_t)t[4]) & reduce_mask_51; c = (uint64_t)((t[3] << 13) >> 64);; r4 += c;
                                     c = (uint64_t)((t[4] << 13) >> 64);; r0 += c * 19;
                 c = r0 >> 51; r0 &= reduce_mask_51;
  r1 += c ; c = r1 >> 51; r1 &= reduce_mask_51;
  r2 += c ; c = r2 >> 51; r2 &= reduce_mask_51;
  r3 += c ; c = r3 >> 51; r3 &= reduce_mask_51;
  r4 += c ; c = r4 >> 51; r4 &= reduce_mask_51;
  r0 += c * 19;
 } while(--count);

 out[0] = r0;
 out[1] = r1;
 out[2] = r2;
 out[3] = r3;
 out[4] = r4;
}

inline __attribute__((always_inline)) static void
curve25519_square(bignum25519 out, const bignum25519 in) {



 uint128_t t[5];
 uint64_t r0,r1,r2,r3,r4,c;
 uint64_t d0,d1,d2,d4,d419;

 r0 = in[0];
 r1 = in[1];
 r2 = in[2];
 r3 = in[3];
 r4 = in[4];

 d0 = r0 * 2;
 d1 = r1 * 2;
 d2 = r2 * 2 * 19;
 d419 = r4 * 19;
 d4 = d419 * 2;


 t[0] = ((uint128_t) r0) * r0 + ((uint128_t) d4) * r1 + (((uint128_t) d2) * (r3 ));
 t[1] = ((uint128_t) d0) * r1 + ((uint128_t) d4) * r2 + (((uint128_t) r3) * (r3 * 19));
 t[2] = ((uint128_t) d0) * r2 + ((uint128_t) r1) * r1 + (((uint128_t) d4) * (r3 ));
 t[3] = ((uint128_t) d0) * r3 + ((uint128_t) d1) * r2 + (((uint128_t) r4) * (d419 ));
 t[4] = ((uint128_t) d0) * r4 + ((uint128_t) d1) * r3 + (((uint128_t) r2) * (r2 ));
// 271 "curve25519-donna-64bit.h"
                      r0 = ((uint64_t)t[0]) & reduce_mask_51; c = (uint64_t)(t[0] >> (51));;
 t[1] += (uint64_t)c; r1 = ((uint64_t)t[1]) & reduce_mask_51; c = (uint64_t)(t[1] >> (51));;
 t[2] += (uint64_t)c; r2 = ((uint64_t)t[2]) & reduce_mask_51; c = (uint64_t)(t[2] >> (51));;
 t[3] += (uint64_t)c; r3 = ((uint64_t)t[3]) & reduce_mask_51; c = (uint64_t)(t[3] >> (51));;
 t[4] += (uint64_t)c; r4 = ((uint64_t)t[4]) & reduce_mask_51; c = (uint64_t)(t[4] >> (51));;
 r0 += c * 19; c = r0 >> 51; r0 = r0 & reduce_mask_51;
 r1 += c;

 out[0] = r0;
 out[1] = r1;
 out[2] = r2;
 out[3] = r3;
 out[4] = r4;
}


inline __attribute__((always_inline)) static void
curve25519_expand(bignum25519 out, const unsigned char *in) {
 static const union { uint8_t b[2]; uint16_t s; } endian_check = {{1,0}};
 uint64_t x0,x1,x2,x3;

 if (endian_check.s == 1) {
  x0 = *(uint64_t *)(in + 0);
  x1 = *(uint64_t *)(in + 8);
  x2 = *(uint64_t *)(in + 16);
  x3 = *(uint64_t *)(in + 24);
 } else {
// 308 "curve25519-donna-64bit.h"
  x0 = ((((uint64_t)in[0 + 0]) ) | (((uint64_t)in[0 + 1]) << 8) | (((uint64_t)in[0 + 2]) << 16) | (((uint64_t)in[0 + 3]) << 24) | (((uint64_t)in[0 + 4]) << 32) | (((uint64_t)in[0 + 5]) << 40) | (((uint64_t)in[0 + 6]) << 48) | (((uint64_t)in[0 + 7]) << 56));
  x1 = ((((uint64_t)in[8 + 0]) ) | (((uint64_t)in[8 + 1]) << 8) | (((uint64_t)in[8 + 2]) << 16) | (((uint64_t)in[8 + 3]) << 24) | (((uint64_t)in[8 + 4]) << 32) | (((uint64_t)in[8 + 5]) << 40) | (((uint64_t)in[8 + 6]) << 48) | (((uint64_t)in[8 + 7]) << 56));
  x2 = ((((uint64_t)in[16 + 0]) ) | (((uint64_t)in[16 + 1]) << 8) | (((uint64_t)in[16 + 2]) << 16) | (((uint64_t)in[16 + 3]) << 24) | (((uint64_t)in[16 + 4]) << 32) | (((uint64_t)in[16 + 5]) << 40) | (((uint64_t)in[16 + 6]) << 48) | (((uint64_t)in[16 + 7]) << 56));
  x3 = ((((uint64_t)in[24 + 0]) ) | (((uint64_t)in[24 + 1]) << 8) | (((uint64_t)in[24 + 2]) << 16) | (((uint64_t)in[24 + 3]) << 24) | (((uint64_t)in[24 + 4]) << 32) | (((uint64_t)in[24 + 5]) << 40) | (((uint64_t)in[24 + 6]) << 48) | (((uint64_t)in[24 + 7]) << 56));
 }

 out[0] = x0 & reduce_mask_51; x0 = (x0 >> 51) | (x1 << 13);
 out[1] = x0 & reduce_mask_51; x1 = (x1 >> 38) | (x2 << 26);
 out[2] = x1 & reduce_mask_51; x2 = (x2 >> 25) | (x3 << 39);
 out[3] = x2 & reduce_mask_51; x3 = (x3 >> 12);
 out[4] = x3 & reduce_mask_51;
}




inline __attribute__((always_inline)) static void
curve25519_contract(unsigned char *out, const bignum25519 input) {
 uint64_t t[5];
 uint64_t f, i;

 t[0] = input[0];
 t[1] = input[1];
 t[2] = input[2];
 t[3] = input[3];
 t[4] = input[4];
// 347 "curve25519-donna-64bit.h"
 t[1] += t[0] >> 51; t[0] &= reduce_mask_51; t[2] += t[1] >> 51; t[1] &= reduce_mask_51; t[3] += t[2] >> 51; t[2] &= reduce_mask_51; t[4] += t[3] >> 51; t[3] &= reduce_mask_51; t[0] += 19 * (t[4] >> 51); t[4] &= reduce_mask_51;
 t[1] += t[0] >> 51; t[0] &= reduce_mask_51; t[2] += t[1] >> 51; t[1] &= reduce_mask_51; t[3] += t[2] >> 51; t[2] &= reduce_mask_51; t[4] += t[3] >> 51; t[3] &= reduce_mask_51; t[0] += 19 * (t[4] >> 51); t[4] &= reduce_mask_51;



 t[0] += 19;
 t[1] += t[0] >> 51; t[0] &= reduce_mask_51; t[2] += t[1] >> 51; t[1] &= reduce_mask_51; t[3] += t[2] >> 51; t[2] &= reduce_mask_51; t[4] += t[3] >> 51; t[3] &= reduce_mask_51; t[0] += 19 * (t[4] >> 51); t[4] &= reduce_mask_51;


 t[0] += (reduce_mask_51 + 1) - 19;
 t[1] += (reduce_mask_51 + 1) - 1;
 t[2] += (reduce_mask_51 + 1) - 1;
 t[3] += (reduce_mask_51 + 1) - 1;
 t[4] += (reduce_mask_51 + 1) - 1;


 t[1] += t[0] >> 51; t[0] &= reduce_mask_51; t[2] += t[1] >> 51; t[1] &= reduce_mask_51; t[3] += t[2] >> 51; t[2] &= reduce_mask_51; t[4] += t[3] >> 51; t[3] &= reduce_mask_51; t[4] &= reduce_mask_51;





 f = ((t[0] >> 13*0) | (t[0 +1] << (51 - 13*0))); for (i = 0; i < 8; i++, f >>= 8) *out++ = (unsigned char)f;
 f = ((t[1] >> 13*1) | (t[1 +1] << (51 - 13*1))); for (i = 0; i < 8; i++, f >>= 8) *out++ = (unsigned char)f;
 f = ((t[2] >> 13*2) | (t[2 +1] << (51 - 13*2))); for (i = 0; i < 8; i++, f >>= 8) *out++ = (unsigned char)f;
 f = ((t[3] >> 13*3) | (t[3 +1] << (51 - 13*3))); for (i = 0; i < 8; i++, f >>= 8) *out++ = (unsigned char)f;
}




inline __attribute__((always_inline)) static void
curve25519_move_conditional_bytes(uint8_t out[96], const uint8_t in[96], uint64_t flag) {
 const uint64_t nb = flag - 1, b = ~nb;
 const uint64_t *inq = (const uint64_t *)in;
 uint64_t *outq = (uint64_t *)out;
 outq[0] = (outq[0] & nb) | (inq[0] & b);
 outq[1] = (outq[1] & nb) | (inq[1] & b);
 outq[2] = (outq[2] & nb) | (inq[2] & b);
 outq[3] = (outq[3] & nb) | (inq[3] & b);
 outq[4] = (outq[4] & nb) | (inq[4] & b);
 outq[5] = (outq[5] & nb) | (inq[5] & b);
 outq[6] = (outq[6] & nb) | (inq[6] & b);
 outq[7] = (outq[7] & nb) | (inq[7] & b);
 outq[8] = (outq[8] & nb) | (inq[8] & b);
 outq[9] = (outq[9] & nb) | (inq[9] & b);
 outq[10] = (outq[10] & nb) | (inq[10] & b);
 outq[11] = (outq[11] & nb) | (inq[11] & b);
}


inline __attribute__((always_inline)) static void
curve25519_swap_conditional(bignum25519 a, bignum25519 b, uint64_t iswap) {
 const uint64_t swap = (uint64_t)(-(int64_t)iswap);
 uint64_t x0,x1,x2,x3,x4;

 x0 = swap & (a[0] ^ b[0]); a[0] ^= x0; b[0] ^= x0;
 x1 = swap & (a[1] ^ b[1]); a[1] ^= x1; b[1] ^= x1;
 x2 = swap & (a[2] ^ b[2]); a[2] ^= x2; b[2] ^= x2;
 x3 = swap & (a[3] ^ b[3]); a[3] ^= x3; b[3] ^= x3;
 x4 = swap & (a[4] ^ b[4]); a[4] ^= x4; b[4] ^= x4;
}
// 48 "ed25519-donna.h" 2




// 1 "curve25519-donna-helpers.h" 1
// 12 "curve25519-donna-helpers.h"
static void
curve25519_pow_two5mtwo0_two250mtwo0(bignum25519 b) {
 bignum25519 __attribute__((aligned(16))) t0,c;


                  curve25519_square_times(t0, b, 5);
                  curve25519_mul_noinline(b, t0, b);
                   curve25519_square_times(t0, b, 10);
                  curve25519_mul_noinline(c, t0, b);
                   curve25519_square_times(t0, c, 20);
                  curve25519_mul_noinline(t0, t0, c);
                   curve25519_square_times(t0, t0, 10);
                  curve25519_mul_noinline(b, t0, b);
                    curve25519_square_times(t0, b, 50);
                   curve25519_mul_noinline(c, t0, b);
                     curve25519_square_times(t0, c, 100);
                   curve25519_mul_noinline(t0, t0, c);
                    curve25519_square_times(t0, t0, 50);
                   curve25519_mul_noinline(b, t0, b);
}




static void
curve25519_recip(bignum25519 out, const bignum25519 z) {
 bignum25519 __attribute__((aligned(16))) a,t0,b;

         curve25519_square_times(a, z, 1);
         curve25519_square_times(t0, a, 2);
         curve25519_mul_noinline(b, t0, z);
          curve25519_mul_noinline(a, b, a);
          curve25519_square_times(t0, a, 1);
                      curve25519_mul_noinline(b, t0, b);
                   curve25519_pow_two5mtwo0_two250mtwo0(b);
                   curve25519_square_times(b, b, 5);
                  curve25519_mul_noinline(out, b, a);
}




static void
curve25519_pow_two252m3(bignum25519 two252m3, const bignum25519 z) {
 bignum25519 __attribute__((aligned(16))) b,c,t0;

         curve25519_square_times(c, z, 1);
         curve25519_square_times(t0, c, 2);
         curve25519_mul_noinline(b, t0, z);
          curve25519_mul_noinline(c, b, c);
          curve25519_square_times(t0, c, 1);
                      curve25519_mul_noinline(b, t0, b);
                   curve25519_pow_two5mtwo0_two250mtwo0(b);
                   curve25519_square_times(b, b, 2);
                 curve25519_mul_noinline(two252m3, b, z);
}
// 53 "ed25519-donna.h" 2



// 1 "modm-donna-64bit.h" 1
// 18 "modm-donna-64bit.h"
/*   typedef uint64_t bignum256modm_element_t;   */
/*   typedef bignum256modm_element_t bignum256modm[5];   */

static const bignum256modm modm_m = {
 0x12631a5cf5d3ed,
 0xf9dea2f79cd658,
 0x000000000014de,
 0x00000000000000,
 0x00000010000000
};

static const bignum256modm modm_mu = {
 0x9ce5a30a2c131b,
 0x215d086329a7ed,
 0xffffffffeb2106,
 0xffffffffffffff,
 0x00000fffffffff
};

static bignum256modm_element_t
lt_modm(bignum256modm_element_t a, bignum256modm_element_t b) {
 return (a - b) >> 63;
}

static void
reduce256_modm(bignum256modm r) {
 bignum256modm t;
 bignum256modm_element_t b = 0, pb, mask;


 pb = 0;
 pb += modm_m[0]; b = lt_modm(r[0], pb); t[0] = (r[0] - pb + (b << 56)); pb = b;
 pb += modm_m[1]; b = lt_modm(r[1], pb); t[1] = (r[1] - pb + (b << 56)); pb = b;
 pb += modm_m[2]; b = lt_modm(r[2], pb); t[2] = (r[2] - pb + (b << 56)); pb = b;
 pb += modm_m[3]; b = lt_modm(r[3], pb); t[3] = (r[3] - pb + (b << 56)); pb = b;
 pb += modm_m[4]; b = lt_modm(r[4], pb); t[4] = (r[4] - pb + (b << 32));


 mask = b - 1;

 r[0] ^= mask & (r[0] ^ t[0]);
 r[1] ^= mask & (r[1] ^ t[1]);
 r[2] ^= mask & (r[2] ^ t[2]);
 r[3] ^= mask & (r[3] ^ t[3]);
 r[4] ^= mask & (r[4] ^ t[4]);
}

static void
barrett_reduce256_modm(bignum256modm r, const bignum256modm q1, const bignum256modm r1) {
 bignum256modm q3, r2;
 uint128_t c, mul;
 bignum256modm_element_t f, b, pb;




 c = (uint128_t)modm_mu[0] * q1[3]; mul = (uint128_t)modm_mu[3] * q1[0]; c += mul; mul = (uint128_t)modm_mu[1] * q1[2]; c += mul; mul = (uint128_t)modm_mu[2] * q1[1]; c += mul; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_mu[0] * q1[4]; c += (uint64_t)f; mul = (uint128_t)modm_mu[4] * q1[0]; c += mul; mul = (uint128_t)modm_mu[3] * q1[1]; c += mul; mul = (uint128_t)modm_mu[1] * q1[3]; c += mul; mul = (uint128_t)modm_mu[2] * q1[2]; c += mul;
 f = ((uint64_t)c); q3[0] = (f >> 40) & 0xffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_mu[4] * q1[1]; c += (uint64_t)f; mul = (uint128_t)modm_mu[1] * q1[4]; c += mul; mul = (uint128_t)modm_mu[2] * q1[3]; c += mul; mul = (uint128_t)modm_mu[3] * q1[2]; c += mul;
 f = ((uint64_t)c); q3[0] |= (f << 16) & 0xffffffffffffff; q3[1] = (f >> 40) & 0xffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_mu[4] * q1[2]; c += (uint64_t)f; mul = (uint128_t)modm_mu[2] * q1[4]; c += mul; mul = (uint128_t)modm_mu[3] * q1[3]; c += mul;
 f = ((uint64_t)c); q3[1] |= (f << 16) & 0xffffffffffffff; q3[2] = (f >> 40) & 0xffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_mu[4] * q1[3]; c += (uint64_t)f; mul = (uint128_t)modm_mu[3] * q1[4]; c += mul;
 f = ((uint64_t)c); q3[2] |= (f << 16) & 0xffffffffffffff; q3[3] = (f >> 40) & 0xffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_mu[4] * q1[4]; c += (uint64_t)f;
 f = ((uint64_t)c); q3[3] |= (f << 16) & 0xffffffffffffff; q3[4] = (f >> 40) & 0xffff; f = (uint64_t)(c >> (56));;
 q3[4] |= (f << 16);

 c = (uint128_t)modm_m[0] * q3[0];
 r2[0] = ((uint64_t)c) & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_m[0] * q3[1]; c += (uint64_t)f; mul = (uint128_t)modm_m[1] * q3[0]; c += mul;
 r2[1] = ((uint64_t)c) & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_m[0] * q3[2]; c += (uint64_t)f; mul = (uint128_t)modm_m[2] * q3[0]; c += mul; mul = (uint128_t)modm_m[1] * q3[1]; c += mul;
 r2[2] = ((uint64_t)c) & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_m[0] * q3[3]; c += (uint64_t)f; mul = (uint128_t)modm_m[3] * q3[0]; c += mul; mul = (uint128_t)modm_m[1] * q3[2]; c += mul; mul = (uint128_t)modm_m[2] * q3[1]; c += mul;
 r2[3] = ((uint64_t)c) & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)modm_m[0] * q3[4]; c += (uint64_t)f; mul = (uint128_t)modm_m[4] * q3[0]; c += mul; mul = (uint128_t)modm_m[3] * q3[1]; c += mul; mul = (uint128_t)modm_m[1] * q3[3]; c += mul; mul = (uint128_t)modm_m[2] * q3[2]; c += mul;
 r2[4] = ((uint64_t)c) & 0x0000ffffffffff;

 pb = 0;
 pb += r2[0]; b = lt_modm(r1[0], pb); r[0] = (r1[0] - pb + (b << 56)); pb = b;
 pb += r2[1]; b = lt_modm(r1[1], pb); r[1] = (r1[1] - pb + (b << 56)); pb = b;
 pb += r2[2]; b = lt_modm(r1[2], pb); r[2] = (r1[2] - pb + (b << 56)); pb = b;
 pb += r2[3]; b = lt_modm(r1[3], pb); r[3] = (r1[3] - pb + (b << 56)); pb = b;
 pb += r2[4]; b = lt_modm(r1[4], pb); r[4] = (r1[4] - pb + (b << 40));

 reduce256_modm(r);
 reduce256_modm(r);
}


static void
add256_modm(bignum256modm r, const bignum256modm x, const bignum256modm y) {
 bignum256modm_element_t c;

 c = x[0] + y[0]; r[0] = c & 0xffffffffffffff; c >>= 56;
 c += x[1] + y[1]; r[1] = c & 0xffffffffffffff; c >>= 56;
 c += x[2] + y[2]; r[2] = c & 0xffffffffffffff; c >>= 56;
 c += x[3] + y[3]; r[3] = c & 0xffffffffffffff; c >>= 56;
 c += x[4] + y[4]; r[4] = c;

 reduce256_modm(r);
}

static void
mul256_modm(bignum256modm r, const bignum256modm x, const bignum256modm y) {
 bignum256modm q1, r1;
 uint128_t c, mul;
 bignum256modm_element_t f;

 c = (uint128_t)x[0] * y[0];
 f = ((uint64_t)c); r1[0] = f & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[0] * y[1]; c += (uint64_t)f; mul = (uint128_t)x[1] * y[0]; c += mul;
 f = ((uint64_t)c); r1[1] = f & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[0] * y[2]; c += (uint64_t)f; mul = (uint128_t)x[2] * y[0]; c += mul; mul = (uint128_t)x[1] * y[1]; c += mul;
 f = ((uint64_t)c); r1[2] = f & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[0] * y[3]; c += (uint64_t)f; mul = (uint128_t)x[3] * y[0]; c += mul; mul = (uint128_t)x[1] * y[2]; c += mul; mul = (uint128_t)x[2] * y[1]; c += mul;
 f = ((uint64_t)c); r1[3] = f & 0xffffffffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[0] * y[4]; c += (uint64_t)f; mul = (uint128_t)x[4] * y[0]; c += mul; mul = (uint128_t)x[3] * y[1]; c += mul; mul = (uint128_t)x[1] * y[3]; c += mul; mul = (uint128_t)x[2] * y[2]; c += mul;
 f = ((uint64_t)c); r1[4] = f & 0x0000ffffffffff; q1[0] = (f >> 24) & 0xffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[4] * y[1]; c += (uint64_t)f; mul = (uint128_t)x[1] * y[4]; c += mul; mul = (uint128_t)x[2] * y[3]; c += mul; mul = (uint128_t)x[3] * y[2]; c += mul;
 f = ((uint64_t)c); q1[0] |= (f << 32) & 0xffffffffffffff; q1[1] = (f >> 24) & 0xffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[4] * y[2]; c += (uint64_t)f; mul = (uint128_t)x[2] * y[4]; c += mul; mul = (uint128_t)x[3] * y[3]; c += mul;
 f = ((uint64_t)c); q1[1] |= (f << 32) & 0xffffffffffffff; q1[2] = (f >> 24) & 0xffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[4] * y[3]; c += (uint64_t)f; mul = (uint128_t)x[3] * y[4]; c += mul;
 f = ((uint64_t)c); q1[2] |= (f << 32) & 0xffffffffffffff; q1[3] = (f >> 24) & 0xffffffff; f = (uint64_t)(c >> (56));;
 c = (uint128_t)x[4] * y[4]; c += (uint64_t)f;
 f = ((uint64_t)c); q1[3] |= (f << 32) & 0xffffffffffffff; q1[4] = (f >> 24) & 0xffffffff; f = (uint64_t)(c >> (56));;
 q1[4] |= (f << 32);

 barrett_reduce256_modm(r, q1, r1);
}

static void
expand256_modm(bignum256modm out, const unsigned char *in, size_t len) {
 unsigned char work[64] = {0};
 bignum256modm_element_t x[16];
 bignum256modm q1;

 memcpy(work, in, len);
 x[0] = U8TO64_LE(work + 0);
 x[1] = U8TO64_LE(work + 8);
 x[2] = U8TO64_LE(work + 16);
 x[3] = U8TO64_LE(work + 24);
 x[4] = U8TO64_LE(work + 32);
 x[5] = U8TO64_LE(work + 40);
 x[6] = U8TO64_LE(work + 48);
 x[7] = U8TO64_LE(work + 56);


 out[0] = ( x[0]) & 0xffffffffffffff;
 out[1] = ((x[ 0] >> 56) | (x[ 1] << 8)) & 0xffffffffffffff;
 out[2] = ((x[ 1] >> 48) | (x[ 2] << 16)) & 0xffffffffffffff;
 out[3] = ((x[ 2] >> 40) | (x[ 3] << 24)) & 0xffffffffffffff;
 out[4] = ((x[ 3] >> 32) | (x[ 4] << 32)) & 0x0000ffffffffff;


 if (len < 32)
  return;


 q1[0] = ((x[ 3] >> 56) | (x[ 4] << 8)) & 0xffffffffffffff;
 q1[1] = ((x[ 4] >> 48) | (x[ 5] << 16)) & 0xffffffffffffff;
 q1[2] = ((x[ 5] >> 40) | (x[ 6] << 24)) & 0xffffffffffffff;
 q1[3] = ((x[ 6] >> 32) | (x[ 7] << 32)) & 0xffffffffffffff;
 q1[4] = ((x[ 7] >> 24) );

 barrett_reduce256_modm(out, q1, out);
}

static void
expand_raw256_modm(bignum256modm out, const unsigned char in[32]) {
 bignum256modm_element_t x[4];

 x[0] = U8TO64_LE(in + 0);
 x[1] = U8TO64_LE(in + 8);
 x[2] = U8TO64_LE(in + 16);
 x[3] = U8TO64_LE(in + 24);

 out[0] = ( x[0]) & 0xffffffffffffff;
 out[1] = ((x[ 0] >> 56) | (x[ 1] << 8)) & 0xffffffffffffff;
 out[2] = ((x[ 1] >> 48) | (x[ 2] << 16)) & 0xffffffffffffff;
 out[3] = ((x[ 2] >> 40) | (x[ 3] << 24)) & 0xffffffffffffff;
 out[4] = ((x[ 3] >> 32) ) & 0x000000ffffffff;
}

static void
contract256_modm(unsigned char out[32], const bignum256modm in) {
 U64TO8_LE(out + 0, (in[0] ) | (in[1] << 56));
 U64TO8_LE(out + 8, (in[1] >> 8) | (in[2] << 48));
 U64TO8_LE(out + 16, (in[2] >> 16) | (in[3] << 40));
 U64TO8_LE(out + 24, (in[3] >> 24) | (in[4] << 32));
}

static void
contract256_window4_modm(signed char r[64], const bignum256modm in) {
 char carry;
 signed char *quads = r;
 bignum256modm_element_t i, j, v, m;

 for (i = 0; i < 5; i++) {
  v = in[i];
  m = (i == 4) ? 8 : 14;
  for (j = 0; j < m; j++) {
   *quads++ = (v & 15);
   v >>= 4;
  }
 }


 carry = 0;
 for(i = 0; i < 63; i++) {
  r[i] += carry;
  r[i+1] += (r[i] >> 4);
  r[i] &= 15;
  carry = (r[i] >> 3);
  r[i] -= (carry << 4);
 }
 r[63] += carry;
}

static void
contract256_slidingwindow_modm(signed char r[256], const bignum256modm s, int windowsize) {
 int i,j,k,b;
 int m = (1 << (windowsize - 1)) - 1, soplen = 256;
 signed char *bits = r;
 bignum256modm_element_t v;


 for (i = 0; i < 4; i++) {
  v = s[i];
  for (j = 0; j < 56; j++, v >>= 1)
   *bits++ = (v & 1);
 }
 v = s[4];
 for (j = 0; j < 32; j++, v >>= 1)
  *bits++ = (v & 1);


 for (j = 0; j < soplen; j++) {
  if (!r[j])
   continue;

  for (b = 1; (b < (soplen - j)) && (b <= 6); b++) {
   if ((r[j] + (r[j + b] << b)) <= m) {
    r[j] += r[j + b] << b;
    r[j + b] = 0;
   } else if ((r[j] - (r[j + b] << b)) >= -m) {
    r[j] -= r[j + b] << b;
    for (k = j + b; k < soplen; k++) {
     if (!r[k]) {
      r[k] = 1;
      break;
     }
     r[k] = 0;
    }
   } else if (r[j + b]) {
    break;
   }
  }
 }
}






static void
sub256_modm_batch(bignum256modm out, const bignum256modm a, const bignum256modm b, size_t limbsize) {
 size_t i = 0;
 bignum256modm_element_t carry = 0;
 switch (limbsize) {
  case 4: out[i] = (a[i] - b[i]) ; carry = (out[i] >> 63); out[i] &= 0xffffffffffffff; i++;
  case 3: out[i] = (a[i] - b[i]) - carry; carry = (out[i] >> 63); out[i] &= 0xffffffffffffff; i++;
  case 2: out[i] = (a[i] - b[i]) - carry; carry = (out[i] >> 63); out[i] &= 0xffffffffffffff; i++;
  case 1: out[i] = (a[i] - b[i]) - carry; carry = (out[i] >> 63); out[i] &= 0xffffffffffffff; i++;
  case 0:
  default: out[i] = (a[i] - b[i]) - carry;
 }
}



static int
lt256_modm_batch(const bignum256modm a, const bignum256modm b, size_t limbsize) {
 size_t i = 0;
 bignum256modm_element_t t, carry = 0;
 switch (limbsize) {
  case 4: t = (a[i] - b[i]) ; carry = (t >> 63); i++;
  case 3: t = (a[i] - b[i]) - carry; carry = (t >> 63); i++;
  case 2: t = (a[i] - b[i]) - carry; carry = (t >> 63); i++;
  case 1: t = (a[i] - b[i]) - carry; carry = (t >> 63); i++;
  case 0: t = (a[i] - b[i]) - carry; carry = (t >> 63);
 }
 return (int)carry;
}


static int
lte256_modm_batch(const bignum256modm a, const bignum256modm b, size_t limbsize) {
 size_t i = 0;
 bignum256modm_element_t t, carry = 0;
 switch (limbsize) {
  case 4: t = (b[i] - a[i]) ; carry = (t >> 63); i++;
  case 3: t = (b[i] - a[i]) - carry; carry = (t >> 63); i++;
  case 2: t = (b[i] - a[i]) - carry; carry = (t >> 63); i++;
  case 1: t = (b[i] - a[i]) - carry; carry = (t >> 63); i++;
  case 0: t = (b[i] - a[i]) - carry; carry = (t >> 63);
 }
 return (int)!carry;
}


static int
iszero256_modm_batch(const bignum256modm a) {
 size_t i;
 for (i = 0; i < 5; i++)
  if (a[i])
   return 0;
 return 1;
}


static int
isone256_modm_batch(const bignum256modm a) {
 size_t i;
 for (i = 0; i < 5; i++)
  if (a[i] != ((i) ? (size_t)0 : (size_t)1))
   return 0;
 return 1;
}


static int
isatmost128bits256_modm_batch(const bignum256modm a) {
 uint64_t mask =
  ((a[4] ) |
   (a[3] ) |
   (a[2] & 0xffffffffff0000));

 return (mask == 0);
}
// 57 "ed25519-donna.h" 2




typedef unsigned char hash_512bits[64];




static int
ed25519_verify(const unsigned char *x, const unsigned char *y, size_t len) {
 size_t differentbits = 0;
 while (len--)
  differentbits |= (*x++ ^ *y++);
 return (int) (1 & ((differentbits - 1) >> 8));
}
// 81 "ed25519-donna.h"
/*   typedef struct ge25519_t {   */
/*    bignum25519 x, y, z, t;   */
/*   } ge25519;   */

/*   typedef struct ge25519_p1p1_t {   */
/*    bignum25519 x, y, z, t;   */
/*   } ge25519_p1p1;   */

/*   typedef struct ge25519_niels_t {   */
/*    bignum25519 ysubx, xaddy, t2d;   */
/*   } ge25519_niels;   */

/*   typedef struct ge25519_pniels_t {   */
/*    bignum25519 ysubx, xaddy, z, t2d;   */
/*   } ge25519_pniels;   */

#include "ed25519-donna-basepoint-table.h"

// 101 "ed25519-donna.h" 2
// 1 "ed25519-donna-64bit-x86.h" 1

__attribute__((noinline)) static void
ge25519_scalarmult_base_choose_niels(ge25519_niels *t, const uint8_t table[256][96], uint32_t pos, signed char b) {
 int64_t breg = (int64_t)b;
 uint64_t sign = (uint64_t)breg >> 63;
 uint64_t mask = ~(sign - 1);
 uint64_t u = (breg + mask) ^ mask;

 __asm__ __volatile__ (

  "movq %0, %%rax                  ;\n"
  "movd %%rax, %%xmm14             ;\n"
  "pshufd $0x00, %%xmm14, %%xmm14  ;\n"
  "pxor %%xmm0, %%xmm0             ;\n"
  "pxor %%xmm1, %%xmm1             ;\n"
  "pxor %%xmm2, %%xmm2             ;\n"
  "pxor %%xmm3, %%xmm3             ;\n"
  "pxor %%xmm4, %%xmm4             ;\n"
  "pxor %%xmm5, %%xmm5             ;\n"


  "movq $0, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movq $1, %%rax                  ;\n"
  "movd %%rax, %%xmm6              ;\n"
  "pxor %%xmm7, %%xmm7             ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm6, %%xmm2              ;\n"
  "por %%xmm7, %%xmm3              ;\n"


  "movq $1, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 0(%1), %%xmm6            ;\n"
  "movdqa 16(%1), %%xmm7           ;\n"
  "movdqa 32(%1), %%xmm8           ;\n"
  "movdqa 48(%1), %%xmm9           ;\n"
  "movdqa 64(%1), %%xmm10          ;\n"
  "movdqa 80(%1), %%xmm11          ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $2, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 96(%1), %%xmm6           ;\n"
  "movdqa 112(%1), %%xmm7          ;\n"
  "movdqa 128(%1), %%xmm8          ;\n"
  "movdqa 144(%1), %%xmm9          ;\n"
  "movdqa 160(%1), %%xmm10         ;\n"
  "movdqa 176(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $3, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 192(%1), %%xmm6          ;\n"
  "movdqa 208(%1), %%xmm7          ;\n"
  "movdqa 224(%1), %%xmm8          ;\n"
  "movdqa 240(%1), %%xmm9          ;\n"
  "movdqa 256(%1), %%xmm10         ;\n"
  "movdqa 272(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $4, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 288(%1), %%xmm6          ;\n"
  "movdqa 304(%1), %%xmm7          ;\n"
  "movdqa 320(%1), %%xmm8          ;\n"
  "movdqa 336(%1), %%xmm9          ;\n"
  "movdqa 352(%1), %%xmm10         ;\n"
  "movdqa 368(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $5, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 384(%1), %%xmm6          ;\n"
  "movdqa 400(%1), %%xmm7          ;\n"
  "movdqa 416(%1), %%xmm8          ;\n"
  "movdqa 432(%1), %%xmm9          ;\n"
  "movdqa 448(%1), %%xmm10         ;\n"
  "movdqa 464(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $6, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 480(%1), %%xmm6          ;\n"
  "movdqa 496(%1), %%xmm7          ;\n"
  "movdqa 512(%1), %%xmm8          ;\n"
  "movdqa 528(%1), %%xmm9          ;\n"
  "movdqa 544(%1), %%xmm10         ;\n"
  "movdqa 560(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $7, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 576(%1), %%xmm6          ;\n"
  "movdqa 592(%1), %%xmm7          ;\n"
  "movdqa 608(%1), %%xmm8          ;\n"
  "movdqa 624(%1), %%xmm9          ;\n"
  "movdqa 640(%1), %%xmm10         ;\n"
  "movdqa 656(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq $8, %%rax                  ;\n"
  "movd %%rax, %%xmm15             ;\n"
  "pshufd $0x00, %%xmm15, %%xmm15  ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa 672(%1), %%xmm6          ;\n"
  "movdqa 688(%1), %%xmm7          ;\n"
  "movdqa 704(%1), %%xmm8          ;\n"
  "movdqa 720(%1), %%xmm9          ;\n"
  "movdqa 736(%1), %%xmm10         ;\n"
  "movdqa 752(%1), %%xmm11         ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pand %%xmm15, %%xmm8            ;\n"
  "pand %%xmm15, %%xmm9            ;\n"
  "pand %%xmm15, %%xmm10           ;\n"
  "pand %%xmm15, %%xmm11           ;\n"
  "por %%xmm6, %%xmm0              ;\n"
  "por %%xmm7, %%xmm1              ;\n"
  "por %%xmm8, %%xmm2              ;\n"
  "por %%xmm9, %%xmm3              ;\n"
  "por %%xmm10, %%xmm4             ;\n"
  "por %%xmm11, %%xmm5             ;\n"


  "movq %3, %%rax                  ;\n"
  "xorq $1, %%rax                  ;\n"
  "movd %%rax, %%xmm14             ;\n"
  "pxor %%xmm15, %%xmm15           ;\n"
  "pshufd $0x00, %%xmm14, %%xmm14  ;\n"
  "pxor %%xmm0, %%xmm2             ;\n"
  "pxor %%xmm1, %%xmm3             ;\n"
  "pcmpeqd %%xmm14, %%xmm15        ;\n"
  "movdqa %%xmm2, %%xmm6           ;\n"
  "movdqa %%xmm3, %%xmm7           ;\n"
  "pand %%xmm15, %%xmm6            ;\n"
  "pand %%xmm15, %%xmm7            ;\n"
  "pxor %%xmm6, %%xmm0             ;\n"
  "pxor %%xmm7, %%xmm1             ;\n"
  "pxor %%xmm0, %%xmm2             ;\n"
  "pxor %%xmm1, %%xmm3             ;\n"


  "movq $0x7ffffffffffff, %%rax    ;\n"
  "movd %%xmm0, %%rcx              ;\n"
  "movd %%xmm0, %%r8               ;\n"
  "movd %%xmm1, %%rsi              ;\n"
  "pshufd $0xee, %%xmm0, %%xmm0    ;\n"
  "pshufd $0xee, %%xmm1, %%xmm1    ;\n"
  "movd %%xmm0, %%rdx              ;\n"
  "movd %%xmm1, %%rdi              ;\n"
  "shrdq $51, %%rdx, %%r8          ;\n"
  "shrdq $38, %%rsi, %%rdx         ;\n"
  "shrdq $25, %%rdi, %%rsi         ;\n"
  "shrq $12, %%rdi                 ;\n"
  "andq %%rax, %%rcx               ;\n"
  "andq %%rax, %%r8                ;\n"
  "andq %%rax, %%rdx               ;\n"
  "andq %%rax, %%rsi               ;\n"
  "andq %%rax, %%rdi               ;\n"
  "movq %%rcx, 0(%2)               ;\n"
  "movq %%r8, 8(%2)                ;\n"
  "movq %%rdx, 16(%2)              ;\n"
  "movq %%rsi, 24(%2)              ;\n"
  "movq %%rdi, 32(%2)              ;\n"


  "movq $0x7ffffffffffff, %%rax    ;\n"
  "movd %%xmm2, %%rcx              ;\n"
  "movd %%xmm2, %%r8               ;\n"
  "movd %%xmm3, %%rsi              ;\n"
  "pshufd $0xee, %%xmm2, %%xmm2    ;\n"
  "pshufd $0xee, %%xmm3, %%xmm3    ;\n"
  "movd %%xmm2, %%rdx              ;\n"
  "movd %%xmm3, %%rdi              ;\n"
  "shrdq $51, %%rdx, %%r8          ;\n"
  "shrdq $38, %%rsi, %%rdx         ;\n"
  "shrdq $25, %%rdi, %%rsi         ;\n"
  "shrq $12, %%rdi                 ;\n"
  "andq %%rax, %%rcx               ;\n"
  "andq %%rax, %%r8                ;\n"
  "andq %%rax, %%rdx               ;\n"
  "andq %%rax, %%rsi               ;\n"
  "andq %%rax, %%rdi               ;\n"
  "movq %%rcx, 40(%2)              ;\n"
  "movq %%r8, 48(%2)               ;\n"
  "movq %%rdx, 56(%2)              ;\n"
  "movq %%rsi, 64(%2)              ;\n"
  "movq %%rdi, 72(%2)              ;\n"


  "movq $0x7ffffffffffff, %%rax    ;\n"
  "movd %%xmm4, %%rcx              ;\n"
  "movd %%xmm4, %%r8               ;\n"
  "movd %%xmm5, %%rsi              ;\n"
  "pshufd $0xee, %%xmm4, %%xmm4    ;\n"
  "pshufd $0xee, %%xmm5, %%xmm5    ;\n"
  "movd %%xmm4, %%rdx              ;\n"
  "movd %%xmm5, %%rdi              ;\n"
  "shrdq $51, %%rdx, %%r8          ;\n"
  "shrdq $38, %%rsi, %%rdx         ;\n"
  "shrdq $25, %%rdi, %%rsi         ;\n"
  "shrq $12, %%rdi                 ;\n"
  "andq %%rax, %%rcx               ;\n"
  "andq %%rax, %%r8                ;\n"
  "andq %%rax, %%rdx               ;\n"
  "andq %%rax, %%rsi               ;\n"
  "andq %%rax, %%rdi               ;\n"


  "movq %3, %%rax                  ;\n"
  "movq $0xfffffffffffda, %%r9     ;\n"
  "movq $0xffffffffffffe, %%r10    ;\n"
  "movq %%r10, %%r11               ;\n"
  "movq %%r10, %%r12               ;\n"
  "movq %%r10, %%r13               ;\n"
  "subq %%rcx, %%r9                ;\n"
  "subq %%r8, %%r10                ;\n"
  "subq %%rdx, %%r11               ;\n"
  "subq %%rsi, %%r12               ;\n"
  "subq %%rdi, %%r13               ;\n"
  "cmpq $1, %%rax                  ;\n"
  "cmove %%r9, %%rcx               ;\n"
  "cmove %%r10, %%r8               ;\n"
  "cmove %%r11, %%rdx              ;\n"
  "cmove %%r12, %%rsi              ;\n"
  "cmove %%r13, %%rdi              ;\n"


  "movq %%rcx, 80(%2)              ;\n"
  "movq %%r8, 88(%2)               ;\n"
  "movq %%rdx, 96(%2)              ;\n"
  "movq %%rsi, 104(%2)             ;\n"
  "movq %%rdi, 112(%2)             ;\n"
  :
  : "m"(u), "r"(&table[pos * 8]), "r"(t), "m"(sign)
  :
   "%rax", "%rcx", "%rdx", "%rdi", "%rsi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13",
   "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", "%xmm8", "%xmm9", "%xmm10", "%xmm11", "%xmm14", "%xmm14",
   "cc", "memory"
 );
}
// 102 "ed25519-donna.h" 2
// 113 "ed25519-donna.h"
// 1 "ed25519-donna-impl-base.h" 1




inline __attribute__((always_inline)) static void
ge25519_p1p1_to_partial(ge25519 *r, const ge25519_p1p1 *p) {
 curve25519_mul(r->x, p->x, p->t);
 curve25519_mul(r->y, p->y, p->z);
 curve25519_mul(r->z, p->z, p->t);
}

inline __attribute__((always_inline)) static void
ge25519_p1p1_to_full(ge25519 *r, const ge25519_p1p1 *p) {
 curve25519_mul(r->x, p->x, p->t);
 curve25519_mul(r->y, p->y, p->z);
 curve25519_mul(r->z, p->z, p->t);
 curve25519_mul(r->t, p->x, p->y);
}

static void
ge25519_full_to_pniels(ge25519_pniels *p, const ge25519 *r) {
 curve25519_sub(p->ysubx, r->y, r->x);
 curve25519_add(p->xaddy, r->y, r->x);
 curve25519_copy(p->z, r->z);
 curve25519_mul(p->t2d, r->t, ge25519_ec2d);
}





static void
ge25519_add_p1p1(ge25519_p1p1 *r, const ge25519 *p, const ge25519 *q) {
 bignum25519 a,b,c,d,t,u;

 curve25519_sub(a, p->y, p->x);
 curve25519_add(b, p->y, p->x);
 curve25519_sub(t, q->y, q->x);
 curve25519_add(u, q->y, q->x);
 curve25519_mul(a, a, t);
 curve25519_mul(b, b, u);
 curve25519_mul(c, p->t, q->t);
 curve25519_mul(c, c, ge25519_ec2d);
 curve25519_mul(d, p->z, q->z);
 curve25519_add(d, d, d);
 curve25519_sub(r->x, b, a);
 curve25519_add(r->y, b, a);
 curve25519_add_after_basic(r->z, d, c);
 curve25519_sub_after_basic(r->t, d, c);
}


static void
ge25519_double_p1p1(ge25519_p1p1 *r, const ge25519 *p) {
 bignum25519 a,b,c;

 curve25519_square(a, p->x);
 curve25519_square(b, p->y);
 curve25519_square(c, p->z);
 curve25519_add_reduce(c, c, c);
 curve25519_add(r->x, p->x, p->y);
 curve25519_square(r->x, r->x);
 curve25519_add(r->y, b, a);
 curve25519_sub(r->z, b, a);
 curve25519_sub_after_basic(r->x, r->x, r->y);
 curve25519_sub_after_basic(r->t, c, r->z);
}

static void
ge25519_nielsadd2_p1p1(ge25519_p1p1 *r, const ge25519 *p, const ge25519_niels *q, unsigned char signbit) {
 const bignum25519 *qb = (const bignum25519 *)q;
 bignum25519 *rb = (bignum25519 *)r;
 bignum25519 a,b,c;

 curve25519_sub(a, p->y, p->x);
 curve25519_add(b, p->y, p->x);
 curve25519_mul(a, a, qb[signbit]);
 curve25519_mul(r->x, b, qb[signbit^1]);
 curve25519_add(r->y, r->x, a);
 curve25519_sub(r->x, r->x, a);
 curve25519_mul(c, p->t, q->t2d);
 curve25519_add_reduce(r->t, p->z, p->z);
 curve25519_copy(r->z, r->t);
 curve25519_add(rb[2+signbit], rb[2+signbit], c);
 curve25519_sub(rb[2+(signbit^1)], rb[2+(signbit^1)], c);
}

static void
ge25519_pnielsadd_p1p1(ge25519_p1p1 *r, const ge25519 *p, const ge25519_pniels *q, unsigned char signbit) {
 const bignum25519 *qb = (const bignum25519 *)q;
 bignum25519 *rb = (bignum25519 *)r;
 bignum25519 a,b,c;

 curve25519_sub(a, p->y, p->x);
 curve25519_add(b, p->y, p->x);
 curve25519_mul(a, a, qb[signbit]);
 curve25519_mul(r->x, b, qb[signbit^1]);
 curve25519_add(r->y, r->x, a);
 curve25519_sub(r->x, r->x, a);
 curve25519_mul(c, p->t, q->t2d);
 curve25519_mul(r->t, p->z, q->z);
 curve25519_add_reduce(r->t, r->t, r->t);
 curve25519_copy(r->z, r->t);
 curve25519_add(rb[2+signbit], rb[2+signbit], c);
 curve25519_sub(rb[2+(signbit^1)], rb[2+(signbit^1)], c);
}

static void
ge25519_double_partial(ge25519 *r, const ge25519 *p) {
 ge25519_p1p1 t;
 ge25519_double_p1p1(&t, p);
 ge25519_p1p1_to_partial(r, &t);
}

static void
ge25519_double(ge25519 *r, const ge25519 *p) {
 ge25519_p1p1 t;
 ge25519_double_p1p1(&t, p);
 ge25519_p1p1_to_full(r, &t);
}

static void
ge25519_add(ge25519 *r, const ge25519 *p, const ge25519 *q) {
 ge25519_p1p1 t;
 ge25519_add_p1p1(&t, p, q);
 ge25519_p1p1_to_full(r, &t);
}

static void
ge25519_nielsadd2(ge25519 *r, const ge25519_niels *q) {
 bignum25519 a,b,c,e,f,g,h;

 curve25519_sub(a, r->y, r->x);
 curve25519_add(b, r->y, r->x);
 curve25519_mul(a, a, q->ysubx);
 curve25519_mul(e, b, q->xaddy);
 curve25519_add(h, e, a);
 curve25519_sub(e, e, a);
 curve25519_mul(c, r->t, q->t2d);
 curve25519_add(f, r->z, r->z);
 curve25519_add_after_basic(g, f, c);
 curve25519_sub_after_basic(f, f, c);
 curve25519_mul(r->x, e, f);
 curve25519_mul(r->y, h, g);
 curve25519_mul(r->z, g, f);
 curve25519_mul(r->t, e, h);
}

static void
ge25519_pnielsadd(ge25519_pniels *r, const ge25519 *p, const ge25519_pniels *q) {
 bignum25519 a,b,c,x,y,z,t;

 curve25519_sub(a, p->y, p->x);
 curve25519_add(b, p->y, p->x);
 curve25519_mul(a, a, q->ysubx);
 curve25519_mul(x, b, q->xaddy);
 curve25519_add(y, x, a);
 curve25519_sub(x, x, a);
 curve25519_mul(c, p->t, q->t2d);
 curve25519_mul(t, p->z, q->z);
 curve25519_add(t, t, t);
 curve25519_add_after_basic(z, t, c);
 curve25519_sub_after_basic(t, t, c);
 curve25519_mul(r->xaddy, x, t);
 curve25519_mul(r->ysubx, y, z);
 curve25519_mul(r->z, z, t);
 curve25519_mul(r->t2d, x, y);
 curve25519_copy(y, r->ysubx);
 curve25519_sub(r->ysubx, r->ysubx, r->xaddy);
 curve25519_add(r->xaddy, r->xaddy, y);
 curve25519_mul(r->t2d, r->t2d, ge25519_ec2d);
}






static void
ge25519_pack(unsigned char r[32], const ge25519 *p) {
 bignum25519 tx, ty, zi;
 unsigned char parity[32];
 curve25519_recip(zi, p->z);
 curve25519_mul(tx, p->x, zi);
 curve25519_mul(ty, p->y, zi);
 curve25519_contract(r, ty);
 curve25519_contract(parity, tx);
 r[31] ^= ((parity[0] & 1) << 7);
}

static int
ge25519_unpack_negative_vartime(ge25519 *r, const unsigned char p[32]) {
 static const unsigned char zero[32] = {0};
 static const bignum25519 one = {1};
 unsigned char parity = p[31] >> 7;
 unsigned char check[32];
 bignum25519 t, root, num, den, d3;

 curve25519_expand(r->y, p);
 curve25519_copy(r->z, one);
 curve25519_square(num, r->y);
 curve25519_mul(den, num, ge25519_ecd);
 curve25519_sub_reduce(num, num, r->z);
 curve25519_add(den, den, r->z);



 curve25519_square(t, den);
 curve25519_mul(d3, t, den);
 curve25519_square(r->x, d3);
 curve25519_mul(r->x, r->x, den);
 curve25519_mul(r->x, r->x, num);
 curve25519_pow_two252m3(r->x, r->x);


 curve25519_mul(r->x, r->x, d3);
 curve25519_mul(r->x, r->x, num);


 curve25519_square(t, r->x);
 curve25519_mul(t, t, den);
 curve25519_sub_reduce(root, t, num);
 curve25519_contract(check, root);
 if (!ed25519_verify(check, zero, 32)) {
  curve25519_add_reduce(t, t, num);
  curve25519_contract(check, t);
  if (!ed25519_verify(check, zero, 32))
   return 0;
  curve25519_mul(r->x, r->x, ge25519_sqrtneg1);
 }

 curve25519_contract(check, r->x);
 if ((check[0] & 1) == parity) {
  curve25519_copy(t, r->x);
  curve25519_neg(r->x, t);
 }
 curve25519_mul(r->t, r->x, r->y);
 return 1;
}
// 252 "ed25519-donna-impl-base.h"
static void
ge25519_double_scalarmult_vartime(ge25519 *r, const ge25519 *p1, const bignum256modm s1, const bignum256modm s2) {
 signed char slide1[256], slide2[256];
 ge25519_pniels pre1[(1<<(5 -2))];
 ge25519 d1;
 ge25519_p1p1 t;
 int32_t i;

 contract256_slidingwindow_modm(slide1, s1, 5);
 contract256_slidingwindow_modm(slide2, s2, 7);

 ge25519_double(&d1, p1);
 ge25519_full_to_pniels(pre1, p1);
 for (i = 0; i < (1<<(5 -2)) - 1; i++)
  ge25519_pnielsadd(&pre1[i+1], &d1, &pre1[i]);


 memset(r, 0, sizeof(ge25519));
 r->y[0] = 1;
 r->z[0] = 1;

 i = 255;
 while ((i >= 0) && !(slide1[i] | slide2[i]))
  i--;

 for (; i >= 0; i--) {
  ge25519_double_p1p1(&t, r);

  if (slide1[i]) {
   int index = abs(slide1[i]) / 2;
   ge25519_p1p1_to_full(r, &t);
   ge25519_pnielsadd_p1p1(&t, r, &pre1[index], (unsigned char)slide1[i] >> 7);
  }

  if (slide2[i]) {
   int index = abs(slide2[i]) / 2;
   ge25519_p1p1_to_full(r, &t);
   ge25519_nielsadd2_p1p1(&t, r, &ge25519_niels_sliding_multiples[index], (unsigned char)slide2[i] >> 7);
  }

  ge25519_p1p1_to_partial(r, &t);
 }
}
// 334 "ed25519-donna-impl-base.h"
static void
ge25519_scalarmult_base_niels(ge25519 *r, const uint8_t basepoint_table[256][96], const bignum256modm s) {
 signed char b[64];
 uint32_t i;
 ge25519_niels t;

 contract256_window4_modm(b, s);

 ge25519_scalarmult_base_choose_niels(&t, basepoint_table, 0, b[1]);
 curve25519_sub_reduce(r->x, t.xaddy, t.ysubx);
 curve25519_add_reduce(r->y, t.xaddy, t.ysubx);
 memset(r->z, 0, sizeof(bignum25519));
 curve25519_copy(r->t, t.t2d);
 r->z[0] = 2;
 for (i = 3; i < 64; i += 2) {
  ge25519_scalarmult_base_choose_niels(&t, basepoint_table, i / 2, b[i]);
  ge25519_nielsadd2(r, &t);
 }
 ge25519_double_partial(r, r);
 ge25519_double_partial(r, r);
 ge25519_double_partial(r, r);
 ge25519_double(r, r);
 ge25519_scalarmult_base_choose_niels(&t, basepoint_table, 0, b[0]);
 curve25519_mul(t.t2d, t.t2d, ge25519_ecd);
 ge25519_nielsadd2(r, &t);
 for(i = 2; i < 64; i += 2) {
  ge25519_scalarmult_base_choose_niels(&t, basepoint_table, i / 2, b[i]);
  ge25519_nielsadd2(r, &t);
 }
}
// 114 "ed25519-donna.h" 2
// 18 "ed25519.c" 2
// 1 "ed25519.h" 1
// 10 "ed25519.h"
typedef unsigned char ed25519_signature[64];
typedef unsigned char ed25519_public_key[32];
typedef unsigned char ed25519_secret_key[32];

typedef unsigned char curved25519_key[32];

/*
void ed25519_publickey(const ed25519_secret_key sk, ed25519_public_key pk);
int ed25519_sign_open(const unsigned char *m, size_t mlen, const ed25519_public_key pk, const ed25519_signature RS);
void ed25519_sign(const unsigned char *m, size_t mlen, const ed25519_secret_key sk, const ed25519_public_key pk, ed25519_signature RS);

int ed25519_sign_open_batch(const unsigned char **m, size_t *mlen, const unsigned char **pk, const unsigned char **RS, size_t num, int *valid);

void ed25519_randombytes_unsafe(void *out, size_t count);

void curved25519_scalarmult_basepoint(curved25519_key pk, const curved25519_key e);
*/

// 195 "ed25519-hash.h" 2

//typedef SHA512_CTX ed25519_hash_context;

typedef int ed25519_hash_context;

static void
ed25519_hash_init(ed25519_hash_context *ctx) {
// SHA512_Init(ctx);
}

static void
ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen) {
// SHA512_Update(ctx, in, inlen);
}

static void
ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash) {
// SHA512_Final(hash, ctx);
}

static void
ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen) {
// SHA512(in, inlen, hash);
}
// 21 "ed25519.c" 2





inline __attribute__((always_inline)) static void
ed25519_extsk(hash_512bits extsk, const ed25519_secret_key sk) {
 ed25519_hash(extsk, sk, 32);
 extsk[0] &= 248;
 extsk[31] &= 127;
 extsk[31] |= 64;
}

static void
ed25519_hram(hash_512bits hram, const ed25519_signature RS, const ed25519_public_key pk, const unsigned char *m, size_t mlen) {
 ed25519_hash_context ctx;
 ed25519_hash_init(&ctx);
 ed25519_hash_update(&ctx, RS, 32);
 ed25519_hash_update(&ctx, pk, 32);
 ed25519_hash_update(&ctx, m, mlen);
 ed25519_hash_final(&ctx, hram);
}
/*
void
ed25519_publickey (const ed25519_secret_key sk, ed25519_public_key pk) {
 bignum256modm a;
 ge25519 __attribute__((aligned(16))) A;
 hash_512bits extsk;


 ed25519_extsk(extsk, sk);
 expand256_modm(a, extsk, 32);
 ge25519_scalarmult_base_niels(&A, ge25519_niels_base_multiples, a);
 ge25519_pack(pk, &A);
}


void
ed25519_sign (const unsigned char *m, size_t mlen, const ed25519_secret_key sk, const ed25519_public_key pk, ed25519_signature RS) {
 ed25519_hash_context ctx;
 bignum256modm r, S, a;
 ge25519 __attribute__((aligned(16))) R;
 hash_512bits extsk, hashr, hram;

 ed25519_extsk(extsk, sk);


 ed25519_hash_init(&ctx);
 ed25519_hash_update(&ctx, extsk + 32, 32);
 ed25519_hash_update(&ctx, m, mlen);
 ed25519_hash_final(&ctx, hashr);
 expand256_modm(r, hashr, 64);


 ge25519_scalarmult_base_niels(&R, ge25519_niels_base_multiples, r);
 ge25519_pack(RS, &R);


 ed25519_hram(hram, RS, pk, m, mlen);
 expand256_modm(S, hram, 64);


 expand256_modm(a, extsk, 32);
 mul256_modm(S, S, a);


 add256_modm(S, S, r);


 contract256_modm(RS + 32, S);
}

int
ed25519_sign_open (const unsigned char *m, size_t mlen, const ed25519_public_key pk, const ed25519_signature RS) {
 ge25519 __attribute__((aligned(16))) R, A;
 hash_512bits hash;
 bignum256modm hram, S;
 unsigned char checkR[32];

 if ((RS[63] & 224) || !ge25519_unpack_negative_vartime(&A, pk))
  return -1;


 ed25519_hram(hash, RS, pk, m, mlen);
 expand256_modm(hram, hash, 64);


 expand256_modm(S, RS + 32, 32);


 ge25519_double_scalarmult_vartime(&R, &A, hram, S);
 ge25519_pack(checkR, &R);


 return ed25519_verify(RS, checkR, 32) ? 0 : -1;
}
*/
// 1 "ed25519-donna-batchverify.h" 1
// 9 "ed25519-donna-batchverify.h"
static const size_t limb128bits = (128 + 56 - 1) / 56;

typedef size_t heap_index_t;

typedef struct batch_heap_t {
 unsigned char r[((64 * 2) + 1)][16];
 ge25519 points[((64 * 2) + 1)];
 bignum256modm scalars[((64 * 2) + 1)];
 heap_index_t heap[((64 * 2) + 1)];
 size_t size;
} batch_heap;


static void
heap_swap(heap_index_t *heap, size_t a, size_t b) {
 heap_index_t temp;
 temp = heap[a];
 heap[a] = heap[b];
 heap[b] = temp;
}


static void
heap_insert_next(batch_heap *heap) {
 size_t node = heap->size, parent;
 heap_index_t *pheap = heap->heap;
 bignum256modm *scalars = heap->scalars;


 pheap[node] = (heap_index_t)node;


 parent = (node - 1) / 2;
 while (node && lt256_modm_batch(scalars[pheap[parent]], scalars[pheap[node]], 5 - 1)) {
  heap_swap(pheap, parent, node);
  node = parent;
  parent = (node - 1) / 2;
 }
 heap->size++;
}


static void
heap_updated_root(batch_heap *heap, size_t limbsize) {
 size_t node, parent, childr, childl;
 heap_index_t *pheap = heap->heap;
 bignum256modm *scalars = heap->scalars;


 parent = 0;
 node = 1;
 childl = 1;
 childr = 2;
 while ((childr < heap->size)) {
  node = lt256_modm_batch(scalars[pheap[childl]], scalars[pheap[childr]], limbsize) ? childr : childl;
  heap_swap(pheap, parent, node);
  parent = node;
  childl = (parent * 2) + 1;
  childr = childl + 1;
 }


 parent = (node - 1) / 2;
 while (node && lte256_modm_batch(scalars[pheap[parent]], scalars[pheap[node]], limbsize)) {
  heap_swap(pheap, parent, node);
  node = parent;
  parent = (node - 1) / 2;
 }
}


static void
heap_build(batch_heap *heap, size_t count) {
 heap->heap[0] = 0;
 heap->size = 0;
 while (heap->size < count)
  heap_insert_next(heap);
}


static void
heap_extend(batch_heap *heap, size_t new_count) {
 while (heap->size < new_count)
  heap_insert_next(heap);
}


static void
heap_get_top2(batch_heap *heap, heap_index_t *max1, heap_index_t *max2, size_t limbsize) {
 heap_index_t h0 = heap->heap[0], h1 = heap->heap[1], h2 = heap->heap[2];
 if (lt256_modm_batch(heap->scalars[h1], heap->scalars[h2], limbsize))
  h1 = h2;
 *max1 = h0;
 *max2 = h1;
}


static void
ge25519_multi_scalarmult_vartime_final(ge25519 *r, ge25519 *point, bignum256modm scalar) {
 const bignum256modm_element_t topbit = ((bignum256modm_element_t)1 << (56 - 1));
 size_t limb = limb128bits;
 bignum256modm_element_t flag;

 if (isone256_modm_batch(scalar)) {

  *r = *point;
  return;
 } else if (iszero256_modm_batch(scalar)) {

  memset(r, 0, sizeof(*r));
  r->y[0] = 1;
  r->z[0] = 1;
  return;
 }

 *r = *point;


 while (!scalar[limb])
  limb--;


 flag = topbit;
 while ((scalar[limb] & flag) == 0)
  flag >>= 1;


 for (;;) {
  ge25519_double(r, r);
  if (scalar[limb] & flag)
   ge25519_add(r, r, point);

  flag >>= 1;
  if (!flag) {
   if (!limb--)
    break;
   flag = topbit;
  }
 }
}


static void
ge25519_multi_scalarmult_vartime(ge25519 *r, batch_heap *heap, size_t count) {
 heap_index_t max1, max2;


 size_t limbsize = 5 - 1;


 int extended = 0;


 heap_build(heap, ((count + 1) / 2) | 1);

 for (;;) {
  heap_get_top2(heap, &max1, &max2, limbsize);


  if (iszero256_modm_batch(heap->scalars[max2]))
   break;


  if (!heap->scalars[max1][limbsize])
   limbsize -= 1;


  if (!extended && isatmost128bits256_modm_batch(heap->scalars[max1])) {
   heap_extend(heap, count);
   heap_get_top2(heap, &max1, &max2, limbsize);
   extended = 1;
  }

  sub256_modm_batch(heap->scalars[max1], heap->scalars[max1], heap->scalars[max2], limbsize);
  ge25519_add(&heap->points[max2], &heap->points[max2], &heap->points[max1]);
  heap_updated_root(heap, limbsize);
 }

 ge25519_multi_scalarmult_vartime_final(r, &heap->points[max1], heap->scalars[max1]);
}

/*
unsigned char batch_point_buffer[3][32];

static int
ge25519_is_neutral_vartime(const ge25519 *p) {
 static const unsigned char zero[32] = {0};
 unsigned char point_buffer[3][32];
 curve25519_contract(point_buffer[0], p->x);
 curve25519_contract(point_buffer[1], p->y);
 curve25519_contract(point_buffer[2], p->z);
 memcpy(batch_point_buffer[1], point_buffer[1], 32);
 return (memcmp(point_buffer[0], zero, 32) == 0) && (memcmp(point_buffer[1], point_buffer[2], 32) == 0);
}

int
ed25519_sign_open_batch (const unsigned char **m, size_t *mlen, const unsigned char **pk, const unsigned char **RS, size_t num, int *valid) {
 batch_heap __attribute__((aligned(16))) batch;
 ge25519 __attribute__((aligned(16))) p;
 bignum256modm *r_scalars;
 size_t i, batchsize;
 unsigned char hram[64];
 int ret = 0;

 for (i = 0; i < num; i++)
  valid[i] = 1;

 while (num > 3) {
  batchsize = (num > 64) ? 64 : num;


  ed25519_randombytes_unsafe (batch.r, batchsize * 16);
  r_scalars = &batch.scalars[batchsize + 1];
  for (i = 0; i < batchsize; i++)
   expand256_modm(r_scalars[i], batch.r[i], 16);


  for (i = 0; i < batchsize; i++) {
   expand256_modm(batch.scalars[i], RS[i] + 32, 32);
   mul256_modm(batch.scalars[i], batch.scalars[i], r_scalars[i]);
  }
  for (i = 1; i < batchsize; i++)
   add256_modm(batch.scalars[0], batch.scalars[0], batch.scalars[i]);


  for (i = 0; i < batchsize; i++) {
   ed25519_hram(hram, RS[i], pk[i], m[i], mlen[i]);
   expand256_modm(batch.scalars[i+1], hram, 64);
   mul256_modm(batch.scalars[i+1], batch.scalars[i+1], r_scalars[i]);
  }


  batch.points[0] = ge25519_basepoint;
  for (i = 0; i < batchsize; i++)
   if (!ge25519_unpack_negative_vartime(&batch.points[i+1], pk[i]))
    goto fallback;
  for (i = 0; i < batchsize; i++)
   if (!ge25519_unpack_negative_vartime(&batch.points[batchsize+i+1], RS[i]))
    goto fallback;

  ge25519_multi_scalarmult_vartime(&p, &batch, (batchsize * 2) + 1);
  if (!ge25519_is_neutral_vartime(&p)) {
   ret |= 2;

   fallback:
   for (i = 0; i < batchsize; i++) {
    valid[i] = ed25519_sign_open (m[i], mlen[i], pk[i], RS[i]) ? 0 : 1;
    ret |= (valid[i] ^ 1);
   }
  }

  m += batchsize;
  mlen += batchsize;
  pk += batchsize;
  RS += batchsize;
  num -= batchsize;
  valid += batchsize;
 }

 for (i = 0; i < num; i++) {
  valid[i] = ed25519_sign_open (m[i], mlen[i], pk[i], RS[i]) ? 0 : 1;
  ret |= (valid[i] ^ 1);
 }

 return ret;
}
*/
// 119 "ed25519.c" 2





/*   void   */
/*   curved25519_scalarmult_basepoint (curved25519_key pk, const curved25519_key e) {   */
/*    curved25519_key ec;   */
/*    bignum256modm s;   */
/*    bignum25519 __attribute__((aligned(16))) yplusz, zminusy;   */
/*    ge25519 __attribute__((aligned(16))) p;   */
/*    size_t i;   */


/*    for (i = 0; i < 32; i++) ec[i] = e[i];   */
/*    /\*   ec[0] &= 248;   *\/   */
/*    /\*   ec[31] &= 127;   *\/   */
/*    /\*   ec[31] |= 64;   *\/   */

/*    expand_raw256_modm(s, ec);   */


/*    ge25519_scalarmult_base_niels(&p, ge25519_niels_base_multiples, s);   */


/*    curve25519_add(yplusz, p.y, p.z);   */
/*    curve25519_sub(zminusy, p.z, p.y);   */
/*    curve25519_recip(zminusy, zminusy);   */
/*    curve25519_mul(yplusz, yplusz, zminusy);   */
/*    curve25519_contract(pk, yplusz);   */
/*   }   */



// derek: added
static int
ge25519_unpack_vartime(ge25519 *r, const unsigned char p[32]) {
 static const unsigned char zero[32] = {0};
 static const bignum25519 one = {1};
 unsigned char parity = p[31] >> 7;
 unsigned char check[32];
 bignum25519 t, root, num, den, d3;

 curve25519_expand(r->y, p);
 curve25519_copy(r->z, one);
 curve25519_square(num, r->y);
 curve25519_mul(den, num, ge25519_ecd);
 curve25519_sub_reduce(num, num, r->z);
 curve25519_add(den, den, r->z);



 curve25519_square(t, den);
 curve25519_mul(d3, t, den);
 curve25519_square(r->x, d3);
 curve25519_mul(r->x, r->x, den);
 curve25519_mul(r->x, r->x, num);
 curve25519_pow_two252m3(r->x, r->x);


 curve25519_mul(r->x, r->x, d3);
 curve25519_mul(r->x, r->x, num);


 curve25519_square(t, r->x);
 curve25519_mul(t, t, den);
 curve25519_sub_reduce(root, t, num);
 curve25519_contract(check, root);
 if (!ed25519_verify(check, zero, 32)) {
  curve25519_add_reduce(t, t, num);
  curve25519_contract(check, t);
  if (!ed25519_verify(check, zero, 32))
   return 0;
  curve25519_mul(r->x, r->x, ge25519_sqrtneg1);
 }

 curve25519_contract(check, r->x);
 if ((check[0] & 1) != parity) {
  curve25519_copy(t, r->x);
  curve25519_neg(r->x, t);
 }
 curve25519_mul(r->t, r->x, r->y);
 return 1;
}

