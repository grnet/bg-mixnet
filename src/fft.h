/*
 * fft.h
 *
 *  Created on: 13.12.2012
 *      Author: stephaniebayer
 */

#ifndef FFT_H_
#define FFT_H_

#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include <NTL/ZZ.h>
#include <NTL/mat_ZZ.h>
NTL_CLIENT

class fft {
public:
	fft();
	virtual ~fft();


	ZZ r_o_u(ZZ gen, long m, ZZ ord);
	static long to_long(vector<int>* bit_r);


	static void bitreverse(long & z, long x, long d);
	static void brevorder(vector<ZZ>* ret, vector<ZZ>* v);

	static void FFT(vector<ZZ>* fft, vector<ZZ>* v, long N, ZZ rootofunity, ZZ ord);
	static void FFT_matrix(vector<vector<ZZ>*>* fft, vector<vector<ZZ>*>* v, long N, ZZ rootofunity, ZZ ord);
	static void	FFTinv(vector<ZZ>* ret, vector<ZZ>* points, long N, ZZ rootofunity, ZZ ord);
	//static vector<Cipher_elg>* brevorder(vector<Cipher_elg>* v);
	static void fft_in(vector<ZZ>*, vector<ZZ>* v, ZZ omega, ZZ ord, ZZ mod);
	static void fft_sum_in(vector<ZZ>*, vector<ZZ>* v, ZZ omega, ZZ ord);
	static void fft_mult_cipher(vector<vector<vector<ZZ>*>*>* ret, vector<vector<Cipher_elg>* >* v, ZZ omega, ZZ ord, ZZ mod);
	static void fft_matrix(vector<vector<ZZ>*>*, vector<vector<ZZ>* >* v, ZZ omega, ZZ ord);
	static void fft_matrix_inv(vector<vector<ZZ>*>*, vector<vector<ZZ>* >* v, ZZ omega, ZZ ord);
	static void sum_t(vector<vector<ZZ>* >*, vector<vector<ZZ>* >* T, ZZ omega, ZZ ord);
	static void calc_Pk (vector<vector<ZZ>*>*, vector<vector<Cipher_elg>* >* v, vector<vector<ZZ>* >* T, ZZ omega, ZZ ord, ZZ mod, int omega_sw);
	static void  calc_m(vector<vector<ZZ>* >*, long m, ZZ omgea, ZZ ord);
};

#endif /* FFT_H_ */
