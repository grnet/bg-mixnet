/*
 * Mod_p.h
 *
 *  Created on: 15.09.2010
 *      Author: stephaniebayer
 *
 *      An instance of thi class represents an element of Z_p. The class gives all operators and functionality needed.
 *
 */

#ifndef MOD_P_H_
#define MOD_P_H_

#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT

class Mod_p {
private:
	CurvePoint val; //Value of the element
	ZZ mod; //Modular value
public:
	//Constructors and destructor
	Mod_p();
	Mod_p(CurvePoint v, ZZ p);
	Mod_p(const Mod_p& other);
	virtual ~Mod_p();

        // added: explicit initializer
	Mod_p(bool dummy);

	//Access to the parameters
	ZZ get_mod() const;
	CurvePoint get_val() const;

	//operators
	void operator =(const Mod_p& el);
	Mod_p operator *(const Mod_p& b) const;
	bool operator ==(const Mod_p& b) const;
	bool operator !=(const Mod_p& b) const;

	friend ostream& operator<<(ostream& os, const Mod_p& b);
	friend istream& operator>>(istream& is,  Mod_p& b);

	//Returns the inverse of an element
	static void inv(Mod_p& a, const Mod_p& el);

	//multiplication and exponentiation functions
	static void mult(Mod_p& a,const Mod_p& b , const Mod_p& c);
	static void expo(Mod_p& a,const Mod_p& b, const long e);
	static void expo(Mod_p& a, const Mod_p& b,const ZZ e);
	Mod_p expo(const ZZ e);


};

#endif /* MOD_P_H_ */
