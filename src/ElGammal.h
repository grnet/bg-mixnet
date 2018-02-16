/*
 * ElGammal.h
 *
 *  Created on: 03.10.2010
 *      Author: stephaniebayer
 *
 *      An instance of the class represent the ElGammal encryption, it has information of the group used the secret and
 *      public key. The class also provides functions to encrypt and decrypt
 */

#ifndef ELGAMMAL_H_
#define ELGAMMAL_H_

#include "Cipher_elg.h"
#include "G_q.h"
#include <NTL/ZZ.h>
NTL_CLIENT

#include "Mod_p.h"


class ElGammal {
private:
	G_q G;  //Group used for encryption
	ZZ sk;	//secret key
	Mod_p pk; //public key
public:
	//Constructor & destructor
	ElGammal();
	ElGammal(long s, Mod_p p, G_q H);
	ElGammal(ZZ s, Mod_p p,G_q H);
	ElGammal(long s,G_q H);
	ElGammal(ZZ s,G_q H);
	ElGammal(Mod_p gen, long o, long mod, long s);
	ElGammal(Mod_p gen, long o, ZZ mod, long s);
	ElGammal(Mod_p gen, long o, ZZ mod, ZZ s);
	ElGammal(Mod_p gen, ZZ o, ZZ mod, long s);
	ElGammal(Mod_p gen, ZZ o, ZZ mod, ZZ s);
	ElGammal(Mod_p gen, long o, long mod, long s, Mod_p p);
	ElGammal(Mod_p gen, long o, ZZ mod, long s, Mod_p p);
	ElGammal(Mod_p gen, long o, ZZ mod, ZZ s, Mod_p p);
	ElGammal(Mod_p gen, ZZ o, ZZ mod, long s, Mod_p p);
	ElGammal(Mod_p gen, ZZ o, ZZ mod, ZZ s, Mod_p p);
	ElGammal(long o, long mod, long s, Mod_p p);
	ElGammal(long o, ZZ mod, long s, Mod_p p);
	ElGammal(ZZ o, ZZ mod, long s, Mod_p p);
	ElGammal(long o, ZZ mod, ZZ s, Mod_p p);
	ElGammal(ZZ o, ZZ mod, ZZ s, Mod_p p);
	ElGammal(long o, long mod, long s);
	ElGammal(long o, ZZ mod, long s);
	ElGammal(ZZ o, ZZ mod, long s);
	ElGammal(long o, ZZ mod, ZZ s);
	ElGammal(ZZ o, ZZ mod, ZZ s);
	virtual ~ElGammal();

	//Access to the variables
	G_q get_group() const;
	Mod_p get_pk() const;
	ZZ get_sk() const;

	//functions to change parameters
	void set_group(G_q G);
	void set_sk(ZZ s);
	void set_sk(long s);

	//encryption and decryption functions
	Cipher_elg encrypt(Mod_p m);
	Cipher_elg encrypt(ZZ m);
	Cipher_elg encrypt(long m);

	Cipher_elg encrypt(Mod_p m, long ran);
	Cipher_elg encrypt(Mod_p m, ZZ ran);
	Cipher_elg encrypt(ZZ m, long ran);
	Cipher_elg encrypt(ZZ m, ZZ ran);
	Cipher_elg encrypt(long m, long ran);
	Cipher_elg encrypt(long m, ZZ ran);

	//decryption function
	Mod_p decrypt(Cipher_elg c);

	//Assigment operator
	void operator =(const ElGammal& el);

};

#endif /* ELGAMMAL_H_ */
