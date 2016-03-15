
/*
 * Mod_p.cpp
 *
 *  Created on: 15.09.2010
 *      Author: stephaniebayer
 */

#include "Mod_p.h"

#include <assert.h>

#include "G_q.h"
extern G_q G;

#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT


Mod_p::Mod_p() {}

// added: explicit initializer
Mod_p::Mod_p(bool dummy) {
#if !USE_NTL
        mod.is_initialized = true;
        mod.is_scalar = true;
#endif
}

//Creates an instance which belongs to Z_p with value v
Mod_p::Mod_p(CurvePoint v, ZZ p){
        val = v;
#if USE_REAL_POINTS
        // pass
#else
	mod = p;
# if !USE_NTL
        val.zz = v.zz % mod.zz;
# else
	val.zz = v.zz % mod;
# endif
#endif
}

Mod_p::Mod_p(const Mod_p& other) {
	mod = other.mod;
	val = other.val;
}


Mod_p::~Mod_p() {}


//Returns the modular value
ZZ Mod_p::get_mod() const{

	return mod;
}

//returns the value of the instance
CurvePoint Mod_p::get_val() const{

	return val;
}


//Assigment operator
void Mod_p::operator =(const Mod_p& el){
	mod = el.get_mod();
	val = el.get_val();
}

//Multiplication
Mod_p Mod_p::operator *(const Mod_p& el) const{
	CurvePoint temp;
        assert(mod == el.get_mod());
        temp = MulMod(val,el.get_val(),mod);
        return Mod_p(temp, mod);
}

//Equal to
bool Mod_p::operator ==(const Mod_p& el) const{
	if (mod == el.get_mod())
	{
		if (val == el.get_val())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

//Not equal to
bool Mod_p::operator !=(const Mod_p& el) const{
	if (mod != el.get_mod())
	{

		return true;

	}
	else
	{
		if (val != el.get_val())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

//Output operator, output format is val (modular mod)
ostream& operator <<(ostream& os , const Mod_p &b){
        os << b.get_val();
        return os;
}

//Input operator,
istream& operator>>(istream& is, Mod_p &b){
	CurvePoint val; ZZ mod;
        is >> val;
	mod = G.get_mod();
	b = Mod_p(val, mod);
	return is;
}

void Mod_p::inv(Mod_p&a, const Mod_p& el){
	CurvePoint temp;
	ZZ mod=el.get_mod();
	InvMod(temp, el.get_val(),mod);
	a= Mod_p(temp, mod);

}

//Multiplication functions
void Mod_p::mult(Mod_p& a , const Mod_p& b, const Mod_p& c){
	CurvePoint temp;
	ZZ mod=b.get_mod();
	MulMod(temp,b.get_val(), c.get_val(),mod);
	a= Mod_p(temp,mod);
}

void Mod_p::expo(Mod_p& a ,const Mod_p& b,const long e){
	CurvePoint temp;
	ZZ mod=b.get_mod();
	PowerMod(temp, b.get_val(),e,mod);
	a= Mod_p(temp,mod);
}

void Mod_p::expo(Mod_p& a , const Mod_p& b, const ZZ e){
	CurvePoint temp;
	ZZ mod=b.get_mod();
	PowerMod(temp, b.get_val(), e, mod);
	a= Mod_p(temp,mod);
}

Mod_p Mod_p::expo(const ZZ e){
	CurvePoint temp;
	PowerMod(temp, val,e, mod);
	return Mod_p(temp,mod);
}
