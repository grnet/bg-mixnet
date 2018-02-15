/*
 * Pedersen.h
 *
 *  Created on: 04.10.2010
 *      Author: stephaniebayer
 *
 * The class contains the public information to commit to vectors or a single value and functions to do this.
 *
 */

#ifndef PEDERSEN_H_
#define PEDERSEN_H_
#include<vector>
#include "G_q.h"
#include "FakeZZ.h"
NTL_CLIENT

#include "Mod_p.h"

class Pedersen {
private:
	//G_q G; //Group  G_q with order o used for the commitment
	vector<Mod_p>* gen; //vector of generators, public key of the commitment
	vector<vector<CurvePoint>* >* gen_prec;//vector containing the precomputated values of the first two generator for SW algorithm

	int omega_expo; //window size used for multi-expo.
	int omega_ll; //window size used for multi-expo LL.
	int omega_sw; //window size used for sliding window
public:
	//constructors and destructor

	void print_commitment(long i) const;
	Pedersen();
	Pedersen(long n);
	string get_public_vector() const;
	void set_public_vector(istringstream& f, long n, int o1, int o2, int o3);
	virtual ~Pedersen();

	//functions to access the variables of the class
	G_q get_group() const;
	long get_length()const;

	void set_omega(int o1, int o2, int o3);
	int get_omega() const;
	vector<Mod_p>* get_gen() const;

	//Different function to calculate the commitments
	Mod_p commit(ZZ t, ZZ ran);

	Mod_p commit_opt(const vector<ZZ>* t, ZZ ran);
	Mod_p commit_sw(ZZ t, ZZ ran);

	//Multi expo functions
	vector<vector<CurvePoint>* >* precomp(CurvePoint g, CurvePoint h);
	long to_long(vector<int>* bit_r);
	void to_long(long& t, vector<int>* bit_r);
	vector<long>* to_basis(ZZ e, long num_b);
	ZZ expo_mult(const vector<ZZ>* e, ZZ ran);
	void expo_mult(ZZ& ret, const vector<ZZ>* e, ZZ ran);
	vector<int>* to_basis_sw(ZZ e, long num_b);
	ZZ multi_expo_sw(ZZ e_1, ZZ e_2);
	void multi_expo_sw(ZZ& ret, ZZ e_1, ZZ e_2);


	void operator =(const Pedersen& el);
};

#endif /* PEDERSEN_H_ */
