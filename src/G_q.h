/*
 * G_q.h
 *
 *  Created on: 30.09.2010
 *      Author: stephaniebayer
 *
 *      Class describing the group G_q subset of Z_p with prime order q.
 *
 */

#ifndef G_q_H_
#define G_q_H_

#include "Cyclic_group.h"
#include "Mod_p.h"
#include <NTL/ZZ.h>
NTL_CLIENT


class G_q: public Cyclic_group {
private:
	Mod_p generator;  //generator of the group
	ZZ order; //order of the group
	ZZ mod;  //value of p such that G_q subset of Z_p
public:
	//Constructors and destructor
	G_q();
	G_q(Mod_p gen, long o, long mod);
	G_q(Mod_p gen,long o, ZZ mod);
	G_q(Mod_p gen, ZZ o, ZZ mod);
	G_q(ZZ gen_val, long o, long mod);
	G_q(ZZ gen_val, long o , ZZ mod);
	G_q(ZZ gen_val, ZZ o , ZZ mod);
	G_q(long gen_val, long o, long mod);
	G_q(long gen_val, long o, ZZ mod);
	G_q(long gen_val, ZZ o, ZZ mod);
	G_q(Mod_p gen, long o);
	G_q(Mod_p gen, ZZ o);
	G_q(long o,ZZ mod);
	G_q(ZZ o,ZZ mod);
	G_q(long o, long mod);
	virtual ~G_q();

	//Access to the variables
	Mod_p get_gen() const;
	ZZ get_ord() const;
	ZZ get_mod()const;

	//operators
	void operator =(const G_q& H);
	friend ostream& operator<<(ostream& os, const G_q& G);


	//Test if an element is a generator of the group
	bool is_generator(const Mod_p& el);
	bool is_generator(const ZZ& x);
	bool is_generator(const long& x);

	//returns the identity
	Mod_p identity();

	//returns random an element
	Mod_p random_el();
	Mod_p random_el(int c);

	//creates an element with value val
	Mod_p element(ZZ val);
	Mod_p element(long val);

	//returns the inverse of an value
	Mod_p inverse(ZZ x);
	Mod_p inverse(long x);
};

#endif /* G_q_H_ */
