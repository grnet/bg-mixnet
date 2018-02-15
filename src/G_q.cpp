/*
 * G_q.cpp
 *
 *  Created on: 30.09.2010
 *      Author: stephaniebayer
 *
 *
 */

#include "G_q.h"

#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT

#include <assert.h>

#include <time.h>
#include <sstream>
void G_q::print() const {
	cout << generator << endl;
	cout << order << endl;
	cout << mod << endl;
}

G_q::G_q() {}

//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(CurvePoint val, ZZ o, ZZ p){
	generator = Mod_p(val, p);
	order = o;
	mod = p;
}

G_q::~G_q() {}


//return the generator
Mod_p G_q::get_gen()const{

	return generator;
}

//return the order o
ZZ G_q::get_ord()const{

	return order;
}

//return the modular value mod
ZZ G_q::get_mod()const{

	return mod;
}

//Checks if an element is a generator of the group G
bool G_q::is_generator(const Mod_p& el){
#if USE_REAL_POINTS
        return true; // TODO replace with not_identity
#else
	CurvePoint pow;
	bool b;
	b=false;
	pow = PowerMod(el.get_val(),order,mod);
	if((pow == curve_zeropoint()) & (el.get_val()!=curve_zeropoint()))
	{
		b=true;
	}
	return b;
#endif
}

//returns the identity of the group
Mod_p G_q::identity(){
  // TODO this is never called, right?
  assert(false);
	return Mod_p(curve_zeropoint(), mod);
}

Mod_p G_q::map_to_group_element(ZZ& m) {
#if USE_REAL_POINTS
        CurvePoint x;
        basepoint_scalarmult(x, m);
        return Mod_p(x, mod);
#else
	return generator.expo(m);
#endif
}


//returns an element of the group with value v
Mod_p G_q::element(CurvePoint v){


	return Mod_p(v,mod);

}


//returns an element of the group with value v
Mod_p G_q::element(long v){
  // TODO this is never called, right?
  assert(false);


	// return Mod_p(to_curve_pt(v),mod);
  return Mod_p(curve_zeropoint(),mod);

}


void G_q::operator =(const G_q& H){

	generator = H.get_gen();
	order = H.get_ord();
	mod = H.get_mod();
}

//Output operator, output format is (generator value, order, modular value)
ostream& operator<<(ostream& os, const G_q G){
	return os<< "(";
        os << G.get_gen().get_val();
        return os << ", " << G.get_ord() <<", " << G.get_mod()<<")";
}

