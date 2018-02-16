/*
 * func_pro.h
 *
 *  Created on: 05.07.2012
 *      Author: stephaniebayer
 */

#ifndef FUNC_PRO_H_
#define FUNC_PRO_H_

#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Pedersen.h"

#include <NTL/ZZ.h>
NTL_CLIENT


class func_pro {
public:
	func_pro();
	virtual ~func_pro();

	//Help functions to generate X and Y
	static void set_X(vector<vector<ZZ>*>* X, long m,  long n);
	static void set_A(vector<vector<ZZ>*>* Y,  vector<vector<vector<long>* >* >* pi, long m, long n);

	static void set_x2(vector<vector<ZZ>*>* chal_x2, ZZ x2, long m, long n);
	static void set_B_op(vector<vector<ZZ>*>* B, vector<vector<vector<long>* >* >* basis_B, vector<vector<ZZ>*>* chal_x2, vector<vector<vector<long>* >*>* pi, long omega_mulex);
	static void set_B(vector<vector<ZZ>*>* B,vector<vector<ZZ>*>* chal_x2, vector<vector<vector<long>* >*>* pi);

	static void set_D(vector<vector<ZZ>*>* D, vector<vector<ZZ>*>* A, vector<vector<ZZ>*>* B, ZZ chal_z, ZZ chal_y);
	static void set_D_h(vector<vector<ZZ>*>* D_h, vector<vector<ZZ>*>* D);
	static void commit_B0_op(vector<ZZ>* B_0, vector<vector<long>* >* basis_B0, ZZ &r_B0, Mod_p &c_B0, long omega_mulex );
	static void commit_B0(vector<ZZ>* B_0, ZZ &r_B0, Mod_p &c_B0);

	static void set_Rb(vector<vector<ZZ>*>* B, vector<vector<ZZ>*>* R, ZZ &R_b);
	static void commit_a_op(vector<ZZ>* a, vector<ZZ>* r_a, vector<Mod_p>* c_a);
	static void commit_a(vector<ZZ>* a, vector<ZZ>* r_a, vector<Mod_p>* c_a);

	static void set_D_s(vector<vector<ZZ>*>* D_s, vector<vector<ZZ>*>* D_h, vector<vector<ZZ>*>* D, vector<ZZ>* chal, ZZ & r_Dl_bar);
	static void commit_Dl_op(vector<Mod_p>* c_Dl, vector<ZZ>* Dl, vector<ZZ>* r_Dl, vector<vector<ZZ>*>* D, vector<vector<ZZ>*>* D_s, vector<ZZ>* chal);
	static void commit_d_op(vector<ZZ>* d, ZZ &r_d, Mod_p & c_d);
	static void commit_Delta_op(vector<ZZ>* Delta, vector<ZZ>* d, ZZ & r_Delta, Mod_p & c_Delta);
	static void commit_d_h_op(vector<vector<ZZ>*>* D_h, vector<ZZ>* d_h, vector<ZZ>* d, vector<ZZ>* Delta, ZZ & r_d_h, Mod_p &c_d_h);
	static void commit_Dl(vector<Mod_p>* c_Dl, vector<ZZ>* Dl, vector<ZZ>* r_Dl, vector<vector<ZZ>*>* D, vector<vector<ZZ>*>* D_s, vector<ZZ>* chal, ZZ rou);
	static void commit_d(vector<ZZ>* d, ZZ &r_d, Mod_p & c_d);
	static void commit_Delta(vector<ZZ>* Delta,vector<ZZ>* d, ZZ & r_Delta, Mod_p & c_Delta);
	static void commit_d_h(vector<vector<ZZ>*>* D_h, vector<ZZ>* d_h, vector<ZZ>* d, vector<ZZ>* Delta, ZZ & r_d_h, Mod_p &c_d_h);

	static void calculate_B_bar(vector<ZZ>* T_0, vector<vector<ZZ>*>* T, vector<ZZ>* chal, vector<ZZ>* B_bar);
	static void calculate_r_B_bar(vector<ZZ>* r_T, vector<ZZ>* chal, ZZ r_T0, ZZ &r_B_bar);
	static void calculate_a_bar(vector<ZZ>* a, vector<ZZ>* chal, ZZ & a_bar);
	static void calculate_r_a_bar(vector<ZZ>* r_a, vector<ZZ>* chal, ZZ & r_a_bar);
	static void calculate_rho_a_bar(vector<ZZ>* rho_a, vector<ZZ>* chal, ZZ & rho_bar);

	static void calculate_D_h_bar(vector<ZZ>* D_h_bar, vector<vector<ZZ>*>* D_h, vector<ZZ>* chal);
	static void calculate_r_Dh_bar(vector<ZZ>* r_D_h, vector<ZZ>* chal, ZZ & r_Dh_bar);
	static void calculate_dbar_rdbar(vector<vector<ZZ>*>* D_h, vector<ZZ>* chal, vector<ZZ>* d_bar, vector<ZZ>* d, vector<ZZ>* r_D_h, ZZ r_d, ZZ & r_d_bar);
	static void calculate_Deltabar_rDeltabar(vector<ZZ>* d_h, vector<ZZ>* chal, vector<ZZ>* Delta_bar, vector<ZZ>* Delta, ZZ r_d_h, ZZ r_Delta, ZZ & r_Delta_bar);

	static void calculate_A_bar(vector<vector<ZZ>*>* D, vector<ZZ>* A_bar, vector<ZZ>* chal);
	static void calculate_D_s_bar(vector<vector<ZZ>*>* D_s, vector<ZZ>* D_s_bar, vector<ZZ>* chal);
	static void calculate_r_A_bar(ZZ r_Y0, vector<ZZ>* r_Y, vector<ZZ>* r_T, vector<ZZ>* chal, ZZ r_z, ZZ lamda, ZZ & r_A_bar);
	static void calculate_r_Ds_bar(vector<ZZ>* r_D_h,  vector<ZZ>* chal_1, vector<ZZ>* chal_2, ZZ & r_Ds_bar, ZZ r_Dm);
	static void calculate_r_Dl_bar(vector<ZZ>* r_C, vector<ZZ>* chal, ZZ &r_Dl_bar);

};

#endif /* FUNC_PRO_H_ */
