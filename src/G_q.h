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

#include "Mod_p.h"
#include "FakeZZ.h"
#include "CurvePoint.h"
#include <fstream>
NTL_CLIENT


class G_q {
private:
	Mod_p generator;  //generator of the group
	ZZ order; //order of the group
	ZZ mod;  //value of p such that G_q subset of Z_p
public:
	void print() const;

	//Constructors and destructor
	G_q();
	G_q(CurvePoint gen_val, ZZ o , ZZ mod);
	virtual ~G_q();

	//Access to the variables
	Mod_p get_gen() const;
	ZZ get_ord() const;
	ZZ get_mod()const;
	Mod_p map_to_group_element(ZZ& m);
	//operators
	void operator =(const G_q& H);
	friend ostream& operator<<(ostream& os, const G_q& G);


	//Test if an element is a generator of the group
	bool is_generator(const Mod_p& el);

	//returns the identity
	Mod_p identity();

	//creates an element with value val
	Mod_p element(CurvePoint val);
	Mod_p element(long val);

	//returns the inverse of an value
	Mod_p inverse(CurvePoint x);
	Mod_p inverse(long x);
};

#endif /* G_q_H_ */
