/*
 * Cipher_elg.cpp
 *
 *  Created on: 03.10.2010
 *      Author: stephaniebayer
 */

#include "Cipher_elg.h"
#include "Mod_p.h"
#include "G_q.h"

#include <assert.h>

#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT

extern G_q H;// group used for the the encryption

Cipher_elg::Cipher_elg() {}

// added: explicit initializer
Cipher_elg::Cipher_elg(bool dummy) {
#if !USE_NTL
        mod.is_initialized = true;
        mod.is_scalar = true;
#endif
}

Cipher_elg::Cipher_elg(CurvePoint u_val, CurvePoint v_val, ZZ mod_in){
	u = u_val;
	v = v_val;
	mod = mod_in;
}

Cipher_elg::Cipher_elg(Mod_p u_t, Mod_p v_t){

	if (u_t.get_mod()!=v_t.get_mod()) {
		cout << "It is not possible to set these elements: mods:" << u_t << " and " << v_t.get_mod() << endl;
	} else {
                u = u_t.get_val();
		v= v_t.get_val();
		mod = u_t.get_mod();
	}
}


Cipher_elg::~Cipher_elg() {}

//access to value of u
CurvePoint Cipher_elg::get_u() const{
	return u;
}

//access to value of v
CurvePoint Cipher_elg::get_v()const{
	return v;
}

//access to the value of mod
ZZ Cipher_elg::get_mod()const{
	return mod;
}

//Assignment operator
void Cipher_elg::operator =(const Cipher_elg& c){
	u = c.get_u();
	v = c.get_v();
	mod = c.get_mod();
}

//Multiplicative operator and multiplication functions
Cipher_elg Cipher_elg::operator *(const Cipher_elg& el)const{
	CurvePoint temp_1,temp_2;

	MulMod(temp_1,u,el.get_u(),mod);
	MulMod(temp_2,v,el.get_v(),mod);

	return Cipher_elg(temp_1, temp_2,mod);
}

void Cipher_elg::mult(Cipher_elg & a, const Cipher_elg& b, const Cipher_elg& c){
	CurvePoint temp_1,temp_2;
	ZZ mod = b.get_mod();
	MulMod(temp_1,b.get_u(),c.get_u(),mod);
	MulMod(temp_2,b.get_v(),c.get_v(),mod);
	a= Cipher_elg(temp_1, temp_2,mod);
}

//Equality Check
bool Cipher_elg::operator ==(const Cipher_elg& b) const{
	bool bo = false;
	if (u == b.get_u()) {
		if (v == b.get_v()) {
			bo=true;
		}
	}
	return bo;
}

void Cipher_elg::expo(Cipher_elg& a, const Cipher_elg& el, const ZZ ex){
	CurvePoint t_u, t_v;
	ZZ mod =el.get_mod();
	PowerMod(t_u, el.get_u(),ex, mod);
	PowerMod(t_v,el.get_v(),ex,mod);
	a= Cipher_elg(t_u,t_v, mod);
}

//Output operator, the format of a ciphertext is (u,v) (modular mod)
ostream& operator <<(ostream&os, const Cipher_elg b){
	os << "(";
        os << b.get_u();
        os << ",";
        os << b.get_v();
        os << ")";
        return os;
}

//Input operator,
istream& operator >>(istream& is, Cipher_elg& el){
	CurvePoint val_u, val_v; ZZ mod;
	char ch1, ch2, ch3;
	//char str1, str2, str3, str4;
	is >> ch1;
        is >> val_u;
        is >> ch2;
        is >> val_v;
        is >> ch3;
	if (ch1 != '(' || ch2 !=',' || ch3 != ')'){// || str3 != '(' || str4 != ')' ){
		is.clear(ios_base::failbit);
		return is;
	}
	mod = H.get_mod();
	el = Cipher_elg(val_u, val_v, mod);

	return is;
}

void Cipher_elg::print() const {
	//   cout << u << endl;
	//   cout << v << endl;
	cout << mod << endl;
}
