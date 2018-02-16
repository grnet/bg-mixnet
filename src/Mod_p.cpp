
/*
 * Mod_p.cpp
 *
 *  Created on: 15.09.2010
 *      Author: stephaniebayer
 */

#include "Mod_p.h"


#include "G_q.h"
extern G_q G;

#include <NTL/ZZ.h>
NTL_CLIENT


Mod_p::Mod_p() {
	// TODO Auto-generated constructor stub

}

//Creates an instance which belongs to Z_p
Mod_p::Mod_p(long p){

	mod = to_ZZ(p);
}


//Creates an instance which belongs to Z_p
Mod_p::Mod_p(ZZ p){

	mod = p;
}


//Creates an instance which belongs to Z_p with value v
Mod_p::Mod_p(long v, long p){

	mod = to_ZZ(p);
	val = to_ZZ(v) % mod;
}

//Creates an instance which belongs to Z_p with value v
Mod_p::Mod_p(ZZ v, long p){

	mod = to_ZZ(p);
	val = v % mod;
}


//Creates an instance which belongs to Z_p with value v
Mod_p::Mod_p(long v, ZZ p){

	mod = p;
	val = to_ZZ(v) % mod;
}


//Creates an instance which belongs to Z_p with value v
Mod_p::Mod_p(ZZ v, ZZ p){

	mod = p;
	val = v % mod;
}


Mod_p::~Mod_p() {
	// TODO Auto-generated destructor stub
}


//Changes the modular value p
void Mod_p::set_mod(long p){

	mod = to_ZZ(p);
}

//Changes the modular value p
void Mod_p::set_mod(ZZ p){

	mod = p;
}

//Sets or changes the value val
void Mod_p::set_val(long v){
	if (mod ==0)
		cout << "Please set a value for the modulus p" << endl;
	else
		val = to_ZZ(v)% mod;
}

//Sets or changes the value val
void Mod_p::set_val(ZZ p){

	if (mod ==0)
			cout << "Please set a value for the modulus p" << endl;
	else
		val = p % mod;
}

//Returns the modular value
ZZ Mod_p::get_mod() const{

	return mod;
}

//returns the value of the instance
ZZ Mod_p::get_val() const{

	return val;
}


//Assigment operator
void Mod_p::operator =(const Mod_p& el){

	mod = el.get_mod();
	val = el.get_val();
}

//Addition
Mod_p Mod_p::operator +(const Mod_p& el) const{
	ZZ temp;
	if (mod != el.get_mod())
	{		cout <<"It is not possible to add these elements" << endl;
			return Mod_p(0,1);}
	else
	{
		temp = AddMod(val, el.get_val(),mod);
		return Mod_p(temp, mod);}
}

//Subtraction
Mod_p Mod_p::operator -(const Mod_p& el) const{
	ZZ temp;
	if (mod != el.get_mod())
	{		cout <<"It is not possible to subtract these elements" << endl;
			return Mod_p(0,1);}
	else
	{
		temp = SubMod(val , el.get_val(),mod);
		return Mod_p(temp, mod);}
}

//Unary plus
Mod_p Mod_p::operator +() const{

	return Mod_p(val, mod);

}

//Unary Minus
Mod_p Mod_p::operator -() const{

	ZZ temp;
	temp = -val % mod;
	return Mod_p(temp, mod);

}

//Multiplication
Mod_p Mod_p::operator *(const Mod_p& el) const{

	ZZ temp;
	if (mod != el.get_mod())
	{		cout <<"It is not possible to multiply these elements" << endl;
			return Mod_p(0,1);}
	else
	{
		temp = MulMod(val,el.get_val(),mod);
			return Mod_p(temp, mod);}
}

//Division
Mod_p Mod_p::operator /(const Mod_p& el) const{

	ZZ temp;
	if (mod != el.get_mod())
	{		cout <<"It is not possible to divide these elements" << endl;
			return Mod_p(0,1);}
	else
	{
		temp = val*InvMod(el.get_val(), mod) ;
			return Mod_p(temp, mod);}
}

//Increment prefix
Mod_p& Mod_p::operator ++(){

	++val;
	return *this;
}

//Increment suffix
Mod_p Mod_p::operator ++(int){

    Mod_p temp = *this;
    ++val;
    return temp;


}

//Decrement prefix
Mod_p& Mod_p::operator --(){

	--val;
	return *this;
}

//Decrement suffix
Mod_p Mod_p::operator --(int){

    Mod_p temp = *this;
    --val;
    return temp;


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

//Smaller
bool Mod_p::operator <(const Mod_p& el) const{

	if (mod != el.get_mod())
	{
		cout << "It is not possible to compare to elements with different modulus" << endl;
		return false;

	}
	else
	{
		if (val < el.get_val())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

//Bigger
bool Mod_p::operator >(const Mod_p& el) const{

	if (mod != el.get_mod())
	{
		cout << "It is not possible to compare to elements with different modulus" << endl;
		return false;

	}
	else
	{
		if (val > el.get_val())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

// Smaller equal
bool Mod_p::operator <=(const Mod_p& el) const{

	if (mod != el.get_mod())
	{
		cout << "It is not possible to compare to elements with different modulus" << endl;
		return false;

	}
	else
	{
		if (val <=el.get_val())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

//Bigger equal
bool Mod_p::operator >=(const Mod_p& el) const{

	if (mod != el.get_mod())
	{
		cout << "It is not possible to compare to elements with different modulus" << endl;
		return false;

	}
	else
	{
		if (val >=el.get_val())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

//Addition assignment
Mod_p& Mod_p::operator +=(const Mod_p& el){

	if (mod != el.get_mod())
	{
		cout << "It is not possible to add these elements" << endl;
		return *this;
	}
	else
	{
		val = AddMod(val, el.get_val(), mod);
		return *this;
	}
}

//Subtraction assignment
Mod_p& Mod_p::operator -=(const Mod_p& el){

	if (mod != el.get_mod())
	{
		cout << "It is not possible to add these elements" << endl;
		return *this;
	}
	else
	{
		val = SubMod(val, el.get_val(), mod);
		return *this;
	}
}

//Multiplication assignment
Mod_p& Mod_p::operator *=(const Mod_p& el){

	if (mod != el.get_mod())
	{
		cout << "It is not possible to add these elements" << endl;
		return *this;
	}
	else
	{
		val = MulMod(val, el.get_val(),mod);
		return *this;
	}
}


//Division assignment
Mod_p& Mod_p::operator /=(const Mod_p& el){

	if (mod != el.get_mod())
	{
		cout << "It is not possible to add these elements" << endl;
		return *this;
	}
	else
	{
		val = MulMod(val,InvMod(el.get_val(),mod), mod);
		return *this;
	}
}

//Output operator, output format is val (modular mod)
ostream& operator <<(ostream& os , const Mod_p &b){

	return os << b.get_val();
}

//Input operator,
istream& operator>>(istream& is, Mod_p &b){
	ZZ val, mod;

	is>>val;

	mod = G.get_mod();
	b = Mod_p(val, mod);
	return is;
}


//Returns the inverse modular p of the element
Mod_p Mod_p::inv(){

	ZZ temp;
	temp = InvMod(val,mod);
	return Mod_p(temp,mod);
}

//Returns the inverse modular p of the element el
Mod_p Mod_p::inv(const Mod_p& el){
	ZZ temp;
	ZZ mod=el.get_mod();
	temp = InvMod(el.get_val(),mod);
	return Mod_p(temp, mod);

}

void Mod_p::inv(Mod_p&a, const Mod_p& el){
	ZZ temp;
	ZZ mod=el.get_mod();
	InvMod(temp, el.get_val(),mod);
	a= Mod_p(temp, mod);

}

//Multiplication functions
void Mod_p::mult(Mod_p& a , const Mod_p& b, const Mod_p& c){
	ZZ temp;
	ZZ mod=b.get_mod();
	MulMod(temp,b.get_val(), c.get_val(),mod);
	a= Mod_p(temp,mod);
}


//exponentiation functions
Mod_p Mod_p::expo(const long e){

	ZZ temp;
	temp = PowerMod(val,e,mod);
	return Mod_p(temp,mod);
}

Mod_p Mod_p::expo(Mod_p& a, long e){

	ZZ temp;
	ZZ mod=a.get_mod();
	temp = PowerMod(a.get_val(),e,mod);
	return Mod_p(temp,mod);
}

void Mod_p::expo(Mod_p& a ,const Mod_p& b,const long e){
	ZZ temp;
	ZZ mod=b.get_mod();
	PowerMod(temp, b.get_val(),e,mod);
	a= Mod_p(temp,mod);
}

void Mod_p::expo(Mod_p& a , const Mod_p& b, const ZZ e){
	ZZ temp;
	ZZ mod=b.get_mod();
	PowerMod(temp, b.get_val(), e, mod);
	a= Mod_p(temp,mod);
}

Mod_p Mod_p::expo(const ZZ e){

	ZZ temp;
	PowerMod(temp, val,e, mod);
	return Mod_p(temp,mod);
}

Mod_p Mod_p::expo( Mod_p& a, ZZ e){

	ZZ temp;
	ZZ mod=a.get_mod();
	temp = PowerMod(a.get_val(),e,mod);
	return Mod_p(temp,a.mod);
}

