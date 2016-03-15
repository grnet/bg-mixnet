/*
 * ElGammal.cpp
 *
 *  Created on: 03.10.2010
 *      Author: stephaniebayer
 */

#include "ElGammal.h"
#include "G_q.h"

#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT

#include "Mod_p.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>

void ElGammal::print() const {
	cout <<"Elgamal Keys:" << endl;
	G.print();
	cout << "secret:" << endl;
	cout << sk << endl;
	cout << "public:" << endl;
	cout << pk << endl;
}

ElGammal::ElGammal() {}

ElGammal::ElGammal(const ElGammal &other) {
  G = other.G;
  pk = other.pk;
  sk = other.sk;
}

ElGammal::~ElGammal() {}

//Access to the parameters
G_q ElGammal::get_group()const{
	return G;
}

Mod_p ElGammal::get_pk() const{
	return pk;
}

ZZ ElGammal::get_sk()const{
	return sk;
}

//functions to change parameters
void ElGammal::set_group(G_q H){
	G = H;
}

void ElGammal::set_sk(long s){
	sk = to_ZZ(s);
	// pk = G.get_gen().expo( s); // TODO remove containing stack -- this is never called
}

void ElGammal::set_pk(Mod_p& pk_) {
	pk = pk_;
}

void ElGammal::set_sk(ZZ s){
	sk = s;
#if USE_REAL_POINTS
        ZZ mod = G.get_mod();
        CurvePoint x;
        basepoint_scalarmult(x, s);
        pk = Mod_p(x, mod);
#else
	pk = G.get_gen().expo(s);
#endif
}

Cipher_elg ElGammal::encrypt(Mod_p el, ZZ ran){
	// yossigi: this is the function that is called
	//cout << "called " << el << " and " << ran <<endl;
	Cipher_elg c;
	Mod_p temp_1, temp_2;
#if USE_REAL_POINTS
        ZZ mod = G.get_mod();
        CurvePoint x;
        basepoint_scalarmult(x, ran);
        temp_1 = Mod_p(x, mod);
#else
	temp_1 = G.get_gen().expo(ran);
#endif
	temp_2 = pk.expo(ran)*el;
	c = Cipher_elg(temp_1,temp_2);

	return c;

}

Cipher_elg ElGammal::encrypt(CurvePoint m, ZZ ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
#if USE_REAL_POINTS
        ZZ mod = G.get_mod();
        CurvePoint x;
        basepoint_scalarmult(x, ran);
        temp_1 = Mod_p(x, mod);
#else
	temp_1 = G.get_gen().expo(ran);
#endif
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);

	return c;
}

//Decrypts the ciphertext c
Mod_p ElGammal::decrypt(Cipher_elg c){
	CurvePoint temp;
	ZZ mod = G.get_mod();
	temp = InvMod(c.get_u(),mod);
	temp = PowerMod(temp,sk, mod);
	temp = MulMod(temp,c.get_v(),mod);
	return Mod_p(temp, mod);
}

//Assigment operator
void ElGammal::operator=(const ElGammal& el){

	G = el.get_group();
	sk = el.get_sk();
	pk = el.get_pk();
}

