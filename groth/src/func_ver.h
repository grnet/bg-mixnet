/*
 * func_ver.h
 *
 *  Created on: 04.07.2012
 *      Author: stephaniebayer
 */

#ifndef FUNC_VER_H_
#define FUNC_VER_H_

#include "Functions.h"
#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Pedersen.h"

#include "FakeZZ.h"
NTL_CLIENT


class func_ver {
public:
	func_ver();
	virtual ~func_ver();

	static void check_Dh_op(vector<Mod_p>* c_Dh, vector<ZZ>* e, vector<ZZ>* F, ZZ Z, long omega, Pedersen& ped, bool& b);
	static void check_D_op(Mod_p c_D0, Mod_p c_z, vector<Mod_p>* c_A, vector<Mod_p>* c_B, vector<ZZ>* chal_1, ZZ chal_2, vector<ZZ>* A_bar, ZZ r_A_bar, long n, Pedersen& ped, bool& b);
	static void check_Ds_op(vector<Mod_p>* c_Ds, vector<Mod_p>* c_Dh, Mod_p c_Dm, vector<ZZ>* chal_1, vector<ZZ>* chal_2, vector<ZZ>* Ds_bar, ZZ r_Ds_bar, Pedersen& ped, bool& b);
	static void check_Dl_op(vector<Mod_p>* c_Dl, vector<ZZ>* chal_1, vector<ZZ>* A_bar, vector<ZZ>* Ds_bar, vector<ZZ>*  chal_2, ZZ r_Dl_bar, Pedersen& ped, bool& b);
	static void check_d_op(vector<Mod_p>* c_Dh, Mod_p c_d, vector<ZZ>* chal, vector<ZZ>* d_bar, ZZ r_d_bar, Pedersen& ped, bool& b);
	static void check_Delta_op(Mod_p c_dh, Mod_p c_Delta, vector<ZZ>* chal, vector<ZZ>* Delta_bar, vector<ZZ>* d_bar, ZZ r_Delta_bar, ZZ chal_1, ZZ chal_2, ZZ chal_3, Pedersen& ped, bool& b);

	static void fill_vector(vector<ZZ>* t);
	static void fill_vector(vector<ZZ>* t, ZZ& challenge);
	static void fill_x8(vector<ZZ>* chal_x8, vector<vector<long>* >* basis_chal_x8, vector<ZZ>* mul_chal_x8, long omega, ZZ& challenge);
};

#endif /* FUNC_VER_H_ */
