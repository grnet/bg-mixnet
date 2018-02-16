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
#include <NTL/ZZ.h>
#include "Mod_p.h"
#include "Cipher_elg.h"
#include "Permutation.h"
#include <NTL/ZZ.h>
NTL_CLIENT


class multi_expo {
public:
	multi_expo();
	virtual ~multi_expo();


	static vector<vector<int>* >* to_binary(int win);
	static long to_long(vector<int>* bit_r);
	static void to_long(long& t, vector<int>* bit_r);
	static vector<long>* to_basis(ZZ e, long num_b, int omega);
//	vector<long>* to_basis(ZZ e, long num_b, int omega_expo);
	static vector<vector<vector<long>* >* >* to_basis_vec(vector<vector<ZZ>* >* T, long num_b, int omega);


	static ZZ expo_mult(const vector<ZZ>* e, ZZ ran, int omega_expo, vector<Mod_p>* gen);
	static void expo_mult(ZZ& ret, const vector<ZZ>* e, ZZ ran, int omega_expo, vector<Mod_p>* gen);
	static Cipher_elg expo_mult(const vector<Cipher_elg>* a, vector<ZZ>* e, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<Cipher_elg>* a, vector<ZZ>* e, int omega );
	static ZZ expo_mult(const vector<ZZ>* a, vector<vector<ZZ>*>* e, int omega, long pos );
	static void expo_mult(ZZ& ret, const vector<ZZ>* a, vector<vector<ZZ>*>* e, int omega, long pos );
	static ZZ expo_mult(const vector<vector<vector<ZZ>* >*>* a, vector<vector<ZZ>*>* e, int omega, long pos, long pos_2 );
	static void expo_mult(ZZ& ret, const vector<vector<vector<ZZ>* >*>* a, vector<vector<ZZ>*>* e, int omega, long pos, long pos_2 );
	static Cipher_elg expo_mult(const vector<Cipher_elg>* a, ZZ f, vector<ZZ>* e, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<Cipher_elg>* a, ZZ f, vector<ZZ>* e, int omega );
	static Cipher_elg expo_mult(const vector<Cipher_elg>* a, vector<vector<long>*>* e, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<Cipher_elg>* a, vector<vector<long>*>* e, int omega );
	static Cipher_elg expo_mult(const vector<vector<Cipher_elg>*>* a, vector<ZZ>* s1, vector<ZZ>* s2, int omega );
	static void expo_mult(Cipher_elg& ret, const vector<vector<Cipher_elg>*>* a, vector<ZZ>* s1, vector<ZZ>* s2, int omega );
	static Mod_p expo_mult(const vector<Mod_p>* a, vector<vector<long>*>* e, int omega );
	static void expo_mult(Mod_p& ret, const vector<Mod_p>* a, vector<vector<long>*>* e, int omega );


	static vector<vector<ZZ>* >* calc_Yk(vector<ZZ>* y, int win);
	static vector<vector<ZZ>* >* calc_Yk(vector<vector<vector<ZZ>*>*>* y, int win, long pos, long pos_2);
	static vector<vector<Mod_p>* >* calc_Yk(vector<Mod_p>* y, int win);
	static ZZ multi_expo_LL(vector<ZZ>* y, vector<vector<ZZ>*>* e, int win, long pos);
	static void multi_expo_LL(ZZ& ret, vector<ZZ>* y, vector<vector<ZZ>*>* e, int win, long pos);
	static ZZ multi_expo_LL(vector<vector<vector<ZZ>*>*>* y, vector<vector<ZZ>*>* e, int win, long pos, long pos_2);
	static void multi_expo_LL(ZZ& ret,vector<vector<vector<ZZ>*>*>* y, vector<vector<ZZ>*>* e, int win, long pos, long pos_2);
	static Mod_p multi_expo_LL(vector<Mod_p>* y, vector<ZZ>* e, int win);
	static void multi_expo_LL(Mod_p& ret, vector<Mod_p>* y, vector<ZZ>* e, int win);
	static void multi_expo_LL(ZZ& ret, vector<ZZ>* y, vector<ZZ>* e, int win);
	static void multi_expo_LL(Cipher_elg& ret, Cipher_elg c1, Cipher_elg c2, Cipher_elg c3, Cipher_elg c4 , vector<ZZ>* e, int win);


	static vector<int>* to_basis_sw(ZZ e, long num_b, int omega_sw);
	static ZZ multi_expo_sw(ZZ e_1, ZZ e_2, int omega_sw, vector<vector<ZZ>* >* gen_prec);
	static void multi_expo_sw(ZZ& ret, ZZ e_1, ZZ e_2, int omega_sw,  vector<vector<ZZ>* >* gen_prec);
};

#endif /* MULTI_EXPO_H_ */
