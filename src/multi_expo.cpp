/*
 * multi_expo.cpp
 *
 *  Created on: 02.07.2012
 *      Author: stephaniebayer
 */

#include "multi_expo.h"
#include "G_q.h"
#include "ElGammal.h";
#include "Pedersen.h"
#include "Cipher_elg.h"
#include <NTL/ZZ.h>
NTL_CLIENT
#include<vector>
using namespace std;
#include <iostream>
#include <time.h>
#include <fstream>

extern G_q G;
extern G_q H;
extern ElGammal El;
extern Pedersen Ped;

multi_expo::multi_expo() {
	// TODO Auto-generated constructor stub

}

multi_expo::~multi_expo() {
	// TODO Auto-generated destructor stub
}


vector<vector<int>*>* multi_expo::to_binary(int win){

	vector<vector<int>* >* ret;
	vector<int>* temp;
	long e,i,j;
	double two= 2;

	e = pow(two, win);
	ret = new vector<vector<int>* >(e);
	for (i = 0; i<e; i++){
		temp = new vector<int>(win);
		for (j=0; j<win; j++){
			temp->at(j) =bit(i,j);
		}
		ret->at(i)=temp;
	}
	return ret;

}

long multi_expo::to_long(vector<int>* bit_r){

	long  t, length;
	double two,i;
	two = 2;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*pow(two,i);
	}
	return t;
}

void multi_expo::to_long(long& t,vector<int>* bit_r){

	long  length;
	double two,i;
	two = 2;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*pow(two,i);
	}
}

vector<long>* multi_expo::to_basis(ZZ e, long num_b, int omega){
	long i, j, l, t;
	vector<int>* bit_r;
	vector<long>* basis;
	bit_r = new vector<int>(omega);
	t= num_b/omega +1;
	basis = new vector<long>(t);

	j=0;
	l=0;

	for(i=0; i<num_b; i++){
		bit_r->at(j)=bit(e,i);
		j++;
		if(j==omega){
			to_long(basis->at(l),bit_r);
			j=0;
			l++;
		}
		else if(i == num_b-1){
			for(j = j; j<omega; j++){
				bit_r->at(j)= 0;
				to_long(basis->at(l),bit_r);
			}
		}
	}
	delete bit_r;
	return basis;

}


vector<vector<vector<long>* >* >* multi_expo::to_basis_vec(vector<vector<ZZ>* >* a, long num_b, int omega){
	vector<vector<vector<long>* >* >* basis_vec=0;
	vector<vector<long>* >* basis_row=0;
	vector<long>* basis = 0;
	long i, j, m,n;
	m=a->size();
	n=a->at(0)->size();
	basis_vec = new vector<vector<vector<long>* >* >(m+1);

	for (i = 0; i<m; i++){

		basis_row = new vector<vector<long>* >(n);
		for (j = 0; j<n; j++){
			basis = to_basis(a->at(i)->at(j), num_b, omega);
			basis_row->at(j)= basis;
		}
		basis_vec->at(i+1) = basis_row;
	}
	delete basis;
	return basis_vec;
}

ZZ multi_expo::expo_mult(const vector<ZZ>* e, ZZ ran, int omega_expo, vector<Mod_p>* gen){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	long length;// num_b;
	ZZ prod, p, temp_1, temp_2, mod;
	double two;
	long num_b;
	length = e->size();
	mod = G.get_mod();
	num_b = NumBits(G.get_ord());
	l = num_b/omega_expo +1;
	basis_vec = new vector<vector<long>* >(length+1);
	basis_vec->at(0) = to_basis(ran, num_b, omega_expo);
	for(i = 0; i<length; i++){
		basis_vec->at(i+1) = to_basis(e->at(i), num_b, omega_expo);
	}
	prod = 1;
	two = 2;
	p=1;
	t = pow(two, omega_expo)-1;
	for(i=l-1; i>0; i--){

		p=1;
		for(j = 0; j<length+1; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,gen->at(j).get_val(),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p=1;
			for(j = 0; j<length+1; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,gen->at(j).get_val(),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		MulMod(prod , prod,temp_2,mod);
		for(k =0; k<omega_expo; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p=1;
	for(j = 0; j<length+1; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,gen->at(j).get_val(),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p=1;
		for(j = 0; j<length+1; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,gen->at(j).get_val(),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod,prod,temp_2,mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
	return prod;
}


void multi_expo::expo_mult(ZZ& prod, const vector<ZZ>* e, ZZ ran, int omega_expo, vector<Mod_p>* gen){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	long length;// num_b;
	ZZ p, temp_1, temp_2, mod;
	double two;
	long num_b;
	length = e->size();
	mod = G.get_mod();
	num_b = NumBits(G.get_ord());
	l = num_b/omega_expo +1;
	basis_vec = new vector<vector<long>* >(length+1);
	basis_vec->at(0) = to_basis(ran, num_b, omega_expo);
	for(i = 0; i<length; i++){
		basis_vec->at(i+1) = to_basis(e->at(i), num_b, omega_expo);
	}
	prod = 1;
	two = 2;
	p=1;
	t = pow(two, omega_expo)-1;
	length = length +1;
	for(i=l-1; i>0; i--){

		p=1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,gen->at(j).get_val(),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p=1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,gen->at(j).get_val(),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		MulMod(prod , prod,temp_2,mod);
		for(k =0; k<omega_expo; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p=1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,gen->at(j).get_val(),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p=1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,gen->at(j).get_val(),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod,prod,temp_2,mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
}



Cipher_elg multi_expo::expo_mult(const vector<Cipher_elg>* a, vector<ZZ>* e, int omega ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	long length;
	ZZ prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v, mod;
	Cipher_elg prod;
	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	vector<ZZ> a_u(length);
	vector<ZZ> a_v(length);
	basis_vec = new vector<vector<long>* >(length);
	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i), num_b,omega);
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = 1;
	prod_v= 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p_u= 1;
		p_v= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j), mod);
			}
		}
		temp_1_u = p_u;
		temp_2_u = p_u;
		temp_1_v = p_v;
		temp_2_v = p_v;
		for(k = t-1; k>0; k--){
			p_u= 1;
			p_v = 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p_u,p_u,a_u.at(j),mod);
					MulMod(p_v,p_v,a_v.at(j),mod);
				}
			}
			MulMod(temp_1_u,temp_1_u,p_u,mod);
			MulMod(temp_2_u,temp_1_u,temp_2_u,mod);
			MulMod(temp_1_v, temp_1_v,p_v,mod);
			MulMod(temp_2_v,temp_1_v,temp_2_v,mod);

		}
		MulMod(prod_u,prod_u,temp_2_u,mod);
		MulMod(prod_v,prod_v,temp_2_v,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod_u,prod_u,mod);
			SqrMod(prod_v,prod_v,mod);
		}
	}
	p_u= 1;
	p_v= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p_u,p_u,a_u.at(j),mod);
			MulMod(p_v,p_v,a_v.at(j),mod);
		}
	}
	temp_1_u = p_u;
	temp_2_u = p_u;
	temp_1_v = p_v;
	temp_2_v = p_v;
	for(k = t-1; k>0; k--){
		p_u= 1;
		p_v = 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j),mod);
			}
		}
		MulMod(temp_1_u,temp_1_u,p_u,mod);
		MulMod(temp_2_u,temp_1_u,temp_2_u,mod);
		MulMod(temp_1_v,temp_1_v,p_v,mod);
		MulMod(temp_2_v,temp_1_v,temp_2_v,mod);
	}
	MulMod(prod_u,prod_u,temp_2_u,mod);
	MulMod(prod_v,prod_v,temp_2_v,mod);
	prod = Cipher_elg(prod_u, prod_v, mod);

	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
	return prod;
}

void multi_expo::expo_mult(Cipher_elg& prod, const vector<Cipher_elg>* a, vector<ZZ>* e, int omega ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	ZZ ord = H.get_ord();
	long length;
	ZZ prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v, mod,temp;
	double two;
	long num_b;

	length = a->size();
	mod = H.get_mod();
	num_b = NumBits(ord);
	l = num_b/omega +1;

	vector<ZZ> a_u(length);
	vector<ZZ> a_v(length);
	basis_vec = new vector<vector<long>* >(length);
	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i), num_b,omega);
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = 1;
	prod_v= 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p_u= 1;
		p_v= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j), mod);
			}
		}
		temp_1_u = p_u;
		temp_2_u = p_u;
		temp_1_v = p_v;
		temp_2_v = p_v;
		for(k = t-1; k>0; k--){
			p_u= 1;
			p_v = 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p_u,p_u,a_u.at(j),mod);
					MulMod(p_v,p_v,a_v.at(j),mod);
				}
			}
			MulMod(temp_1_u ,temp_1_u,p_u,mod);
			MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
			MulMod(temp_1_v ,temp_1_v,p_v,mod);
			MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);

		}
		prod_u = MulMod(prod_u,temp_2_u,mod);
		prod_v = MulMod(prod_v,temp_2_v,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod_u,prod_u,mod);
			SqrMod(prod_v,prod_v,mod);
		}
	}
	p_u= 1;
	p_v= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p_u,p_u,a_u.at(j),mod);
			MulMod(p_v,p_v,a_v.at(j),mod);
		}
	}
	temp_1_u = p_u;
	temp_2_u = p_u;
	temp_1_v = p_v;
	temp_2_v = p_v;
	for(k = t-1; k>0; k--){
		p_u= 1;
		p_v = 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j),mod);
			}
		}
		MulMod(temp_1_u ,temp_1_u,p_u,mod);
		MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
		MulMod(temp_1_v, temp_1_v,p_v,mod);
		MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);
	}
	MulMod(prod_u,prod_u,temp_2_u,mod);
	MulMod(prod_v,prod_v,temp_2_v,mod);
	prod = Cipher_elg(prod_u, prod_v, mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
}

ZZ multi_expo::expo_mult(const vector<ZZ>* a, vector<vector<ZZ>*>* e, int omega, long pos ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;

	long length;
	ZZ prod, p, temp_1, temp_2, mod;

	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	basis_vec = new vector<vector<long>* >(length);

	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i)->at(pos), num_b,omega);
	}
	prod = 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,a->at(j),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p= 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,a->at(j),mod);
				}
			}
			temp_1 = MulMod(temp_1,p,mod);
			temp_2 = MulMod(temp_1,temp_2,mod);

		}
		prod = MulMod(prod,temp_2,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,a->at(j),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,a->at(j),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod,prod,temp_2,mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
	return prod;
}

void multi_expo::expo_mult(ZZ& prod, const vector<ZZ>* a, vector<vector<ZZ>*>* e, int omega, long pos ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;

	long length;
	ZZ  p, temp_1, temp_2, mod;

	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	basis_vec = new vector<vector<long>* >(length);

	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i)->at(pos), num_b,omega);
	}
	prod = 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,a->at(j),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p= 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,a->at(j),mod);
				}
			}
			temp_1 = MulMod(temp_1,p,mod);
			temp_2 = MulMod(temp_1,temp_2,mod);

		}
		prod = MulMod(prod,temp_2,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,a->at(j),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,a->at(j),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod,prod,temp_2,mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
}

ZZ multi_expo::expo_mult(const vector<vector<vector<ZZ>* >*>* a, vector<vector<ZZ>*>* e, int omega, long pos, long pos_2 ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;

	long length;
	ZZ prod, p, temp_1, temp_2, mod;

	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;
	basis_vec = new vector<vector<long>* >(length);

	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i)->at(pos), num_b,omega);
	}

	prod = 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p= 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		MulMod(prod,prod,temp_2,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod,prod,temp_2,mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
	return prod;
}

void multi_expo::expo_mult(ZZ& prod, const vector<vector<vector<ZZ>* >*>* a, vector<vector<ZZ>*>* e, int omega, long pos, long pos_2 ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;

	long length;
	ZZ  p, temp_1, temp_2, mod;

	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;
	basis_vec = new vector<vector<long>* >(length);

	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i)->at(pos), num_b,omega);
	}

	prod = 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p= 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		MulMod(prod,prod,temp_2,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,a->at(j)->at(pos_2)->at(pos),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod,prod,temp_2,mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
}

Cipher_elg multi_expo::expo_mult(const vector<Cipher_elg>* a, vector<vector<long>*>* basis_vec, int omega ){
	long i, j, k, l,t;
	long length;
	ZZ prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v, mod;
	Cipher_elg prod;
	double two;
	long num_b;

	length = a->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	vector<ZZ> a_u(length);
	vector<ZZ> a_v(length);

	for(i = 0; i<length; i++){
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = 1;
	prod_v= 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p_u= 1;
		p_v= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j), mod);
			}
		}
		temp_1_u = p_u;
		temp_2_u = p_u;
		temp_1_v = p_v;
		temp_2_v = p_v;
		for(k = t-1; k>0; k--){
			p_u= 1;
			p_v = 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p_u,p_u,a_u.at(j),mod);
					MulMod(p_v,p_v,a_v.at(j),mod);
				}
			}
			MulMod(temp_1_u ,temp_1_u,p_u,mod);
			MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
			MulMod(temp_1_v,temp_1_v,p_v,mod);
			MulMod(temp_2_v, temp_1_v,temp_2_v,mod);

		}
		MulMod(prod_u,prod_u,temp_2_u,mod);
		MulMod(prod_v,prod_v,temp_2_v,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod_u,prod_u,mod);
			SqrMod(prod_v,prod_v,mod);
		}
	}
	p_u= 1;
	p_v= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p_u,p_u,a_u.at(j),mod);
			MulMod(p_v,p_v,a_v.at(j),mod);
		}
	}
	temp_1_u = p_u;
	temp_2_u = p_u;
	temp_1_v = p_v;
	temp_2_v = p_v;
	for(k = t-1; k>0; k--){
		p_u= 1;
		p_v = 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j),mod);
			}
		}
		MulMod(temp_1_u ,temp_1_u,p_u,mod);
		MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
		MulMod(temp_1_v, temp_1_v,p_v,mod);
		MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);
	}
	MulMod(prod_u, prod_u,temp_2_u,mod);
	MulMod(prod_v,prod_v,temp_2_v,mod);
	prod = Cipher_elg(prod_u, prod_v, mod);

	return prod;
}

void multi_expo::expo_mult(Cipher_elg& prod, const vector<Cipher_elg>* a, vector<vector<long>*>* basis_vec, int omega ){
	long i, j, k, l,t;
	long length;
	ZZ prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v, mod;
	double two;
	long num_b;

	length = a->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	vector<ZZ> a_u(length);
	vector<ZZ> a_v(length);

	for(i = 0; i<length; i++){
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = 1;
	prod_v= 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p_u= 1;
		p_v= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j), mod);
			}
		}
		temp_1_u = p_u;
		temp_2_u = p_u;
		temp_1_v = p_v;
		temp_2_v = p_v;
		for(k = t-1; k>0; k--){
			p_u= 1;
			p_v = 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p_u,p_u,a_u.at(j),mod);
					MulMod(p_v,p_v,a_v.at(j),mod);
				}
			}
			MulMod(temp_1_u ,temp_1_u,p_u,mod);
			MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
			MulMod(temp_1_v,temp_1_v,p_v,mod);
			MulMod(temp_2_v, temp_1_v,temp_2_v,mod);

		}
		MulMod(prod_u,prod_u,temp_2_u,mod);
		MulMod(prod_v,prod_v,temp_2_v,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod_u,prod_u,mod);
			SqrMod(prod_v,prod_v,mod);
		}
	}
	p_u= 1;
	p_v= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p_u,p_u,a_u.at(j),mod);
			MulMod(p_v,p_v,a_v.at(j),mod);
		}
	}
	temp_1_u = p_u;
	temp_2_u = p_u;
	temp_1_v = p_v;
	temp_2_v = p_v;
	for(k = t-1; k>0; k--){
		p_u= 1;
		p_v = 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j),mod);
			}
		}
		MulMod(temp_1_u ,temp_1_u,p_u,mod);
		MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
		MulMod(temp_1_v, temp_1_v,p_v,mod);
		MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);
	}
	MulMod(prod_u, prod_u,temp_2_u,mod);
	MulMod(prod_v,prod_v,temp_2_v,mod);
	prod = Cipher_elg(prod_u, prod_v, mod);

}


Cipher_elg multi_expo::expo_mult(const vector<Cipher_elg>* a, ZZ f, vector<ZZ>* e, int omega ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	ZZ ord = H.get_ord();
	long length;
	ZZ prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v, mod,temp;
	Cipher_elg prod;
	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(ord);
	l = num_b/omega +1;

	vector<ZZ> a_u(length);
	vector<ZZ> a_v(length);
	basis_vec = new vector<vector<long>* >(length);
	for(i = 0; i<length; i++){
		temp = MulMod(f,e->at(i),ord);
		basis_vec->at(i) = to_basis(temp, num_b,omega);
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = 1;
	prod_v= 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p_u= 1;
		p_v= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j), mod);
			}
		}
		temp_1_u = p_u;
		temp_2_u = p_u;
		temp_1_v = p_v;
		temp_2_v = p_v;
		for(k = t-1; k>0; k--){
			p_u= 1;
			p_v = 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p_u,p_u,a_u.at(j),mod);
					MulMod(p_v,p_v,a_v.at(j),mod);
				}
			}
			MulMod(temp_1_u ,temp_1_u,p_u,mod);
			MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
			MulMod(temp_1_v ,temp_1_v,p_v,mod);
			MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);

		}
		prod_u = MulMod(prod_u,temp_2_u,mod);
		prod_v = MulMod(prod_v,temp_2_v,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod_u,prod_u,mod);
			SqrMod(prod_v,prod_v,mod);
		}
	}
	p_u= 1;
	p_v= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p_u,p_u,a_u.at(j),mod);
			MulMod(p_v,p_v,a_v.at(j),mod);
		}
	}
	temp_1_u = p_u;
	temp_2_u = p_u;
	temp_1_v = p_v;
	temp_2_v = p_v;
	for(k = t-1; k>0; k--){
		p_u= 1;
		p_v = 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j),mod);
			}
		}
		MulMod(temp_1_u ,temp_1_u,p_u,mod);
		MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
		MulMod(temp_1_v, temp_1_v,p_v,mod);
		MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);
	}
	MulMod(prod_u,prod_u,temp_2_u,mod);
	MulMod(prod_v,prod_v,temp_2_v,mod);
	prod = Cipher_elg(prod_u, prod_v, mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	}
	delete basis_vec;
	return prod;
}

void multi_expo::expo_mult(Cipher_elg& prod, const vector<Cipher_elg>* a, ZZ f, vector<ZZ>* e, int omega ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	ZZ ord = H.get_ord();
	long length;
	ZZ prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v, mod,temp;
	double two;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(ord);
	l = num_b/omega +1;
	vector<ZZ> a_u(length);
	vector<ZZ> a_v(length);
	basis_vec = new vector<vector<long>* >(length);
	for(i = 0; i<length; i++){
		temp = MulMod(f,e->at(i),ord);
		basis_vec->at(i) = to_basis(temp, num_b,omega);
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = 1;
	prod_v= 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p_u= 1;
		p_v= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j), mod);
			}
		}
		temp_1_u = p_u;
		temp_2_u = p_u;
		temp_1_v = p_v;
		temp_2_v = p_v;
		for(k = t-1; k>0; k--){
			p_u= 1;
			p_v = 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p_u,p_u,a_u.at(j),mod);
					MulMod(p_v,p_v,a_v.at(j),mod);
				}
			}
			MulMod(temp_1_u ,temp_1_u,p_u,mod);
			MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
			MulMod(temp_1_v ,temp_1_v,p_v,mod);
			MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);

		}
		prod_u = MulMod(prod_u,temp_2_u,mod);
		prod_v = MulMod(prod_v,temp_2_v,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod_u,prod_u,mod);
			SqrMod(prod_v,prod_v,mod);
		}
	}
	p_u= 1;
	p_v= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p_u,p_u,a_u.at(j),mod);
			MulMod(p_v,p_v,a_v.at(j),mod);
		}
	}
	temp_1_u = p_u;
	temp_2_u = p_u;
	temp_1_v = p_v;
	temp_2_v = p_v;
	for(k = t-1; k>0; k--){
		p_u= 1;
		p_v = 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p_u,p_u,a_u.at(j),mod);
				MulMod(p_v,p_v,a_v.at(j),mod);
			}
		}
		MulMod(temp_1_u ,temp_1_u,p_u,mod);
		MulMod(temp_2_u ,temp_1_u,temp_2_u,mod);
		MulMod(temp_1_v, temp_1_v,p_v,mod);
		MulMod(temp_2_v ,temp_1_v,temp_2_v,mod);
	}
	MulMod(prod_u,prod_u,temp_2_u,mod);
	MulMod(prod_v,prod_v,temp_2_v,mod);
	prod = Cipher_elg(prod_u, prod_v, mod);
	j = basis_vec->size();
	for(i=0; i<j; i++){
		delete basis_vec->at(i);
		basis_vec->at(i)=0;
	};
	delete basis_vec;
}

Cipher_elg multi_expo::expo_mult(const vector<vector<Cipher_elg>*>* a, vector<ZZ>* s1, vector<ZZ>* s2, int omega ){
	Cipher_elg prod;
	long i,l;
	l = a->size();
	prod = expo_mult(a->at(0), s1->at(0), s2, omega);
	for(i = 1; i<l; i++){
		prod = prod*expo_mult(a->at(i), s1->at(i), s2, omega);
	}
	return prod;

}


void  multi_expo::expo_mult(Cipher_elg& prod, const vector<vector<Cipher_elg>*>* a, vector<ZZ>* s1, vector<ZZ>* s2, int omega ){
	Cipher_elg temp;
	long i,l;
	l = a->size();
	expo_mult(prod,a->at(0), s1->at(0), s2, omega);
	for(i = 1; i<l; i++){
		expo_mult(temp,a->at(i), s1->at(i), s2, omega);
		Cipher_elg::mult(prod,prod,temp);
	}
}

Mod_p multi_expo::expo_mult(const vector<Mod_p>* a, vector<vector<long>*>* basis_vec, int omega ){
	long i, j, k, l,t;

	long length;
	ZZ prod, p, temp_1, temp_2, mod;
	Mod_p pro;
	double two;
	long num_b;

	length = a->size();
	mod = G.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	vector<ZZ> a_temp(length);

	for(i = 0; i<length; i++){
		a_temp.at(i)=a->at(i).get_val();
	}
	prod = 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,a_temp.at(j),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p= 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,a_temp.at(j),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		prod = MulMod(prod,temp_2,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,a_temp.at(j),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,a_temp.at(j),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod, prod,temp_2,mod);
	pro = Mod_p(prod,  mod);
	return pro;
}

void multi_expo::expo_mult(Mod_p& pro, const vector<Mod_p>* a, vector<vector<long>*>* basis_vec, int omega ){
	long i, j, k, l,t;
	long length;
	ZZ prod, p, temp_1, temp_2, mod;
	//Mod_p pro;
	double two;
	long num_b;

	length = a->size();
	mod = G.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	vector<ZZ> a_temp(length);

	for(i = 0; i<length; i++){
		a_temp.at(i)=a->at(i).get_val();
	}
	prod = 1;
	two = 2;
	t = pow(two, omega)-1;
	for(i=l-1; i>0; i--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,a_temp.at(j),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p= 1;
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,a_temp.at(j),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		prod = MulMod(prod,temp_2,mod);
		for(k =0; k<omega; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p= 1;
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,a_temp.at(j),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p= 1;
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(0)==k){
				MulMod(p,p,a_temp.at(j),mod);
			}
		}
		MulMod(temp_1,temp_1,p,mod);
		MulMod(temp_2,temp_1,temp_2,mod);
	}
	MulMod(prod, prod,temp_2,mod);
	pro = Mod_p(prod,  mod);
}


vector<vector<ZZ>* >* multi_expo::calc_Yk(vector<ZZ>* y, int win){
	vector<vector<ZZ>* >* ret;
	vector<ZZ>* temp;
	long h,t, i,j,k,e;
	double two=2;
	ZZ prod, mod, tem;
	mod = H.get_mod();
	vector<vector<int>* >* binary;

	h= y->size()/win;
	e = pow(two, win);
	binary = to_binary(win);

	if((unsigned) h*win ==y->size()){
		ret = new vector<vector<ZZ>* >(h);
		for(i =0; i<h; i++){
			temp = new vector<ZZ>(e);
			for(j=0; j<e; j++){
				prod = 1;
				for(k=0; k<win; k++){
					PowerMod(tem,y->at(i*win+k), binary->at(j)->at(k), mod);
					MulMod(prod,prod,tem,mod);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
	}
	else{
		ret = new vector<vector<ZZ>* >(h+1);
		t= y->size()-h*win;
		for(i =0; i<h; i++){
			temp = new vector<ZZ>(e);
			for(j=0; j<e; j++){
				prod = 1;
				for(k=0; k<win; k++){
					PowerMod(tem,y->at(i*win+k), binary->at(j)->at(k), mod);
					MulMod(prod,prod,tem,mod);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
		temp = new vector<ZZ>(e);
		for(j=0; j<e; j++){
			prod = 1;
			for(k=0; k<t; k++){
				PowerMod(tem,y->at(h*win+k), binary->at(j)->at(k), mod);
				MulMod(prod,prod,tem,mod);
			}
			temp->at(j)=prod;
		}
		ret->at(h)=temp;

	}
	for (i = 0; i<e; i++){
		delete binary->at(i);
		binary->at(i)= 0;
	}
	delete binary;
	return ret;

}

vector<vector<Mod_p>* >* multi_expo::calc_Yk(vector<Mod_p>* y, int win){
	vector<vector<Mod_p>* >* ret;
	vector<Mod_p>* temp;
	long h,t, i,j,k,e;
	double two=2;
	Mod_p prod, tem;
	vector<vector<int>* >* binary;

	h= y->size()/win;
	e = pow(two, win);
	binary = to_binary(win);

	if((unsigned)h*win == y->size()){
		ret = new vector<vector<Mod_p>* >(h);
		for(i =0; i<h; i++){
			temp = new vector<Mod_p>(e);
			for(j=0; j<e; j++){
				prod = Mod_p(1,G.get_mod());
				for(k=0; k<win; k++){
					Mod_p::expo(tem,y->at(i*win+k), binary->at(j)->at(k));
					Mod_p::mult(prod,prod,tem);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
	}
	else{
		ret = new vector<vector<Mod_p>* >(h+1);
		t=y->size()-h*win;
		for(i =0; i<h; i++){
			temp = new vector<Mod_p>(e);
			for(j=0; j<e; j++){
				prod = Mod_p(1,G.get_mod());
				for(k=0; k<win; k++){
					Mod_p::expo(tem,y->at(i*win+k), binary->at(j)->at(k));
					Mod_p::mult(prod,prod, tem);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
		temp = new vector<Mod_p>(e);
		for(j=0; j<e; j++){
			prod =  Mod_p(1,G.get_mod());;
			for(k=0; k<t; k++){
				Mod_p::expo(tem,y->at(h*win+k), binary->at(j)->at(k));
				Mod_p::mult(prod,prod,tem);
			}
			temp->at(j)=prod;
		}
		ret->at(h)=temp;

	}
	j = binary->size();
	for(i=0; i<j; i++){
		delete binary->at(i);
		binary->at(i)=0;
	}
	for (i = 0; i<e; i++){
		delete binary->at(i);
		binary->at(i)= 0;
	}
	delete binary;
	return ret;

}
vector<vector<ZZ>* >* multi_expo::calc_Yk(vector<vector<vector<ZZ>*>*>* y, int win, long pos, long pos_2){
	vector<vector<ZZ>* >* ret;
	vector<ZZ>* temp;
	long h,t, i,j,k,e;
	double two=2;
	ZZ prod, mod;
	mod = H.get_mod();
	vector<vector<int>* >* binary;

	h= y->size()/win;
	e = pow(two, win);
	binary = to_binary(win);

	if((unsigned)h*win ==y->size()){
		ret = new vector<vector<ZZ>* >(h);
		for(i =0; i<h; i++){
			temp = new vector<ZZ>(e);
			for(j=0; j<e; j++){
				prod = 1;
				for(k=0; k<win; k++){
					MulMod(prod,prod,PowerMod(y->at(i*win+k)->at(pos_2)->at(pos), binary->at(j)->at(k), mod),mod);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
	}
	else{
		ret = new vector<vector<ZZ>*>(h+1);
		t= y->size()-h*win;
		for(i =0; i<h; i++){
			temp = new vector<ZZ>(e);
			for(j=0; j<e; j++){
				prod = 1;
				for(k=0; k<win; k++){
					MulMod(prod,prod,PowerMod(y->at(i*win+k)->at(pos_2)->at(pos), binary->at(j)->at(k), mod),mod);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
		temp = new vector<ZZ>(e);
		for(j=0; j<e; j++){
			prod = 1;
			for(k=0; k<t; k++){
				MulMod(prod,prod,PowerMod(y->at(h*win+k)->at(pos_2)->at(pos), binary->at(j)->at(k), mod),mod);
			}
			temp->at(j)=prod;
		}
		ret->at(h)=temp;

	}
	for (i = 0; i<e; i++){
			delete binary->at(i);
			binary->at(i)= 0;
		}
	delete binary;
	return ret;

}



ZZ multi_expo::multi_expo_LL(vector<ZZ>* y, vector<vector<ZZ>*>* e, int win, long pos){
	vector<vector<ZZ>*>* Yk;
	ZZ ret;
	double two = 2;
	int expo, tem, i,j,k,h;

	ZZ mod = H.get_mod();
	long t = NumBits(H.get_ord());
	h= y->size()/win;
	Yk = calc_Yk(y,win);
	ret = 1;

	if((unsigned)h*win == y->size()){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		for(k=t-2;k>=0; k--){
			SqrMod(ret, ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
		}
	}
	else{
		tem= y->size()-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-h*win);
		}
		MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		for(k=t-2;k>=0; k--){
			SqrMod(ret, ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-h*win);
			}
			MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		}
	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;
	return ret;

}

void multi_expo::multi_expo_LL(ZZ& ret, vector<ZZ>* y, vector<vector<ZZ>*>* e, int win, long pos){
	vector<vector<ZZ>*>* Yk;
	//ZZ ret;
	double two = 2;
	int expo, tem, i,j,k,h;

	ZZ mod = H.get_mod();
	long t = NumBits(H.get_ord());
	h= y->size()/win;
	Yk = calc_Yk(y,win);
	ret = 1;

	if((unsigned)h*win == y->size()){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		for(k=t-2;k>=0; k--){
			SqrMod(ret, ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
		}
	}
	else{
		tem= y->size()-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-h*win);
		}
		MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		for(k=t-2;k>=0; k--){
			SqrMod(ret, ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-h*win);
			}
			MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		}
	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;

}

ZZ multi_expo::multi_expo_LL(vector<vector<vector<ZZ>*>*>* y, vector<vector<ZZ>*>* e, int win, long pos, long pos_2){
	vector<vector<ZZ>*>* Yk;
	ZZ ret;
	double two = 2;
	int expo, tem, i,j,k,h;

	ZZ mod = H.get_mod();
	long t = NumBits(H.get_ord());
	h= y->size()/win;
	Yk = calc_Yk(y,win, pos, pos_2);

	ret = 1;

	if((unsigned)h*win == y->size()){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		for(k=t-2;k>=0; k--){
			SqrMod(ret,ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret, ret,Yk->at(i)->at(expo),mod);
			}
		}
	}
	else{
		tem= y->size()-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-h*win);
		}
		MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		for(k=t-2;k>=0; k--){
			SqrMod(ret,ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-h*win);
			}
			MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		}
	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;
	return ret;

}

void multi_expo::multi_expo_LL(ZZ& ret, vector<vector<vector<ZZ>*>*>* y, vector<vector<ZZ>*>* e, int win, long pos, long pos_2){
	vector<vector<ZZ>*>* Yk;
	//ZZ ret;
	double two = 2;
	int expo, tem, i,j,k,h;

	ZZ mod = H.get_mod();
	long t = NumBits(H.get_ord());
	h= y->size()/win;
	Yk = calc_Yk(y,win, pos, pos_2);

	ret = 1;

	if((unsigned)h*win == y->size()){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		for(k=t-2;k>=0; k--){
			SqrMod(ret,ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret, ret,Yk->at(i)->at(expo),mod);
			}
		}
	}
	else{
		tem= y->size()-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j)->at(pos),t-1)*pow(two,j-h*win);
		}
		MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		for(k=t-2;k>=0; k--){
			SqrMod(ret,ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo+ bit(e->at(j)->at(pos),k)*pow(two,j-h*win);
			}
			MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		}
	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;
}

Mod_p multi_expo::multi_expo_LL(vector<Mod_p>* y, vector<ZZ>* e, int win){
	vector<vector<Mod_p>*>* Yk;
	Mod_p ret;
	double two = 2;
	int expo, i,j,k,h,t,tem;
	t = NumBits(G.get_ord());

	h= y->size()/win;
	Yk = calc_Yk(y,win);
	ret = Mod_p(1,G.get_mod());
	if((unsigned)win*h==y->size()){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j),t-1)*pow(two,j-i*win);
			}
			Mod_p::mult(ret ,ret,Yk->at(i)->at(expo));
		}

		for(k=t-2;k>=0; k--){
			ret = ret*ret;
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j),k)*pow(two,j-i*win);
				}
				Mod_p::mult(ret ,ret,Yk->at(i)->at(expo));
			}
		}
	}
	else{
		tem=y->size()-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j),t-1)*pow(two,j-i*win);
			}
			Mod_p::mult(ret ,ret,Yk->at(i)->at(expo));
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j),t-1)*pow(two,j-h*win);
		}
		Mod_p::mult(ret ,ret,Yk->at(h)->at(expo));

		for(k=t-2;k>=0; k--){
			ret = ret*ret;
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j),k)*pow(two,j-i*win);
				}
				Mod_p::mult(ret,ret,Yk->at(i)->at(expo));
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo + bit(e->at(j),k)*pow(two,j-h*win);
			}
			Mod_p::mult(ret ,ret,Yk->at(h)->at(expo));
		}

	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;
	return ret;
}


void multi_expo::multi_expo_LL(Mod_p& ret, vector<Mod_p>* y, vector<ZZ>* e, int win){
	vector<vector<Mod_p>*>* Yk;

	double two = 2;
	int expo, i,j,k,h,t,tem;
	long length;

	t = NumBits(G.get_ord());
/*	if(e->size() < y->size()){
		length = e->size();
		h= length/win;
	} else {*/
		length = y->size();
		h = length/win;
	//}
	Yk = calc_Yk(y,win);
	ret = Mod_p(1,G.get_mod());

	if(win*h==length){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j),t-1)*pow(two,j-i*win);
			}
			Mod_p::mult(ret ,ret,Yk->at(i)->at(expo));
		}

		for(k=t-2;k>=0; k--){
			ret = ret*ret;
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j),k)*pow(two,j-i*win);
				}
				Mod_p::mult(ret ,ret,Yk->at(i)->at(expo));
			}
		}
	}
	else{
		tem=length-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j),t-1)*pow(two,j-i*win);
			}
			Mod_p::mult(ret ,ret,Yk->at(i)->at(expo));
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j),t-1)*pow(two,j-h*win);
		}
		Mod_p::mult(ret ,ret,Yk->at(h)->at(expo));

		for(k=t-2;k>=0; k--){
			ret = ret*ret;
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j),k)*pow(two,j-i*win);
				}
				Mod_p::mult(ret,ret,Yk->at(i)->at(expo));
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo + bit(e->at(j),k)*pow(two,j-h*win);
			}
			Mod_p::mult(ret ,ret,Yk->at(h)->at(expo));
		}

	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;
}

void multi_expo::multi_expo_LL(ZZ& ret, vector<ZZ>* y, vector<ZZ>* e, int win){
	vector<vector<ZZ>*>* Yk;

	double two = 2;
	int expo, tem, i,j,k,h;

	ZZ mod = H.get_mod();
	long t = NumBits(H.get_ord());
	h= y->size()/win;
	Yk = calc_Yk(y,win);
	ret = 1;

	if((unsigned)h*win == y->size()){
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		for(k=t-2;k>=0; k--){
			SqrMod(ret, ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
		}
	}
	else{
		tem= y->size()-h*win;
		for(i=0; i<h; i++){
			expo=0;
			for(j=i*win; j<(i+1)*win; j++){
				expo = expo + bit(e->at(j),t-1)*pow(two,j-i*win);
			}
			MulMod(ret,ret,Yk->at(i)->at(expo),mod);
		}
		expo=0;
		for(j=h*win; j<h*win+tem; j++){
			expo = expo + bit(e->at(j),t-1)*pow(two,j-h*win);
		}
		MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		for(k=t-2;k>=0; k--){
			SqrMod(ret, ret,mod);
			for(i=0; i<h; i++){
				expo=0;
				for(j=i*win; j<(i+1)*win; j++){
					expo = expo+ bit(e->at(j),k)*pow(two,j-i*win);
				}
				MulMod(ret,ret,Yk->at(i)->at(expo),mod);
			}
			expo=0;
			for(j=h*win; j<h*win+tem; j++){
				expo = expo+ bit(e->at(j),k)*pow(two,j-h*win);
			}
			MulMod(ret,ret,Yk->at(h)->at(expo),mod);
		}
	}
	j = Yk->size();
	for(i=0; i<j; i++){
		delete Yk->at(i);
		Yk->at(i)=0;
	}
	delete Yk;

}

void multi_expo::multi_expo_LL(Cipher_elg& ret, Cipher_elg c1, Cipher_elg c2, Cipher_elg c3, Cipher_elg c4, vector<ZZ>* e, int win){

	ZZ ret_u, ret_v;
	vector<ZZ>* c_u;
	vector<ZZ>* c_v;

	c_u = new vector<ZZ>(4);
	c_u->at(0) = c1.get_u();
	c_u->at(1) = c2.get_u();
	c_u ->at(2) = c3.get_u();
	c_u->at(3) = c4.get_u();

	c_v = new vector<ZZ>(4);
	c_v->at(0) = c1.get_v();
	c_v->at(1) = c2.get_v();
	c_v ->at(2) = c3.get_v();
	c_v->at(3) = c4.get_v();

	multi_expo_LL(ret_u, c_u, e, win);
	multi_expo_LL(ret_v, c_v, e, win);
	delete c_u;
	delete c_v;
	ret = Cipher_elg(ret_u, ret_v, H.get_mod());


}

vector<int>* multi_expo::to_basis_sw(ZZ e, long num_b, int omega_sw){

	long i, j, t, te;
	vector<int>* temp;
	vector<int>* basis;
	temp = new vector<int>(omega_sw);
	basis = new vector<int>(num_b);

	i = num_b-1;
	while(i>=omega_sw){
		if (bit(e,i)==0){
			basis->at(i)=0;
			i=i-1;
		}
		else{
			for(t= i-omega_sw+1; t < i+1; t++){
				if(bit(e,t)==1){
					te = i-t;
					for(j = 0; j<= te; j++){
						temp->at(j)=bit(e, t+j);
					}
					for(j =te+1; j<omega_sw; j++){
						temp->at(j) =0;
					}
					break;
				}
			}
			basis->at(t)= to_long(temp);
			for(j = t+1; j<=i; j++){
				basis->at(j)=0;
			}
			i=t-1;
		}
	}
	while(i>=0){
		if (bit(e,i)==0){
			basis->at(i)=0;
			i=i-1;
		}
		else{
			for(t= 0; t < i+1; t++){
				if(bit(e,t)==1){
					te = i-t;
					for(j = 0; j<= te; j++){
						temp->at(j)=bit(e, t+j);
					}
					for(j =te+1; j<omega_sw; j++){
						temp->at(j) =0;
					}
					break;
				}
			}
			basis->at(t)= to_long(temp);
			for(j = t+1; j<=i; j++){
				basis->at(j)=0;
			}
			i=t-1;
		}

	}
	delete temp;
	return basis;
}


ZZ multi_expo::multi_expo_sw( ZZ e_1, ZZ e_2, int omega_sw,  vector<vector<ZZ>* >* gen_prec){
	long i;
	int t_1, t_2;
	ZZ prod, p;
	vector<int>* E_1;
	vector<int>* E_2;
	long num_b;
	ZZ mod;
	mod = G.get_mod();
	num_b = NumBits(G.get_ord());
	E_1 = to_basis_sw(e_1, num_b, omega_sw);
	E_2 = to_basis_sw(e_2, num_b, omega_sw);
	prod = 1;
	t_1 = 0;
	t_2 = 0;
	t_1 = (E_1->at(num_b-1)+1)/2;
	t_2 = (E_2->at(num_b-1)+1)/2;
	if ( t_1> 0){
		MulMod(prod,prod,gen_prec->at(0)->at(t_1-1),mod);
	}
	if ( t_2> 0){
		MulMod(prod,prod,gen_prec->at(1)->at(t_2-1),mod);
	}
	for(i = num_b-2; i>=0; i--){
		t_1 = (E_1->at(i)+1)/2;
		t_2 = (E_2->at(i)+1)/2;
		SqrMod(prod,prod, mod);
		if ( t_1> 0){
			MulMod(prod,prod,gen_prec->at(0)->at(t_1-1),mod);
		}
		if ( t_2> 0){
			MulMod(prod,prod,gen_prec->at(1)->at(t_2-1),mod);
		}
	}
	delete E_1;
	delete E_2;
	 return prod;
}

void multi_expo::multi_expo_sw(ZZ& prod, ZZ e_1, ZZ e_2, int omega_sw, vector<vector<ZZ>* >* gen_prec){
	long i;
	int t_1, t_2;
	ZZ p;
	vector<int>* E_1;
	vector<int>* E_2;
	long num_b;
	ZZ mod;
	mod = G.get_mod();
	num_b = NumBits(G.get_ord());
	E_1 = to_basis_sw(e_1, num_b, omega_sw);
	E_2 = to_basis_sw(e_2, num_b, omega_sw);
	prod = 1;
	t_1 = 0;
	t_2 = 0;
	t_1 = (E_1->at(num_b-1)+1)/2;
	t_2 = (E_2->at(num_b-1)+1)/2;
	if ( t_1> 0){
		MulMod(prod,prod,gen_prec->at(0)->at(t_1-1),mod);
	}
	if ( t_2> 0){
		MulMod(prod,prod,gen_prec->at(1)->at(t_2-1),mod);
	}
	for(i = num_b-2; i>=0; i--){
		t_1 = (E_1->at(i)+1)/2;
		t_2 = (E_2->at(i)+1)/2;
		SqrMod(prod,prod, mod);
		if ( t_1> 0){
			MulMod(prod,prod,gen_prec->at(0)->at(t_1-1),mod);
		}
		if ( t_2> 0){
			MulMod(prod,prod,gen_prec->at(1)->at(t_2-1),mod);
		}
	}
	delete E_1;
	delete E_2;
	// return prod;
}


