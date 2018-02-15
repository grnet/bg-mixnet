/*
 * Pedersen.cpp
 *
 *  Created on: 04.10.2010
 *      Author: stephaniebayer
 */
#include "Pedersen.h"
#include "G_q.h"
#include "FakeZZ.h"
NTL_CLIENT

#include "Mod_p.h"
#include "multi_expo.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>
#include <sstream>

extern G_q G;

void Pedersen::print_commitment(long i) const {
	cout << "random for commitment = " << gen->at(i) << endl;
}

string Pedersen::get_public_vector() const {
	string ret;
	stringstream ss(ret);
	for (unsigned int i = 0; i < gen->size(); i++) {
		ss << gen->at(i) << endl;
	}

	return ss.str();
}

void Pedersen::set_public_vector(istringstream& f, long n, int o1, int o2, int o3) {
    string line;
	
	if (gen != nullptr)	{ 
		gen->clear();
		delete gen;
	}
	gen = new vector<Mod_p>(n+1);

	Mod_p temp;
	long i = 0;
    while (std::getline(f, line)) {
		istringstream gstr(line);
        gstr >> gen->at(i);
		i++;
    }
	set_omega(o1, o2, o3);
}

Pedersen::Pedersen() : gen(nullptr) {
// TODO remove containing stack -- this is never called
}

//Generates an instance of Pedersen with group H, which is able to commit up to n elements
Pedersen::Pedersen(long n) { //, G_q H){
	long i;
	bool b;
	ZZ ran;
	Mod_p temp;

	//G = H;
	gen = new vector<Mod_p>(n+1);
	for (i =0; i <=n; i++)
	{
		b = true;
		while(b){
			ran = RandomBnd(G.get_ord());

#if USE_REAL_POINTS
                        ZZ mod = G.get_mod();
                        CurvePoint x;
                        basepoint_scalarmult(x, ran);
                        temp = Mod_p(x, mod);
#else
			temp = G.get_gen().expo(to_ZZ(ran));
#endif
			if (G.is_generator(temp))
			{
				(*gen).at(i)=temp;
				b = false;
			}
		}
	}
        gen_prec = 0;
}


Pedersen::~Pedersen() {
	if (gen != nullptr) {
		gen->clear();
		delete gen;
	}
        if (gen_prec != 0) {
          delete gen_prec->at(0);
          delete gen_prec->at(1);
          delete gen_prec;
        }
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

//Calculates the commitment to  t using randomness r
Mod_p Pedersen::commit(ZZ t,  ZZ ran){

	CurvePoint temp,temp_1;
	ZZ mod = G.get_mod();

	PowerMod(temp,gen->at(0).get_val(),ran, mod);
	PowerMod(temp_1,gen->at(1).get_val(),t,mod);
	MulMod(temp , temp,temp_1,mod);

	return Mod_p(temp, mod);
}

//Calculates the commitment to the values in t using randomness r
Mod_p Pedersen::commit_opt(const vector<ZZ>*  t, ZZ ran){
	CurvePoint temp;
	Mod_p temp_1;
	long length = t->size();

	if (length > (long) gen->size()) {
		cout << "too many elements to commit to. Max = " << gen->size() << " Requested: " << t->size() << endl;
		throw runtime_error(string("too many elements to commit to"));
	}
	else
	{
			multi_expo::expo_mult(temp, t, ran, omega_expo, gen);
	}

	return Mod_p(temp, G.get_mod());
}


//Calculates the commitment to  t using randomness r
Mod_p Pedersen::commit_sw(ZZ t, ZZ ran){

	CurvePoint temp;
	multi_expo::multi_expo_sw(temp, ran, t, omega_sw, gen_prec);
	return Mod_p(temp, G.get_mod());
}

void Pedersen::operator =(const Pedersen& el){
	G = el.get_group();
	gen = el.get_gen();
}

long Pedersen::to_long(vector<int>* bit_r){

	long  t, length;
	long i;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*(1L << i);
	}
	return t;
}

void Pedersen::to_long(long& t, vector<int>* bit_r){

	long   length;
	long i;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*(1L << i);
	}
}

vector<vector<CurvePoint>* >* Pedersen::precomp(CurvePoint g, CurvePoint h){
	vector<vector<CurvePoint>* >* pre;
	vector<CurvePoint>* t_1;
	vector<CurvePoint>* t_2;
	CurvePoint temp_1, temp_2;
	ZZ mod = G.get_mod();
	long i,t;

	pre = new vector<vector<CurvePoint> *>(2);
        t = 1L << (omega_sw-1);
	t_1 = new vector<CurvePoint>(t);
	t_2 = new vector<CurvePoint>(t);

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
