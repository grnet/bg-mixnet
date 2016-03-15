/*
 * Permutation.h
 *
 *  Created on: 22.10.2010
 *      Author: stephaniebayer
 *
 *      Class permutation contains a function which creates a random permutation to the values 1 to N and one which
 *      represent a permutation as a matrix containing the position of the values
 *
 */



#ifndef PERMUTATION_H_
#define PERMUTATION_H_
#include<vector>
#include "G_q.h"
#include "FakeZZ.h"
NTL_CLIENT

#include "Mod_p.h"

class Permutation {
public:
	Permutation();
	virtual ~Permutation();

//creates permutation of length N and returns is as a vector
static	vector<long>* permutation(long N);
//creates a permutation of a mxn matrix
static	void perm_matrix(vector<vector<vector<long>* >* >* pi,long n, long m);
};

#endif /* PERMUTATION_H_ */
