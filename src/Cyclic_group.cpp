/*
 * Cyclic_group.cpp
 *
 *  Created on: 08.09.2010
 *      Author: stephaniebayer
 */

#include "Cyclic_group.h"
#include <iostream>

#include <NTL/ZZ.h>
NTL_CLIENT


Cyclic_group::Cyclic_group() {
	// TODO Auto-generated constructor stub

}

Cyclic_group::~Cyclic_group() {
	// TODO Auto-generated destructor stub
}

//generates a group for given order and generator
Cyclic_group::Cyclic_group(ZZ ord, G_mem gen){
	o = ord;
	g = gen;
}

// sets the order of a group to ord
void Cyclic_group::set_order(ZZ ord){

	o = ord;
}

// sets the generator of a group to gen
void Cyclic_group::set_generator(G_mem gen){

	g = gen;
}

//returns the value of the order
ZZ Cyclic_group::get_ord()const{

	return o;
}

//returns the generator
G_mem Cyclic_group::get_gen()const{

	return g;
}

//returns the identity
G_mem Cyclic_group::identity(){

	return G_mem(1);
}

//returns the n's root off unity
G_mem Cyclic_group::n_rou(int n){
	cout << "Return of the n's root of unity" <<endl;
	return G_mem(1);
}

//returns the n's root off unity
G_mem Cyclic_group::n_rou(ZZ n){
	cout << "Return of the n's root of unity" <<endl;
	return G_mem(to_ZZ(1));
}
