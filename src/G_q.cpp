/*
 * G_q.cpp
 *
 *  Created on: 30.09.2010
 *      Author: stephaniebayer
 *
 *
 */

#include "G_q.h"

#include <NTL/ZZ.h>
NTL_CLIENT

#include <time.h>
G_q::G_q() {
	// TODO Auto-generated constructor stub

}


//Constructor creates an instance of G_q subset Z_p with order o and generator g
G_q::G_q(Mod_p gen, long o, long p){

	generator = gen;
	order = to_ZZ(o);
	mod = to_ZZ(p);
	if (gen.get_mod() != p)
		cout  << "The modular value of the generator and p are not equal" << endl;
}

//Constructor creates an instance of G_q subset Z_p with order o and generator g
G_q::G_q(Mod_p gen, long o, ZZ p){

	generator = gen;
	order = to_ZZ(o);
	mod = p;

	if (gen.get_mod() != p)
		cout  << "The modular value of the generator and p are not equal" << endl;

}


//Constructor creates an instance of G_q subset Z_p with order o and generator g
G_q::G_q(Mod_p gen, ZZ o, ZZ p){

	generator = gen;
	order = o;
	mod = p;

	if (gen.get_mod() != p)
		cout  << "The modular value of the generator and p are not equal" << endl;

}


//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(ZZ val, long o, long p){

	generator = Mod_p(val, p);
	order = to_ZZ(o);
	mod = to_ZZ(p);

}

//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(ZZ val, long o, ZZ p){

	generator = Mod_p(val, p);
	order = to_ZZ(o);
	mod = p;

}

//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(ZZ val, ZZ o, ZZ p){

	generator = Mod_p(val, p);
	order = o;
	mod = p;

}

//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(long val, long o, long p){

	generator = Mod_p(val, p);
	order = to_ZZ(o);
	mod = to_ZZ(p);

}

//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(long val, long o, ZZ p){

	generator = Mod_p(val, p);
	order = to_ZZ(o),
	mod = p;

}

//Constructor creates an instance of G_q subset Z_p with order o and generator value val
G_q::G_q(long val,ZZ o, ZZ p){

	generator = Mod_p(val, p);
	order = o;
	mod = p;

}

//Constructor creates an instance of G_q with order o, generator gen and G_q is a subgroup if Z modulo gen.get_mod()
G_q::G_q(Mod_p gen, long o){

	generator = gen;
	order = to_ZZ(o);
	mod = gen.get_mod();
}

//Constructor creates an instance of G_q with order o, generator gen and G_q is a subgroup if Z modulo gen.get_mod()
G_q::G_q(Mod_p gen, ZZ o){

	generator = gen;
	order = o;
	mod = gen.get_mod();
}

//Constructor creates an instance of G_q  subset of Z_p with order o and searchs for the smallest generator
G_q::G_q(long o,ZZ p){

	ZZ i;
	order = to_ZZ(o);
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

//Constructor creates an instance of G_q  subset of Z_p with order o and searchs for the smallest generator
G_q::G_q(ZZ o,ZZ p){

	ZZ i,t;
	order = o;
	mod = p;
	for (i = to_ZZ(2); i < p; ++i)
	{
		t=i%100000;
		if(t==0){
			cout<<";";
		}
		if (is_generator(i))
		{
			generator = Mod_p(i,p);
			break;
		}
	}
}

//Constructor creates an instance of G_q  subset of Z_p with order o and searchs for the smallest generator
G_q::G_q(long o, long p){

	long i;
	order = to_ZZ(o);
	mod = to_ZZ(p);
	for (i = 1; i < p; ++i)
	{
		if (is_generator(i))
		{
			generator = Mod_p(i,p);
			break;
		}
	}
}


G_q::~G_q() {
	// TODO Auto-generated destructor stub
}


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
	ZZ pow;
	bool b;
	b=false;
	pow = PowerMod(el.get_val(),order,mod);
	if(pow == to_ZZ(1)& el.get_val()!=1)
	{
		b=true;
	}
	return b;
}

//Checks if an element is a generator of the group G
bool G_q::is_generator(const ZZ& x){
	ZZ pow;
	bool b;
	b=false;
	pow = PowerMod(x,order,mod);

	if(pow == to_ZZ(1) & x!=1)
	{
		b=true;

	}

	return b;
}

//Checks if an element is a generator of the group G
bool G_q::is_generator(const long& x){
	ZZ pow;
	bool b;
	pow = PowerMod(to_ZZ(x),order, mod);
	if(pow == to_ZZ(1)& x!=1)
	{
		b=true;

	}

	return b;
}

//returns the identity of the group
Mod_p G_q::identity(){

	return Mod_p(1, mod);
}

//returns a random element of the group
Mod_p G_q::random_el(){
	ZZ ran,pow;
	Mod_p temp;
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(mod);
	temp = generator.expo(ran);

	return temp;

}

//returns a random element of the group, without setting the seed
Mod_p G_q::random_el(int c){
	ZZ ran,pow;
	Mod_p temp;
	ran = RandomBnd(order);
	temp = generator.expo(ran);


	return temp;

}

//returns an element of the group with value v
Mod_p G_q::element(ZZ v){


	return Mod_p(v,mod);

}


//returns an element of the group with value v
Mod_p G_q::element(long v){


	return Mod_p(v,mod);

}


void G_q::operator =(const G_q& H){

	generator = H.get_gen();
	order = H.get_ord();
	mod = H.get_mod();
}

//Output operator, output format is (generator value, order, modular value)
ostream& operator<<(ostream& os, const G_q G){
	return os<< "("<< G.get_gen().get_val()<< ", " << G.get_ord() <<", " << G.get_mod()<<")";
}

