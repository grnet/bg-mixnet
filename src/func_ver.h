/*
 * func_ver.h
 *
 *  Created on: 04.07.2012
 *      Author: stephaniebayer
 */

#ifndef FUNC_VER_H_
#define FUNC_VER_H_

#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Pedersen.h"

#include <NTL/ZZ.h>
NTL_CLIENT

/* For SHA3-256 hash function */
extern "C" {
#include "KeccakHash.h"
}

class func_ver {
public:
	func_ver();
	virtual ~func_ver();


	static int check_Dh(vector<Mod_p>* c_Dh, vector<ZZ>* chal, vector<ZZ>* D_h_bar, ZZ r_Dh_bar);
	static int check_D(Mod_p c_D0, Mod_p c_z, vector<Mod_p>* c_A, vector<Mod_p>* c_B, vector<ZZ>* chal_1, ZZ chal_2, vector<ZZ>* A_bar, ZZ r_A_bar, long n);
	static int check_Ds(vector<Mod_p>* c_Ds, vector<Mod_p>* c_Dh, Mod_p c_Dm, vector<ZZ>* chal_1, vector<ZZ>* chal_2, vector<ZZ>* Ds_bar, ZZ r_Ds_bar);
	static int check_Dl(vector<Mod_p>* c_Dl, vector<ZZ>* chal_1, vector<ZZ>* A_bar, vector<ZZ>* Ds_bar, vector<ZZ>*  chal_2, ZZ r_Dl_bar);
	static int check_d(vector<Mod_p>* c_Dh, Mod_p c_d, vector<ZZ>* chal, vector<ZZ>* d_bar, ZZ r_d_bar);
	static int check_Delta(Mod_p c_dh, Mod_p c_Delta, vector<ZZ>* chal, vector<ZZ>* Delta_bar, vector<ZZ>* d_bar, ZZ r_Delta_bar, ZZ chal_1, ZZ chal_2, ZZ chal_3);
	static int check_B(vector<Mod_p>* c_B, Mod_p c_B0, vector<ZZ>* chal, vector<ZZ>* B_bar, ZZ r_B_bar);
	static int check_a(vector<Mod_p>* c_a, vector<Mod_p>* c_Dl, vector<ZZ>* chal, ZZ a_bar, ZZ r_a_bar);
	static int check_c(vector<vector<Cipher_elg>* >* enc, vector<Cipher_elg>* E, ZZ chal);
	static int check_E(vector<vector<Cipher_elg>* >* C, vector<Cipher_elg>* E, vector<ZZ>* chal, vector<ZZ>* B_bar, ZZ r_B_bar, ZZ rho_bar);


	static int check_W_op(vector<Mod_p>* a_W, vector<ZZ>* e, vector<ZZ>* F, ZZ Z, long omega);
	static int check_Dh_op(vector<Mod_p>* c_Dh, vector<ZZ>* e, vector<ZZ>* F, ZZ Z, long omega);
	static int check_D_op(Mod_p c_D0, Mod_p c_z, vector<Mod_p>* c_A, vector<Mod_p>* c_B, vector<ZZ>* chal_1, ZZ chal_2, vector<ZZ>* A_bar, ZZ r_A_bar, long n);
	static int check_Ds_op(vector<Mod_p>* c_Ds, vector<Mod_p>* c_Dh, Mod_p c_Dm, vector<ZZ>* chal_1, vector<ZZ>* chal_2, vector<ZZ>* Ds_bar, ZZ r_Ds_bar);
	static int check_Dl_op(vector<Mod_p>* c_Dl, vector<ZZ>* chal_1, vector<ZZ>* A_bar, vector<ZZ>* Ds_bar, vector<ZZ>*  chal_2, ZZ r_Dl_bar);
	static int check_d_op(vector<Mod_p>* c_Dh, Mod_p c_d, vector<ZZ>* chal, vector<ZZ>* d_bar, ZZ r_d_bar);
	static int check_Delta_op(Mod_p c_dh, Mod_p c_Delta, vector<ZZ>* chal, vector<ZZ>* Delta_bar, vector<ZZ>* d_bar, ZZ r_Delta_bar, ZZ chal_1, ZZ chal_2, ZZ chal_3);
	static int check_B_op(vector<Mod_p>* c_B, Mod_p c_B0, vector<ZZ>* chal, vector<ZZ>* B_bar, ZZ r_B_bar, long omega);
	static int check_a_op(vector<Mod_p>* c_a, vector<Mod_p>* c_Dl, vector<ZZ>* chal, ZZ a_bar, ZZ r_a_bar);
	static int check_c_op(vector<vector<Cipher_elg>* >* enc, vector<Cipher_elg>* E, ZZ chal, long omega);
	static int check_E_op(vector<vector<Cipher_elg>* >* C, vector<Cipher_elg>* E, vector<ZZ>* chal, vector<ZZ>* B_bar, ZZ r_B_bar, ZZ rho_bar, long omega);


	static void fill_vector(vector<ZZ>* t);
	static void fill_x8(vector<ZZ>* chal_x8, vector<vector<long>* >* basis_chal_x8, vector<ZZ>* mul_chal_x8, long omega);
	static void fill_e(vector<ZZ>* e, vector<vector<long>* >* basis_e, vector<ZZ>* mul_e, long omega);

	static void hash_fill_x8(ZZ chal_x6, vector<Mod_p>* c_a, //no opt: c_Dl 
					vector<Cipher_elg>* E, // no opt: NULL
					vector<ZZ>* chal_x8,
					vector<vector<long>* >* basis_chal_x8, 
					vector<ZZ>* mul_chal_x8, long omega);
	static void hash_fill_chals(ZZ chal_y4, ZZ chal_x6, 
							vector<ZZ>* chal_y6);
	static void hash_fill_commits_cipher(ZZ chal_in, 
				vector<Mod_p>* com1, 
				vector<Mod_p>* com2, 
				vector<Cipher_elg>* ct,
				vector<ZZ>* chal_out);
	static ZZ hash_chal_x2_c_B(ZZ chal_x2, vector<Mod_p>* c_B);
	static ZZ hash_chal_z4(ZZ chal_z4);
	static ZZ hash_cipher_Pedersen_ElGammal(vector<vector<Cipher_elg>* >* c,
					vector<vector<Cipher_elg>* >* C,
					long n, long omega, long omega_LL,
                                  	long omega_sw, long m, long N,
                                  	vector<Mod_p> *c_A);
	static ZZ hash_keccak_SHA3_256(string input);
	static string stringify_commitment(vector<Mod_p>* com);
	static string stringify_ciphertext(vector<vector<Cipher_elg>* >* ct);
	static int ReadHexIntoBS(string szz, BitSequence *A, int Length);
	static void printBstr(BitSequence *Squeezed, int Length, 
				const string prologue, string &hashString);
};

#endif /* FUNC_VER_H_ */
