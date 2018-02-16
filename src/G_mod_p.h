
/*
 * G_mod_p.h
 *
 *  Created on: 20.09.2010
 *      Author: stephaniebayer
 *
 *      Multiplicative group given by {Z/p}*, where p is a prime number. The order of the group is p-1.
 *      The code assumes that the user enters a prime number for the modular value p.
 */

#ifndef MOD_GROUP_H_
#define MOD_GROUP_H_

#include "Cyclic_group.h"
#include "Mod_p.h"
#include <NTL/ZZ.h>
NTL_CLIENT


class G_mod_p: public Cyclic_group {
private:
	Mod_p generator; //generator of the group
	ZZ mod; //modular value of the group
public:
	//Constructors & destructor
	G_mod_p();
	G_mod_p(Mod_p gen, long mod);
	G_mod_p(Mod_p gen, ZZ mod);
	G_mod_p(ZZ gen_val,  long mod);
	G_mod_p(ZZ gen_val,  ZZ mod);
	G_mod_p(long gen_val,  long mod);
	G_mod_p(long gen_val,  ZZ mod);
	G_mod_p(Mod_p gen);
	G_mod_p(ZZ mod);
	G_mod_p(long mod);

	virtual ~G_mod_p();

	//Access to the variables
	Mod_p get_gen() const;
	ZZ get_mod()const;

	//Assigment operator
	void operator =(const G_mod_p& H);

	//Functionality
	//Test if a given element is a generator of the group
	bool is_generator(const Mod_p& el);
	bool is_generator(const ZZ& x);
	bool is_generator(const long& x);
	//Returns the identity element of the group
	Mod_p identity();
	//Returns a n-th root of unity of the group
	Mod_p rootofunity(long n);
	Mod_p rootofunity(ZZ n);
	//Returns a random element in the group
	Mod_p random_el();
	//Returns a element in the group with the value val modular mod
	Mod_p element(ZZ val);
	Mod_p element(long val);
	//Returns the inverse of an element x
	Mod_p inverse(ZZ x);
	Mod_p inverse(long x);
};

#endif /* G_mod_p_H_ */
