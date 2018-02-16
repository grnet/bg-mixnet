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
#include <NTL/ZZ.h>
NTL_CLIENT

#include "Mod_p.h"

class Pedersen {
private:
	G_q G; //Group  G_q with order o used for the commitment
	vector<Mod_p>* gen; //vector of generators, public key of the commitment
	vector<vector<ZZ>* >* gen_prec;//vector containing the precomputated values of the first two generator for SW algorithm

	int omega_expo; //window size used for multi-expo.
	int omega_ll; //window size used for multi-expo LL.
	int omega_sw; //window size used for sliding window
public:
	//constructors and destructor
	Pedersen();
	Pedersen(long n, G_q H);
	Pedersen(Mod_p gen, long o, long mod, long n);
	Pedersen(Mod_p gen, long o, ZZ mod, long n);
	Pedersen(Mod_p gen, ZZ o, ZZ mod, long n);
	Pedersen(long gen_val, long o, long mod, long n);
	Pedersen(long gen_val, long o, ZZ mod, long n);
	Pedersen(long gen_val, ZZ o, ZZ mod, long n);
	Pedersen(ZZ gen_val, long o, long mod, long n);
	Pedersen(ZZ gen_val, long o, ZZ mod, long n);
	Pedersen(ZZ gen_val, ZZ o, ZZ mod, long n);
	Pedersen(Mod_p gen, long o, long n);
	Pedersen(Mod_p gen,  ZZ o, long n);
	Pedersen(long o, long mod, long n);
	Pedersen(long o, ZZ mod, long n);
	Pedersen(ZZ o, ZZ mod, long n);

	virtual ~Pedersen();

	//functions to access the variables of the class
	G_q get_group() const;
	long get_length()const;

	void set_omega(int o1, int o2, int o3);
	int get_omega() const;
	vector<Mod_p>* get_gen() const;

	//Different function to calculate the commitments
	Mod_p commit(const vector<ZZ>* t, ZZ ran);
	Mod_p commit(const vector<Mod_p>* t, ZZ ran);
	Mod_p commit(const vector<Mod_p>*  t);
	Mod_p commit(ZZ t, ZZ ran);
	Mod_p commit(Mod_p t, ZZ ran);
	Mod_p commit(Mod_p t,  long ran);
	Mod_p commit(Mod_p t);


	Mod_p commit_opt(const vector<ZZ>* t, ZZ ran);
	Mod_p commit_sw(ZZ t, ZZ ran);
	Mod_p commit_sw(Mod_p t, ZZ ran);
	Mod_p commit_sw(Mod_p t,  long ran);
	Mod_p commit_sw(Mod_p t);

	//Multi expo functions
	vector<vector<ZZ>* >* precomp(ZZ g, ZZ h);
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
