/*
 * Permutation.cpp
 *
 *  Created on: 22.10.2010
 *      Author: stephaniebayer
 */

#include "Permutation.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>
#include "FakeZZ.h"
NTL_CLIENT


Permutation::Permutation() {
	// TODO Auto-generated constructor stub

}

Permutation::~Permutation() {
	// TODO Auto-generated destructor stub
}

//creates permuation of the numbers 1 to N as a vector
vector<long>* Permutation::permutation(long N){

	vector<long>* v = new vector<long >(N);
	long i, r, temp;
	//vector containing the values 1 to N ordered
	for (i=0; i<N; i++){
		v->at(i)= i+1;
	}

	//create N times a random number <N, calculates r = i+r%N and switchs the values v[i] and v[r]
	for (i=0; i<N; i++){
		r = RandomBnd(N);
		temp = (*v)[i];
		r=(i+r)%N;
		v->at(i)=v->at(r);
		v->at(r)=temp;
	}

	/*
	string name = "permu.txt";
	ofstream ost;
	ost.open(name.c_str());
	for(i=0; i<N; i++){
		ost<<v->at(i)<<" ";
	}
	ost.close();
	*/
	return v;

}

/* calculate a mxn matrix containing a permutation from N = n*m values. For each value in the vector it calculates the
 * associated position in a matrix and saves this.
 * For example N 0 15, m=3, n = 5, if the first value in the permutation vector is 7, then the element in the matrix,
 *  the position (0,0) would be (1,1)*/
void Permutation::perm_matrix(vector<vector<vector<long>* >* >*  pi, long n, long m){

	vector<long>* v=0;
	long i,j,k,t_1,t_2;
	vector<vector<long>* >* r=0;
	vector<long>* el= 0;
	//generates random permutation
	v = permutation(n*m);
	for (i=0; i< m; i++){

		r=new vector<vector<long>* >(n);
		for (j=0; j<n; j++){

			k= i*n +j;
			t_1 = v->at(k)/n;

			t_2 = v->at(k)%n;
			if (t_2 == 0)
			{
				t_1 = t_1 -1;
				t_2 = n-1;
			}
			else
			{
				t_2 = t_2-1;
			}
			el=new vector<long>(2);
			el->at(0)=t_1;
			el->at(1)= t_2;
			r->at(j)=el;
		}
		pi->at(i)=r;
	}

	delete v;
}


