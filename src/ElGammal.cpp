/*
 * ElGammal.cpp
 *
 *  Created on: 03.10.2010
 *      Author: stephaniebayer
 */

#include "ElGammal.h"
#include "G_q.h"

#include <NTL/ZZ.h>
NTL_CLIENT

#include "Mod_p.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>


ElGammal::ElGammal() {
	// TODO Auto-generated constructor stub

}

//Creates ElGammal with secret key s, public key p and group H
ElGammal::ElGammal(long s, Mod_p p, G_q H){
	G=H;
	sk = to_ZZ(s);
	pk = p;

}

//Creates ElGammal with secret key s, public key p and group H
ElGammal::ElGammal(ZZ s, Mod_p p, G_q H){
	G=H;
	sk = s;
	pk = p;

}

//Creates ElGammal with secret key s and group H, the public key is pk = gen^s , gen generator of H
ElGammal::ElGammal(long s, G_q H){
	Mod_p temp;
	G = H;
	sk = to_ZZ(s);
	temp = Mod_p(G.get_gen().get_val(), G.get_mod());
	pk = temp.expo(s);
}

//Creates ElGammal with secret key s and group H, the public key is pk = gen^s , gen generator of H
ElGammal::ElGammal(ZZ s, G_q H){
	Mod_p temp;
	G = H;
	sk = s;
	temp = Mod_p(G.get_gen().get_val(), G.get_mod());
	pk = temp.expo(s);
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and pk = gen^s
ElGammal::ElGammal(Mod_p gen, long o, long  mod, long s){

	G = G_q(gen, o, mod);
	sk = to_ZZ(s);
	Mod_p temp;
	temp = Mod_p(gen.get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and pk = gen^s
ElGammal::ElGammal(Mod_p gen, long o, ZZ  mod, long s){

	G = G_q(gen, o, mod);
	sk = to_ZZ(s);
	Mod_p temp;
	temp = Mod_p(gen.get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and pk = gen^s
ElGammal::ElGammal(Mod_p gen, long o, ZZ  mod, ZZ s){

	G = G_q(gen, o, mod);
	sk = s;
	Mod_p temp;
	temp = Mod_p(gen.get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and pk = gen^s
ElGammal::ElGammal(Mod_p gen, ZZ o, ZZ  mod, long s){

	G = G_q(gen, o, mod);
	sk = to_ZZ(s);
	Mod_p temp;
	temp = Mod_p(gen.get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and pk = gen^s
ElGammal::ElGammal(Mod_p gen, ZZ o, ZZ  mod, ZZ s){

	G = G_q(gen, o, mod);
	sk = s;
	Mod_p temp;
	temp = Mod_p(gen.get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and public key p
ElGammal::ElGammal(Mod_p gen, long o, long  mod, long s, Mod_p p){

	G = G_q(gen, o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and public key p
ElGammal::ElGammal(Mod_p gen, long o, ZZ  mod, long s, Mod_p p){

	G = G_q(gen, o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and public key p
ElGammal::ElGammal(Mod_p gen, long o, ZZ  mod, ZZ s, Mod_p p){

	G = G_q(gen, o, mod);
	sk = s;
	pk = p;
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and public key p
ElGammal::ElGammal(Mod_p gen, ZZ o, ZZ  mod, long s, Mod_p p){

	G = G_q(gen, o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and public key p
ElGammal::ElGammal(Mod_p gen, ZZ o, ZZ  mod, ZZ s, Mod_p p){

	G = G_q(gen, o, mod);
	sk = s;
	pk = p;
}

//Set the group to G_q with order o, G_q subset of G_mod_p and generator gen, secret key is s and public key p
ElGammal::ElGammal( long o, long  mod, long s, Mod_p p){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key p
ElGammal::ElGammal( long o, ZZ  mod, long s, Mod_p p){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key p
ElGammal::ElGammal( long o, ZZ  mod, ZZ s, Mod_p p){

	G = G_q( o, mod);
	sk = s;
	pk = p;
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key p
ElGammal::ElGammal( ZZ o, ZZ  mod, long s, Mod_p p){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key p
ElGammal::ElGammal( ZZ o, ZZ  mod, ZZ s, Mod_p p){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	pk = p;
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key pk = gen^s
ElGammal::ElGammal( long o, long  mod, long s){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	Mod_p temp;
	temp = Mod_p(G.get_gen().get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key pk = gen^s
ElGammal::ElGammal( long o, ZZ  mod, long s){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	Mod_p temp;
	temp = Mod_p(G.get_gen().get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key pk = gen^s
ElGammal::ElGammal( long o, ZZ  mod, ZZ s){

	G = G_q( o, mod);
	sk = s;
	Mod_p temp;
	temp = Mod_p(G.get_gen().get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key pk = gen^s
ElGammal::ElGammal( ZZ o, ZZ  mod, long s){

	G = G_q( o, mod);
	sk = to_ZZ(s);
	Mod_p temp;
	temp = Mod_p(G.get_gen().get_val(), mod);
	pk = temp.expo(s);
}

//Set the group to G_q with order o and modular value mod, secret key is s and public key pk = gen^s
ElGammal::ElGammal( ZZ o, ZZ  mod, ZZ s){

	G = G_q( o, mod);
	sk = s;
	Mod_p temp;
	temp = Mod_p(G.get_gen().get_val(), mod);
	pk = temp.expo(s);
}


ElGammal::~ElGammal() {
	// TODO Auto-generated destructor stub
}

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
	pk = G.get_gen().expo( s);
}

void ElGammal::set_sk(ZZ s){

	sk = s;
	pk = G.get_gen().expo(s);
	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"private key and public key "<<sk<<" "<<pk<<endl;
}

//functions to encrypt value/element
Cipher_elg ElGammal::encrypt(Mod_p el){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	ZZ ran;
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(G.get_ord());
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*el;
	c = Cipher_elg(temp_1,temp_2);
	return c;

}

Cipher_elg ElGammal::encrypt(ZZ m){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	ZZ ran;
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(G.get_ord());
	cout<< ran << endl;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);
	return c;
}

Cipher_elg ElGammal::encrypt(long m){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	ZZ ran;
	SetSeed(to_ZZ(time(0)));
	ran = RandomBnd(G.get_ord());
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);
	return c;
}

Cipher_elg ElGammal::encrypt(Mod_p el, long ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*el;
	c = Cipher_elg(temp_1,temp_2);
	return c;

}

Cipher_elg ElGammal::encrypt(Mod_p el, ZZ ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*el;
	c = Cipher_elg(temp_1,temp_2);
	return c;

}


Cipher_elg ElGammal::encrypt(ZZ m, long ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);
	return c;
}

Cipher_elg ElGammal::encrypt(ZZ m, ZZ ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);
	return c;
}


Cipher_elg ElGammal::encrypt(long m, long ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);
	return c;
}


Cipher_elg ElGammal::encrypt(long m, ZZ ran){
	Cipher_elg c;
	Mod_p temp_1, temp_2;
	temp_1 = G.get_gen().expo(ran);
	temp_2 = pk.expo(ran)*Mod_p(m,G.get_mod());
	c = Cipher_elg(temp_1,temp_2);
	return c;
}

//Decrypts the ciphertext c
Mod_p ElGammal::decrypt(Cipher_elg c){
	ZZ temp;
	ZZ mod = G.get_mod();
	temp = InvMod(c.get_u(),mod);
	temp = PowerMod(temp,sk, mod);
	temp = MulMod(temp,c.get_v(),mod);
	return temp;
}

//Assigment operator
void ElGammal::operator=(const ElGammal& el){

	G = el.get_group();
	sk = el.get_sk();
	pk = el.get_pk();
}

