/*
 * multi_expo.h
 *
 *  Created on: 02.07.2012
 *      Author: stephaniebayer
 */

#ifndef MULTI_EXPO_H_
#define MULTI_EXPO_H_


#include<vector>
#include "G_q.h"
#include "FakeZZ.h"
#include "Mod_p.h"
#include "Cipher_elg.h"
#include "Permutation.h"
NTL_CLIENT


class multi_expo {
public:
	multi_expo();
	virtual ~multi_expo();


	static vector<vector<int>* >* to_binary(int win);
	static long to_long(vector<int>* bit_r);
	static void to_long(long& t, vector<int>* bit_r);
	static vector<long>* to_basis(ZZ e, long num_b, int omega);
	static vector<vector<vector<long>* >* >* to_basis_vec(vector<vector<ZZ>* >* T, long num_b, int omega);

	static void expo_mult(CurvePoint& ret, const vector<ZZ>* e, ZZ ran, int omega_expo, vector<Mod_p>* gen);
	static void expo_mult(Cipher_elg& ret, const vector<Cipher_elg>* a, vector<ZZ>* e, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<Cipher_elg>* a, ZZ f, vector<ZZ>* e, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<Cipher_elg>* a, vector<vector<long>*>* e, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<vector<Cipher_elg>*>* a, vector<ZZ>* s1, vector<ZZ>* s2, int omega );

	static vector<vector<CurvePoint>* >* calc_Yk(vector<CurvePoint>* y, int win);
	static vector<vector<Mod_p>* >* calc_Yk(vector<Mod_p>* y, int win);

	static void multi_expo_LL(Mod_p& ret, vector<Mod_p>* y, vector<ZZ>* e, int win);
	static void multi_expo_LL(CurvePoint& ret, vector<CurvePoint>* y, vector<ZZ>* e, int win);
	static void multi_expo_LL(Cipher_elg& ret, Cipher_elg c1, Cipher_elg c2, Cipher_elg c3, Cipher_elg c4 , vector<ZZ>* e, int win);

	static vector<int>* to_basis_sw(ZZ e, long num_b, int omega_sw);
	static void multi_expo_sw(CurvePoint& ret, ZZ e_1, ZZ e_2, int omega_sw,  vector<vector<CurvePoint>* >* gen_prec);
};

#endif /* MULTI_EXPO_H_ */
