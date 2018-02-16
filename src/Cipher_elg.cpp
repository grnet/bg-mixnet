/*
 * Cipher_elg.cpp
 *
 *  Created on: 03.10.2010
 *      Author: stephaniebayer
 */

#include "Cipher_elg.h"
#include "Mod_p.h"
#include "G_q.h"
extern G_q H;


#include <NTL/ZZ.h>
NTL_CLIENT

Cipher_elg::Cipher_elg() {
	// TODO Auto-generated constructor stub

}

//Constructors create an element Cipher_elg with values u_val and v_val in group ZZ_p
Cipher_elg::Cipher_elg(long u_val, long v_val, long mod_in){

	u = to_ZZ(u_val);
	v = to_ZZ(v_val);
	mod = to_ZZ(mod_in);
}

Cipher_elg::Cipher_elg(ZZ u_val, long v_val, long mod_in){

	u = u_val;
	v = to_ZZ(v_val);
	mod = to_ZZ(mod_in);
}

Cipher_elg::Cipher_elg(long u_val, ZZ v_val, long mod_in){


	u = to_ZZ(u_val);
	v = v_val;
	mod = to_ZZ(mod_in);
}

Cipher_elg::Cipher_elg(ZZ u_val, ZZ v_val, long mod_in){

	u = u_val;
	v = v_val;
	mod = to_ZZ(mod_in);
}

Cipher_elg::Cipher_elg(long u_val, long v_val, ZZ mod_in){

	u = to_ZZ(u_val);
	v = to_ZZ(v_val);
	mod = mod_in;
}

Cipher_elg::Cipher_elg(ZZ u_val, long v_val, ZZ mod_in){

	u = u_val;
	v = to_ZZ(v_val);
	mod = mod_in;
}

Cipher_elg::Cipher_elg(long u_val, ZZ v_val, ZZ mod_in){

	u = u_val;
	v = to_ZZ(v_val);
	mod = mod_in;
}

Cipher_elg::Cipher_elg(ZZ u_val, ZZ v_val, ZZ mod_in){
	u = u_val;
	v = v_val;
	mod = mod_in;
}

Cipher_elg::Cipher_elg(Mod_p u_t, Mod_p v_t){

	if (u_t.get_mod()!=v_t.get_mod())
		cout << "It is not possible to set these elements" << endl;
	else
	{	u = u_t.get_val();
		v= v_t.get_val();
		mod = u_t.get_mod();
	}
}


Cipher_elg::~Cipher_elg() {
	// TODO Auto-generated destructor stub
}

//access to value of u
ZZ Cipher_elg::get_u() const{

	return u;
}

//access to value of v
ZZ Cipher_elg::get_v()const{

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
	ZZ temp_1,temp_2;

	MulMod(temp_1,u,el.get_u(),mod);
	MulMod(temp_2,v,el.get_v(),mod);
	return Cipher_elg(temp_1, temp_2,mod);
}

void Cipher_elg::mult(Cipher_elg & a, const Cipher_elg& b, const Cipher_elg& c){
	ZZ temp_1,temp_2;
	ZZ mod = b.get_mod();
	MulMod(temp_1,b.get_u(),c.get_u(),mod);
	MulMod(temp_2,b.get_v(),c.get_v(),mod);
	a= Cipher_elg(temp_1, temp_2,mod);
}

//Equality Check
bool Cipher_elg::operator ==(const Cipher_elg& b) const{
	bool bo = false;
	if (u == b.get_u())
	{
		if (v == b.get_v())
		{
			bo=true;
		}
	}
	return bo;
}

//Exponentiation function
Cipher_elg Cipher_elg::expo(const Cipher_elg& el, const ZZ ex){
	ZZ t_u, t_v;
	ZZ mod = el.get_mod();
	t_u = PowerMod(el.get_u(),ex, mod);
	t_v = PowerMod(el.get_v(),ex, mod);
	return Cipher_elg(t_u,t_v, mod);

}

Cipher_elg Cipher_elg::expo(const Cipher_elg& el, const int ex){
	ZZ t_u, t_v;
	ZZ mod = el.get_mod();
	t_u = PowerMod(el.get_u(),ex, mod);
	t_v = PowerMod(el.get_v(),ex, mod);
	return Cipher_elg(t_u,t_v, mod);

}

void Cipher_elg::expo(Cipher_elg& a, const Cipher_elg& el, const ZZ ex){
	ZZ t_u, t_v;
	ZZ mod =el.get_mod();
	PowerMod(t_u, el.get_u(),ex, mod);
	PowerMod(t_v,el.get_v(),ex,mod);
	a= Cipher_elg(t_u,t_v, mod);

}

void Cipher_elg::expo(Cipher_elg& a, const Cipher_elg& el, const long ex){
	ZZ t_u, t_v;
	ZZ mod =el.get_mod();
	PowerMod(t_u, el.get_u(),ex, mod);
	PowerMod(t_v,el.get_v(),ex,mod);
	a= Cipher_elg(t_u,t_v, mod);

}


//function to calculate the inverse
Cipher_elg Cipher_elg::inverse(const Cipher_elg& el){
	ZZ t_u, t_v;

	t_u = InvMod(el.get_u(), el.get_mod());
	t_v = InvMod(el.get_v(), el.get_mod());
	return Cipher_elg(t_u,t_v,el.get_mod());

}

void Cipher_elg::inverse(Cipher_elg& a, const Cipher_elg& el){
	ZZ t_u, t_v;
	ZZ mod = el.get_mod();
	InvMod(t_u, el.get_u(), mod);
	InvMod(t_v, el.get_v(), mod);
	a= Cipher_elg(t_u,t_v,mod);

}


//Output operator, the format of a ciphertext is (u,v) (modular mod)
ostream& operator <<(ostream&os, const Cipher_elg b){

	return os << "("<< b.get_u() <<"," << b.get_v() << ")"; // ("<<b.get_mod()<<")" ;
}

//Input operator,
istream& operator >>(istream& is, Cipher_elg& el){
	ZZ val_u, val_v, mod;
	char ch1, ch2, ch3;
	//char str1, str2, str3, str4;
	is >> ch1 >>val_u >> ch2 >> val_v >> ch3;// >> str3 >> mod >>str4;
	if (ch1 != '(' || ch2 !=',' || ch3 != ')'){// || str3 != '(' || str4 != ')' ){
		is.clear(ios_base::failbit);
		return is;
	}
	mod = H.get_mod();
	el = Cipher_elg(val_u, val_v, mod);
	return is;
}


