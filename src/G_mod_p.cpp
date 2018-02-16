/*
 * G_mod_p.cpp
 *
 *  Created on: 20.09.2010
 *      Author: stephaniebayer
 */

#include "G_mod_p.h"
#include <NTL/ZZ.h>
NTL_CLIENT


#include <time.h>
G_mod_p::G_mod_p() {
	// TODO Auto-generated constructor stub

}

//Sets the generator to gen and the mod to p, checks if the generator has the right modular value p
G_mod_p::G_mod_p(Mod_p gen,  long p){

	generator = gen;
	mod = to_ZZ(p);
	if (gen.get_mod() != p)
		cout  << "The modular value of the generator and p are not equal" << endl;
}

//Sets the generator to gen and the mod to p, checks if the generator has the right modular value p
G_mod_p::G_mod_p(Mod_p gen,  ZZ p){

	generator = gen;
	mod = p;

	if (gen.get_mod() != p)
		cout  << "The modular value of the generator and p are not equal" << endl;

}

//Sets the generator to the value gen and the mod to p
G_mod_p::G_mod_p(ZZ val,  long p){

	generator = Mod_p(val, p);
	mod = to_ZZ(p);

}

//Sets the generator to the value gen and the mod to p
G_mod_p::G_mod_p(ZZ val, ZZ p){

	generator = Mod_p(val, p);
	mod = p;

}

//Sets the generator to the value gen and the mod to p
G_mod_p::G_mod_p(long val,  long p){

	generator = Mod_p(val, p);
	mod = to_ZZ(p);

}

//Sets the generator to the value gen and the mod to p
G_mod_p::G_mod_p(long val, ZZ p){

	generator = Mod_p(val, p);
	mod = p;

}

//Creates a group given the generator, mod is set to modular value if gen
G_mod_p::G_mod_p(Mod_p gen){

	generator = gen;
	mod = gen.get_mod();
}

//Creates a group ZZ/p, the function sets the generator to the smallest possible one
G_mod_p::G_mod_p(ZZ p){

	ZZ i;
	mod = p;
	for (i = to_ZZ(1); i < p; ++i)
	{
		if (is_generator(i))
		{
			generator = Mod_p(i,p);
			break;
		}
	}
}

//Creates a group ZZ/p, the function sets the generator to the smallest possible one
G_mod_p::G_mod_p(long p){

	long i;
	mod = p;
	for (i = 1; i < p; ++i)
	{
		if (is_generator(i))
		{
			generator = Mod_p(i,p);
			break;
		}
	}
}

//Destructor
G_mod_p::~G_mod_p() {
	// TODO Auto-generated destructor stub
}

//Returns the generator
Mod_p G_mod_p::get_gen()const{

	return generator;
}

//Returns the modular value
ZZ G_mod_p::get_mod()const{

	return mod;
}

//Checks if an element is a generator of the group
bool G_mod_p::is_generator(const Mod_p& el){
	ZZ pow;
	bool b;
	b=false;
	pow = PowerMod(el.get_val(),(mod-1)/2,mod);
	if(pow == (mod-1))
	{
		if(el.get_val()!=(mod-1))
		{b=true;
		}
	}
	return b;
}

//Checks if an element with value x is a generator of the group
bool G_mod_p::is_generator(const ZZ& x){
	ZZ pow;
	bool b;
	b=false;
	pow = PowerMod(x,(mod-1)/2,mod);


	if(pow == (mod-1))
	{
		if(x!=(mod-1))
		{b=true;
		}
	}

	return b;
}

//Checks if an element with value x is a generator of the group
bool G_mod_p::is_generator(const long& x){
	ZZ pow;
	bool b;
	pow = PowerMod(to_ZZ(x),(mod-1)/2, mod);
	if(pow == (mod-1))
	{
		if(x!=(mod-1))
		{b=true;
		}
	}

	return b;
}

//Returns the identity of the group
Mod_p G_mod_p::identity(){

	return Mod_p(1, mod);
}

//return a random element of the group
Mod_p G_mod_p::random_el(){

	ZZ ran;
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(mod);

	return Mod_p(ran,mod);

}

//Creates an element with the value v modular mod
Mod_p G_mod_p::element(ZZ v){


	return Mod_p(v,mod);

}

//Creates an element with the value v modular mod
Mod_p G_mod_p::element(long v){


	return Mod_p(v,mod);

}

//Returns a n-th root of unity of the group
Mod_p G_mod_p::rootofunity(long n){

	ZZ i;
	ZZ pow;
	if ((mod-1) % n == 0)
	{	for(i =  to_ZZ(2); i<mod;++i)
		{
			if(GCD(i,mod)==to_ZZ(1))
			{

				if (n&1)
				{
					pow = PowerMod(i,n, mod);
					if(pow==1)
						return Mod_p(i,mod);
				}
				else
				{
					pow = PowerMod(i,n/2,mod);
					if(pow == (mod-1))
					{
						return Mod_p(i,mod);
					}
				}
			}
		}
	}
	else
	{
		cout << "There is no" << n <<"-th root of unity"<< endl;
		return Mod_p(1,mod);
	}
	return Mod_p(1,mod);
}

//Returns a n-th root of unity of the group
Mod_p G_mod_p::rootofunity(ZZ n){

	ZZ i;
	ZZ pow;
	if ((mod-1) % n == 0)
	{	for(i =  to_ZZ(2); i<mod;++i)
		{
			if(GCD(i,mod)==to_ZZ(1))
			{

				if (IsOdd(n))
				{
					pow = PowerMod(i,n, mod);
					if(pow==1)
						return Mod_p(i,mod);
				}
				else
				{
					pow = PowerMod(i,n/2,mod);
					if(pow == (mod-1))
					{
						return Mod_p(i,mod);
					}
				}
			}
		}
	}
	else
	{
		cout << "There is no" << n <<"-th root of unity"<< endl;
		return Mod_p(1,mod);
	}
	return Mod_p(1,mod);
}

//Returns the inverse of an element with value x
Mod_p G_mod_p::inverse(ZZ x){

	ZZ temp;
	temp = InvMod(x,mod);
	return Mod_p(temp,mod);
}

//Returns the inverse of an element with value x
Mod_p G_mod_p::inverse(long x){

	ZZ temp;
	temp = InvMod(to_ZZ(x),mod);
	return Mod_p(temp,mod);
}

//Assignment operator of the group
void G_mod_p::operator =(const G_mod_p& H){

	generator = H.get_gen();
	mod = H.get_mod();
}




