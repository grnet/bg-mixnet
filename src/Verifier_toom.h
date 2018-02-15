/*
 * Verifier_toom.h
 *
 *  Created on: 25.04.2011
 *      Author: stephaniebayer
 */

#ifndef VERIFIER_TOOM_H_
#define VERIFIER_TOOM_H_

#include "Functions.h"
#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Pedersen.h"
#include "ElGammal.h"
#include "FakeZZ.h"
NTL_CLIENT


class Verifier_toom {
private:
	long& m_r;
	long n,m; // User input, defines the dimensions of the matrix used in the protocol, m rows, n columns
	long omega; //window size for multi-exponentiation technique
	long omega_sw; //window size for multi-exponentiation technique sliding window and LL
	long omega_LL; //window size for multi-exponentiation technique of LL

	vector<Mod_p>* c_A; //Commitments to the rows of A send from the prover in round 1
	Mod_p c_D0; //commitment to the 0-th row in D
	vector<Mod_p>* c_B; //Commitments to the rows of permuted exponents B send from the prover in round 3
	vector<Mod_p>* c_B_small; //commitments after reduction with challenges x
	vector<vector<Cipher_elg>* >* C_small; //smaller matrix of ciphertexts constructed for interaction


	ZZ chal_x2; //Challenges for round 2, exponents for permutation
	ZZ chal_y4; // Challenges to prove the use of the permutation in the prove of ciphertexts
	ZZ chal_z4; // Challenges to create the polynomial prod(y_ij -z4), created in round 4
	vector<ZZ>* chal_x6; // Vector of Vandermonde challenges x6, x6^2, ..., x6^m, generated in round 6
	vector<ZZ>* chal_y6; // Vector of Vandermonde challenges y6, y6^2, ..., y6^m, generated in round 6
	vector<ZZ> * chal_x8; // Vector of Vandermode challenges x8, x8^2 ... in round 8
	vector<vector<long>* >* basis_chal_x8; //Vector of basis_vec for multi-expo
	vector<ZZ>* mul_chal_x8; //Vector of basis_vec for multi-expo
	vector<ZZ>* x; //Vector of challenges for reduction

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
	vector<Mod_p>* c_a_c; //vector containing values used to reencrypt the E_c

	vector<Cipher_elg>* E; //reencrypted product of the diogonals of the matrix of ciphertexts
	vector<Cipher_elg>* C_c; //Ciphertexts to prove correctness of reduction

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


	ZZ a_c_bar; //sum over elements to reencrypt E_low_up
	ZZ r_ac_bar; // sum over random elements
/*	ZZ F_c; //sum over elements to reencrypt E_low_up
	ZZ Z_c; // sum over random elements
	ZZ zeta_c; //sum over random elements rho_c*/

public:
	Verifier_toom(long& mr, bool do_process);
	Verifier_toom(vector<long> num, int m, int n, long& mr, bool do_process, ElGammal* elgammal);
	virtual ~Verifier_toom();
	
	void set_public_vector(istringstream& f, long n, int o1, int o2, int o3);


	//Stores the commitments to matrix Y and sends challenges vector s_1 and s_2 to the prover
	string round_2(const string& name, ZZ* challenge, ZZ* random_out);
	string round_2(const string& name, ZZ& challenge, ZZ& random_in);
	
	//round_4 stores the in output a of round_1 and outputs the challenges for round 3
	string round_4(const string& name, ZZ* challenge, ZZ* random_out);
	string round_4(const string& name, ZZ& challenge, ZZ& random_in);
	
	//round_6 outputs the challenge t, t^2,..
	string round_6(const string& input, ZZ* challenge, ZZ* random_out);
	string round_6(const string& input, ZZ& challenge, ZZ& random_in);
	
	//first round for reductions, outputs challenges and
	string round_6_red(const string& name, vector<vector<Cipher_elg>* >* enc, ZZ* challenge, ZZ* random_out);
	string round_6_red(const string& name, vector<vector<Cipher_elg>* >* enc, ZZ& challenge, ZZ& random_in);
	
	//round_6_red_2 second round of reduction outputs the challenge t, t^2,..
	string round_6_red1(const string& input, ZZ* challenge, ZZ* random_out);
	string round_6_red1(const string& input, ZZ& challenge, ZZ& random_in);
	
	//round 6 stores the output com of round 1 and outputs the challenges for round 7
	string round_8(const string& input, ZZ* challenge, ZZ* random_out);
	string round_8(const string& input, ZZ& challenge, ZZ& random_in);

	//round 8 stores the input and checks the first set of equation, if all is true return challenges e, else -1
	bool round_10(const string& name, vector<vector<Cipher_elg>* >* e,vector<vector<Cipher_elg>* >* E);
	bool round_10_red(const string& name, vector<vector<Cipher_elg>* >* e,vector<vector<Cipher_elg>* >* E);

	void calculate_c(Cipher_elg& c, vector<vector<Cipher_elg>* >* enc);
	void calculate_ac(Mod_p& com);
	void reduce_c_B();
	void calculate_C(Cipher_elg& C, vector<Cipher_elg>* E_c, vector<ZZ>* x);

	void check_B(bool& b);
	void check_B_red(bool& b);
	void check_a(bool& b);
	void check_c(vector<vector<Cipher_elg>* >* enc, bool& b);
	void check_c_red(bool& b);
	void check_E(vector<vector<Cipher_elg>* >* E, bool& b);
	void check_E_red(vector<vector<Cipher_elg>* >* E, bool& b);
	void check_ac(bool& b);
	
private:
	ZZ make_challenge(ZZ* random) const;
	bool check_challenge(ZZ& challenge, ZZ& random) const;
	ZZ derive_from_challenge(ZZ& challenge, string id);
	
	string round_2(const string& input, ZZ& challenge);
	string round_4(const string& input, ZZ& challenge);
	string round_6(const string& input, ZZ& challenge);
	string round_6_red(const string& name, vector<vector<Cipher_elg>* >* enc, ZZ& challenge);
	string round_6_red1(const string& name, ZZ& challenge);

	string round_8(const string& name, ZZ& challenge);	
	bool do_process_;
	ElGammal* elgammal_;
	Pedersen ped_;
};

#endif /* VERIFIER_TOOM_H_ */
