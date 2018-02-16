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

#include "G_mem.h"
#include <NTL/ZZ.h>
NTL_CLIENT

class Mod_p: public G_mem {
private:
	ZZ val; //Value of the element
	ZZ mod; //Modular value
public:
	//Constructors and destructor
	Mod_p();
	Mod_p(long p);
	Mod_p(ZZ p);
	Mod_p(long v, long p);
	Mod_p(ZZ v, long p);
	Mod_p(long v, ZZ p);
	Mod_p(ZZ v, ZZ p);
	virtual ~Mod_p();

	//Functions to change parameters
	void set_mod(long p);
	void set_mod(ZZ p);
	void set_val(long v);
	void set_val(ZZ v);

	//Access to the parameters
	ZZ get_mod() const;
	ZZ get_val() const;

	//operators
	void operator =(const Mod_p& el);
	Mod_p operator +(const Mod_p& el) const;
	Mod_p operator -(const Mod_p& el) const;
	Mod_p operator +() const;
	Mod_p operator -() const;
	Mod_p operator *(const Mod_p& b) const;
	Mod_p operator /(const Mod_p& b) const;
	Mod_p& operator ++();
	Mod_p operator ++(int);
	Mod_p& operator --();
	Mod_p operator --(int);
	bool operator ==(const Mod_p& b) const;
	bool operator !=(const Mod_p& b) const;
	bool operator <(const Mod_p& b) const;
	bool operator >(const Mod_p& b) const;
	bool operator <=(const Mod_p& b) const;
	bool operator >=(const Mod_p& b) const;
	Mod_p& operator +=(const Mod_p& b);
	Mod_p& operator -=(const Mod_p& b);
	Mod_p& operator *=(const Mod_p& b);
	Mod_p& operator /=(const Mod_p& b);

	friend ostream& operator<<(ostream& os, const Mod_p& b);
	friend istream& operator>>(istream& is,  Mod_p& b);

	//Returns the inverse of an element
	Mod_p inv();
	static Mod_p inv(const Mod_p& el);
	static void inv(Mod_p& a, const Mod_p& el);

	//multiplication and exponentiation functions
	static void mult(Mod_p& a,const Mod_p& b , const Mod_p& c);
	Mod_p expo(const long e);
	Mod_p expo(const ZZ e);
	static void expo(Mod_p& a,const Mod_p& b, const long e);
	static void expo(Mod_p& a, const Mod_p& b,const ZZ e);
	static Mod_p expo( Mod_p& a, long e);
	static Mod_p expo( Mod_p& a,  ZZ e);


};

#endif /* MOD_P_H_ */
