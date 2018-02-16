/*
 * G_mem.cpp
 *
 *  Created on: 08.09.2010
 *      Author: stephaniebayer
 */

#include "G_mem.h"
#include <iostream>

#include <NTL/ZZ.h>
NTL_CLIENT


//Constructos
G_mem::G_mem() {
	// TODO Auto-generated constructor stub

}

G_mem::G_mem(ZZ x){

	val=x;
}

G_mem::G_mem(long x){

	val=to_ZZ(x);
}


ZZ G_mem::value()const{

	return val;
}

//Destructor
G_mem::~G_mem() {
	// TODO Auto-generated destructor stub
}

//Returns the invers of the element
G_mem G_mem::inv(){
	cout << "Returns the invers of the group element" << endl;
	if (val==to_ZZ(0))
		return G_mem(0);
	else
		return G_mem(-1);
}

//General operator, does nothing
G_mem G_mem::op(const G_mem& x, const G_mem& y){
	cout << "Group operation between the element x and y " << endl;
	return G_mem(1);
}

//Equality Check
bool G_mem::operator==(const G_mem& y) const{

	if (val == y.value())
		return true;
	else
		return false;
}



