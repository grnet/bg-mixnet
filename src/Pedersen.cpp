/*
 * Pedersen.cpp
 *
 *  Created on: 04.10.2010
 *      Author: stephaniebayer
 */
#include "Pedersen.h"
#include "G_q.h"
#include <NTL/ZZ.h>
NTL_CLIENT

#include "Mod_p.h"
#include "multi_expo.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>

extern double time_cm;
extern unsigned long commitment_multiplies;
extern unsigned long commitment_lifts;
extern unsigned long commitment_multi_lifts;

Pedersen::Pedersen() {
	// TODO Auto-generated constructor stub

}

//Generates an instance of Pedersen with group H, which is able to commit up to n elements
Pedersen::Pedersen(long n, G_q H){
	long i;
	bool b;
	ZZ ran;
	Mod_p temp;
	G = H;
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"ck for pedersen ";*/
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_ord());
			temp = G.get_gen().expo(ran);
			if (G.is_generator(temp))
			{
				(*gen).at(i)=temp;
			//	ost<<temp<<" ";
				b = false;
			}
		}
	}
//	ost<<endl;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and generator g,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(Mod_p g, long o, long p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
		ran = RandomBnd(G.get_mod());
		if (G.is_generator(ran))
		{
			gen-> at(i)=Mod_p(ran,G.get_mod());
			b = false;
		}
		}
	}

}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and generator g,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(Mod_p g, long o, ZZ p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen-> at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and generator g,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(Mod_p g, ZZ o, ZZ p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen-> at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;

}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and the generator g has value g_val,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(long g_val, long o, long p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g_val, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen-> at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;

}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and the generator g has value g_val,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(long g_val, long o, ZZ p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g_val, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen-> at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and the generator g has value g_val,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(long g_val, ZZ o, ZZ p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g_val, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;

}



/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and the generator g has value g_val,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(ZZ g_val, long o, long p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g_val, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}



/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and the generator g has value g_val,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(ZZ g_val, long o, ZZ p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g_val, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p and order o and the generator g has value g_val,
 * and which is able to commit up to n elements */
Pedersen::Pedersen(ZZ g_val, ZZ o, ZZ p, long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g_val, o, p);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p, p is the modular value of g
 * and order o and the generator g, and which is able to commit up to n elements */
Pedersen::Pedersen(Mod_p g, long o,  long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g, o);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}

	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p, p is the modular value of g
 * and order o and the generator g, and which is able to commit up to n elements */
Pedersen::Pedersen(Mod_p g, ZZ o,  long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(g, o);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p, and order o,
 *  and which is able to commit up to n elements */
Pedersen::Pedersen( long o, long mod,  long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(o,mod);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p, and order o,
 *  and which is able to commit up to n elements */
Pedersen::Pedersen( long o, ZZ mod,  long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(o,mod);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


/*Generates an instance of Pedersen with group G_q subset of Z_p, and order o,
 *  and which is able to commit up to n elements */
Pedersen::Pedersen( ZZ o, ZZ mod,  long n){
	long i;
	bool b;
	ZZ ran;
	G = G_q(o,mod);
	gen = new vector<Mod_p>(n+1);
	SetSeed(to_ZZ(time(0)));
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_mod());
			if (G.is_generator(ran))
			{
				gen->at(i)=Mod_p(ran,G.get_mod());
				b = false;
			}
		}
	}
	gen_prec = 0;
}


Pedersen::~Pedersen() {

//	delete gen;
	// TODO Auto-generated destructor stub
}

//returns the group of the instance
G_q Pedersen::get_group() const{

	return G;
}

void Pedersen:: set_omega(int o1, int o2, int o3){

	omega_expo = o1;
	omega_ll = o2;
	omega_sw = o3;
	gen_prec = precomp(gen->at(0).get_val(), gen->at(1).get_val());
}

//returns the number of generators used, it is possible to commit up to size()-1 values
long Pedersen::get_length() const{
	return gen->size();
}

//returns a list of all generators
vector<Mod_p>* Pedersen::get_gen()const{
	return gen;
}

//Calculates the commitment to the values in t using randomness r
Mod_p Pedersen::commit(const vector<ZZ>*  t, ZZ ran){
	double tstart, tstop, ttime;
	ZZ temp,temp_1,mod;
	long i;

	tstart = (double)clock()/CLOCKS_PER_SEC;
	mod = G.get_mod();
	if ((long) t->size() > (long) gen->size())
	{cout << "to many elements to commit to" << endl;}
	else
	{
		PowerMod(temp, gen->at(0).get_val(),ran, mod);
		commitment_lifts += 1;
		for (i = 1; i<=(long) t->size(); i++)
		{
			PowerMod(temp_1,gen->at(i).get_val(),t->at(i-1),mod);
			MulMod(temp,temp,temp_1, mod);
			commitment_multiplies += 1;
			commitment_lifts += 1;
		}
	}
	tstop = (double)clock()/CLOCKS_PER_SEC;
	ttime= tstop-tstart;
	time_cm+=ttime;
	return Mod_p(temp, mod);
}


//Calculates the commitment to the values in t using randomness r
Mod_p Pedersen::commit(const vector<Mod_p>*  t,  ZZ ran){
	double tstart, tstop, ttime;
	ZZ temp,temp_1;
	ZZ mod = G.get_mod();
	long i;

	tstart = (double)clock()/CLOCKS_PER_SEC;
	if ((long) t->size()> (long) gen->size())
	{cout << "to many elements to commit to" << endl;}
	else
	{
		PowerMod(temp,gen->at(0).get_val(),ran, mod);
		commitment_lifts += 1;
		for (i = 1; i<=(long) t->size(); i++)
		{
			PowerMod(temp_1,gen->at(i).get_val(),t->at(i-1).get_val(),mod);
			MulMod(temp , temp,temp_1,mod);
			commitment_multiplies += 1;
			commitment_lifts += 1;
		}
	}
	tstop = (double)clock()/CLOCKS_PER_SEC;
	ttime= tstop-tstart;
	time_cm+=ttime;
	return Mod_p(temp,mod);
}

//Calculates the commitment to t using randomness r
Mod_p Pedersen::commit(const vector<Mod_p>*  t){
	double tstart, tstop, ttime;
	ZZ temp,temp_1;
	long i;
	ZZ ran;
	ZZ mod = G.get_mod();

	tstart = (double)clock()/CLOCKS_PER_SEC;
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(mod);
     cout<< "ran: " << ran << endl;
	if ((long) t->size()> (long) gen->size())
	{cout << "to many elements to commit to" << endl;}
	else
	{
		PowerMod(temp,gen->at(0).get_val(),ran, mod);
		commitment_lifts += 1;
		for (i = 1; i<=(long) t->size(); i++)
		{
			PowerMod(temp_1,gen->at(i).get_val(),t->at(i-1).get_val(),mod);
			MulMod(temp , temp, temp_1,mod);
			commitment_multiplies += 1;
			commitment_lifts += 1;
		}
	}
	tstop = (double)clock()/CLOCKS_PER_SEC;
	ttime= tstop-tstart;
	time_cm+=ttime;
	return Mod_p(temp, mod);
}


//Calculates the commitment to  t using randomness r
Mod_p Pedersen::commit(ZZ t,  ZZ ran){

	ZZ temp,temp_1;
	ZZ mod = G.get_mod();

	//cout<<" in com ";
	PowerMod(temp,gen->at(0).get_val(),ran, mod);
	PowerMod(temp_1,gen->at(1).get_val(),t,mod);
	MulMod(temp , temp,temp_1,mod);

	return Mod_p(temp, mod);
}


//Calculates the commitment to t using randomness r
Mod_p Pedersen::commit(Mod_p t,  long ran){

	ZZ temp,temp_1;
	ZZ mod = G.get_mod();
	PowerMod(temp,gen->at(0).get_val(),ran, mod);
	PowerMod(temp_1,gen->at(1).get_val(),t.get_val(),mod);
	MulMod(temp , temp,temp_1,mod);

	return Mod_p(temp,mod);
}


//Calculates the commitment to t using randomness r
Mod_p Pedersen::commit(Mod_p t, ZZ ran){
	ZZ temp,temp_1;
	ZZ mod = G.get_mod();
	PowerMod(temp,gen->at(0).get_val(),ran, mod);
	PowerMod(temp_1,gen->at(1).get_val(),t.get_val(),mod);
	MulMod(temp,temp,temp_1,mod);

	return Mod_p(temp, mod);
}


//Calculates the commitments to the values in t, generating a random number
Mod_p Pedersen::commit(Mod_p t){

	ZZ temp,temp_1;
	ZZ ran;
	ZZ mod = G.get_mod();
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(mod);
     cout<< "ran: " << ran << endl;

	PowerMod(temp,gen->at(0).get_val(),ran, mod);
	PowerMod(temp_1,gen->at(1).get_val(),t.get_val(),mod);
	MulMod(temp,temp,temp_1,mod);

	return Mod_p(temp, mod);
}


//Calculates the commitment to the values in t using randomness r
Mod_p Pedersen::commit_opt(const vector<ZZ>*  t, ZZ ran){
	double tstart, tstop, ttime;
	ZZ temp;
	Mod_p temp_1;
	long length = t->size();
	long i;

	tstart = (double)clock()/CLOCKS_PER_SEC;
	if (length > (long) gen->size())
	{cout << "to many elements to commit to" << endl;}
	else
	{
		/*if(length <= 1000){
			vector<ZZ>* expo = new vector<ZZ>(length+1);
			expo->at(0)=ran;
			for(i=1; i<=length; i++){
					expo->at(i) = t->at(i-1);
			}
			multi_expo::multi_expo_LL(temp_1, gen, expo, omega_ll);
			temp = temp_1.get_val();
		} else {*/
			multi_expo::expo_mult(temp, t, ran, omega_expo, gen);
			commitment_multi_lifts += 1;
		//}
	}

	tstop = (double)clock()/CLOCKS_PER_SEC;
	ttime= tstop-tstart;
	time_cm+=ttime;
	return Mod_p(temp, G.get_mod());
}


//Calculates the commitment to  t using randomness r
Mod_p Pedersen::commit_sw(ZZ t,  ZZ ran){

	ZZ temp;
	multi_expo::multi_expo_sw(temp,ran, t, omega_sw, gen_prec);

	return Mod_p(temp, G.get_mod());
}


//Calculates the commitment to t using randomness r
Mod_p Pedersen::commit_sw(Mod_p t,  long ran){

	ZZ temp;
	multi_expo::multi_expo_sw(temp,to_ZZ(ran), t.get_val(), omega_sw, gen_prec);

	return Mod_p(temp, G.get_mod());
}


//Calculates the commitment to t using randomness r
Mod_p Pedersen::commit_sw(Mod_p t, ZZ ran){

	ZZ temp;
	multi_expo::multi_expo_sw(temp,to_ZZ(ran), t.get_val(), omega_sw, gen_prec);
	return Mod_p(temp, G.get_mod());
}


//Calculates the commitments to the values in t, generating a random number
Mod_p Pedersen::commit_sw(Mod_p t){

	ZZ temp;
	ZZ ran;
	ZZ mod = G.get_mod();
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(mod);

	multi_expo::multi_expo_sw(temp,to_ZZ(ran), t.get_val(), omega_sw, gen_prec);

	return Mod_p(temp, mod);
}


void Pedersen::operator =(const Pedersen& el){
	G = el.get_group();
	gen = el.get_gen();
}

long Pedersen::to_long(vector<int>* bit_r){

	long  t, length;
	double two,i;
	two = 2;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*pow(two,i);
	}
	return t;
}

void Pedersen::to_long(long& t, vector<int>* bit_r){

	long   length;
	double two,i;
	two = 2;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*pow(two,i);
	}
	//return t;
}




vector<vector<ZZ>* >* Pedersen::precomp(ZZ g, ZZ h){
	vector<vector<ZZ>* >* pre;
	vector<ZZ>* t_1;
	vector<ZZ>* t_2;
	ZZ temp_1, temp_2;
	ZZ mod = G.get_mod();
	double two;
	long i,t;

	pre = new vector<vector<ZZ> *>(2);
	two = 2;
	t = pow(two, omega_sw-1);
	t_1 = new vector<ZZ>(t);
	t_2 = new vector<ZZ>(t);

	temp_1 = sqr(g);
	temp_2 = sqr(h);
	t_1->at(0)= g;
	t_2->at(0)= h;
	for(i = 1; i<t; i++){
		t_1->at(i)= MulMod(t_1->at(i-1),temp_1,mod);
		t_2->at(i)= MulMod(t_2->at(i-1),temp_2,mod);
	}

	pre->at(0)= t_1;
	pre->at(1)= t_2;
	return pre;
}


