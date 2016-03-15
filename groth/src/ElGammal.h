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
#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT

#include "Mod_p.h"


class ElGammal {
private:
	G_q G;  //Group used for encryption
	ZZ sk;	//secret key
	Mod_p pk; //public key
public:
	//Constructor & destructor
	void print() const;
	ElGammal();
        ElGammal(const ElGammal &other);
	virtual ~ElGammal();

	//Access to the variables
	G_q get_group() const;
	Mod_p get_pk() const;
	ZZ get_sk() const;

	//functions to change parameters
	void set_group(G_q G);
	void set_sk(ZZ s);
	void set_sk(long s);
	void set_pk(Mod_p& pk_);

	//encryption and decryption functions
	Cipher_elg encrypt(Mod_p m, ZZ ran);
	Cipher_elg encrypt(CurvePoint m, ZZ ran);

	//decryption function
	Mod_p decrypt(Cipher_elg c);

	//Assigment operator
	void operator =(const ElGammal& el);

};

#endif /* ELGAMMAL_H_ */
