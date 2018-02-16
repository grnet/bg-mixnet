/*
 * Verifier_me.h
 *
 *  Created on: 16.02.2011
 *      Author: stephaniebayer
 */

#ifndef VERIFIER_ME_H_
#define VERIFIER_ME_H_

#include <vector>
#include <map>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Pedersen.h"

#include <NTL/ZZ.h>
NTL_CLIENT

class Verifier_me {
private:
	long n,m; // User input, defines the dimensions of the matrix used in the protocol, m rows, n columns
	long omega; //window size for multi-exponentiation technique
	long omega_sw; //window size for multi-exponentiation technique sliding window
	long omega_LL; //window size for multi-exponentiation technique of Lim and Lee


	vector<Mod_p>* c_A; //Commitments to the rows of A send from the prover in round 1
	Mod_p c_D0; //commitment to the 0-th row in D
	vector<Mod_p>* c_B; //Commitments to the rows of permuted exponents B send from the prover in round 3

	ZZ chal_x2; //Challenges for round 2, exponents for permutation
	ZZ chal_y4; // Challenges to prove the use of the permutation in the prove of ciphertexts
	ZZ chal_z4; // Challenges to create the polynomial prod(y_ij -z4), created in round 4
	vector<ZZ>* chal_x6; // Vector of Vandermonde challenges x6, x6^2, ..., x6^m, generated in round 6
	vector<ZZ>* chal_y6; // Vector of Vandermonde challenges y6, y6^2, ..., y6^m, generated in round 6
	vector<ZZ> * chal_x8; // Vector of Vandermode challenges x8, x8^2 ... in round 8
	vector<vector<long>* >* basis_chal_x8; //Vector of basis_vec for multi-expo
	vector<ZZ>* mul_chal_x8; //Vector of basis_vec for multi-expo

	Mod_p c_z; //Commitments to the vector containing z

	//Commitments vectors from round 5
	vector<Mod_p>* c_Dh; // commitments to D_h;
	vector<Mod_p>* c_Ds;//contains commitment to D_s
	Mod_p c_Dm;  //  commitment to last row in D_s;
	vector<Mod_p>* c_Dl; // commitments to the values D_l
	Mod_p c_B0; //Vector containing commitments to B_0i
	vector<Mod_p>* c_a; //Vector containing the commitments to values used for the reencryption in 5
	Mod_p c_d; //commitment to vector d
	Mod_p c_Delta; //commitment to vector Delta
	Mod_p c_dh; // commitment to vector d_h

	vector<Cipher_elg>* E; //reencrypted product of the diogonals of the matrix of ciphertexts
	vector<ZZ>* D_h_bar;//Sum over the row in D_h multiplied by chal^i
	ZZ r_Dh_bar;// sum over the random elements used for commiments to D_h

	vector<ZZ>* d_bar; // chal_x8*D_h(m-1) +d
	vector<ZZ>* Delta_bar;//chal_x8*d_h+Delta
	ZZ r_d_bar; //chal_x8*r_Dh(m-1)+r_d
	ZZ r_Delta_bar; //chal_x8*r_dh +r_Delta


	vector<ZZ>* B_bar; // sum over the rows in B multiplied by chal^i
	ZZ a_bar; //sum over the elements in a times chal^i
	ZZ r_B_bar; //sum over the random elements used for B
	ZZ r_a_bar; // sum over random elements used for a
	ZZ rho_bar; //sum over random elements rho_a

	vector<ZZ>* A_bar; //sum over the row in A times the challenges
	vector<ZZ>* Ds_bar; //sum over the rows in Ds_bar times the challenges

	ZZ r_A_bar; //sum over the random elements in r_A times the challenges
	ZZ r_Ds_bar; //sum over the random elements in r_Ds times the challenges
	ZZ r_Dl_bar; //sum over the random elements in r_Dl times the challenges

public:
	Verifier_me();
	Verifier_me(map<string, long> num);
	virtual ~Verifier_me();

	//Stores the commitments to matrix Y and sends challenges vector s_1 and s_2 to the prover
	string round_2(string name);
	//round_4 stores the in output a of round_1 and outputs the challenges for round 3
	string round_4(string name);
	//round_6 outputs the challenge t, t^2,..
	string round_6(string name);
	//round 6 stores the output com of round 1 and outputs the challenges for round 7
	string round_8(string name);
	//round 8 stores the input and checks the first set of equation, if all is true return challenges e, else -1
	ZZ round_10(string name, vector<vector<Cipher_elg>* >* e,vector<vector<Cipher_elg>* >* E);
	ZZ round_12(string name);

};

#endif /* VERIFIER_ME_H_ */
