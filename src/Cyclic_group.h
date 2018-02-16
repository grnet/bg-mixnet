/*
 * Cyclic_group.h
 *
 *  Created on: 08.09.2010
 *      Author: stephaniebayer
 *
 *      Class represent a cyclic group with order o and generator g
 *
 */

#ifndef CYCLIC_GROUP_H_
#define CYCLIC_GROUP_H_

#include "G_mem.h"
#include <NTL/ZZ.h>
NTL_CLIENT



using namespace std;

class Cyclic_group {
private:
	ZZ o; //order of the group
	G_mem g; //generator of the group
public:
	//constructors & destructor
	Cyclic_group();
	Cyclic_group(ZZ ord, G_mem gen);
	virtual ~Cyclic_group();
	//Set parameters
	void set_order(ZZ ord);
	void set_generator(G_mem gen);
	//Access to the parameters
	ZZ get_ord()const;
	G_mem get_gen() const;
	//returns the identity
	G_mem identity();
	//returns a n-th root of unity
	G_mem n_rou(int n);
	G_mem n_rou(ZZ n);
};



#endif /* CYCLIC_GROUP_H_ */
