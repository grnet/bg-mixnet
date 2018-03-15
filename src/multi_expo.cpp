/*
 * multi_expo.cpp
 *
 *  Created on: 02.07.2012
 *      Author: stephaniebayer
 */

#include "multi_expo.h"
#include "G_q.h"
#include "ElGammal.h"
#include "Pedersen.h"
#include "Cipher_elg.h"
#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT
#include<vector>
using namespace std;
#include <iostream>
#include <time.h>
#include <fstream>

#include <assert.h>

extern G_q G;
extern G_q H;
extern ElGammal El;
extern Pedersen Ped;

multi_expo::multi_expo() {}

multi_expo::~multi_expo() {}


vector<vector<int>*>* multi_expo::to_binary(int win){

	vector<vector<int>* >* ret;
	vector<int>* temp;
	long e,i,j;

        e = (1L << win);
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
	long i;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*(1L << i);
	}
	return t;
}

void multi_expo::to_long(long& t,vector<int>* bit_r){

	long  length;
	long i;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t = t+bit_r->at(i)*(1L << i);
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

void multi_expo::expo_mult(CurvePoint& prod, const vector<ZZ>* e, ZZ ran, int omega_expo, vector<Mod_p>* gen){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	long length;// num_b;
	CurvePoint p, temp_1, temp_2;
        ZZ mod;
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
	prod = curve_zeropoint();
	p = curve_zeropoint();
        t = (1L << omega_expo) - 1;
	length = length +1;
	for(i=l-1; i>0; i--){

		p = curve_zeropoint();
		for(j = 0; j<length; j++){
			if(basis_vec->at(j)->at(i)==t){
				MulMod(p,p,gen->at(j).get_val(),mod);
			}
		}
		temp_1 = p;
		temp_2 = p;
		for(k = t-1; k>0; k--){
			p = curve_zeropoint();
			for(j = 0; j<length; j++){
				if(basis_vec->at(j)->at(i)==k){
					MulMod(p,p,gen->at(j).get_val(),mod);
				}
			}
			MulMod(temp_1,temp_1,p,mod);
			MulMod(temp_2,temp_1,temp_2,mod);

		}
		MulMod(prod,prod,temp_2,mod);
		for(k = 0; k<omega_expo; k++){
			SqrMod(prod,prod,mod);
		}
	}
	p = curve_zeropoint();
	for(j = 0; j<length; j++){
		if(basis_vec->at(j)->at(0)==t){
			MulMod(p,p,gen->at(j).get_val(),mod);
		}
	}
	temp_1 = p;
	temp_2 = p;
	for(k = t-1; k>0; k--){
		p = curve_zeropoint();
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

void multi_expo::expo_mult(Cipher_elg& prod, const vector<Cipher_elg>* a, vector<ZZ>* e, int omega ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	ZZ ord = H.get_ord();
	long length;
	CurvePoint prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v;
        ZZ mod;
	long num_b;

	length = a->size();
	mod = H.get_mod();
	num_b = NumBits(ord);
	l = num_b/omega +1;

	vector<CurvePoint> a_u(length);
	vector<CurvePoint> a_v(length);
	basis_vec = new vector<vector<long>* >(length);
	for(i = 0; i<length; i++){
		basis_vec->at(i) = to_basis(e->at(i), num_b,omega);
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = curve_zeropoint();
	prod_v = curve_zeropoint();
	t = (1L << omega)-1;
	for(i=l-1; i>0; i--){
		p_u= curve_zeropoint();
		p_v= curve_zeropoint();
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
			p_u= curve_zeropoint();
			p_v = curve_zeropoint();
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
	p_u= curve_zeropoint();
	p_v= curve_zeropoint();
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
		p_u= curve_zeropoint();
		p_v = curve_zeropoint();
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

void multi_expo::expo_mult(Cipher_elg& prod, const vector<Cipher_elg>* a, vector<vector<long>*>* basis_vec, int omega ){
	long i, j, k, l,t;
	long length;
	CurvePoint prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v;
        ZZ mod;
	long num_b;

	length = a->size();
	mod = H.get_mod();
	num_b = NumBits(H.get_ord());
	l = num_b/omega +1;

	vector<CurvePoint> a_u(length);
	vector<CurvePoint> a_v(length);

	for(i = 0; i<length; i++){
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = curve_zeropoint();
	prod_v = curve_zeropoint();
        t = (1L << omega) - 1;
	for(i=l-1; i>0; i--){
		p_u = curve_zeropoint();
		p_v = curve_zeropoint();
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
			p_u = curve_zeropoint();
			p_v = curve_zeropoint();
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
	p_u = curve_zeropoint();
	p_v = curve_zeropoint();
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
		p_u = curve_zeropoint();
		p_v = curve_zeropoint();
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

void multi_expo::expo_mult(Cipher_elg& prod, const vector<Cipher_elg>* a, ZZ f, vector<ZZ>* e, int omega ){
	long i, j, k, l,t;
	vector<vector<long>* >* basis_vec;
	ZZ ord = H.get_ord();
	long length;
	CurvePoint prod_u, p_u, temp_1_u, temp_2_u,prod_v, p_v, temp_1_v, temp_2_v;
        ZZ mod, temp;
	long num_b;

	length = e->size();
	mod = H.get_mod();
	num_b = NumBits(ord);
	l = num_b/omega +1;
	vector<CurvePoint> a_u(length);
	vector<CurvePoint> a_v(length);
	basis_vec = new vector<vector<long>* >(length);
	for(i = 0; i<length; i++){
		temp = MulMod(f,e->at(i),ord);
		basis_vec->at(i) = to_basis(temp, num_b,omega);
		a_u.at(i)=a->at(i).get_u();
		a_v.at(i)=a->at(i).get_v();
	}
	prod_u = curve_zeropoint();
	prod_v= curve_zeropoint();
        t = (1L << omega) - 1;
	for(i=l-1; i>0; i--){
		p_u= curve_zeropoint();
		p_v= curve_zeropoint();
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
			p_u= curve_zeropoint();
			p_v = curve_zeropoint();
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
	p_u= curve_zeropoint();
	p_v= curve_zeropoint();
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
		p_u= curve_zeropoint();
		p_v = curve_zeropoint();
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

vector<vector<CurvePoint>* >* multi_expo::calc_Yk(vector<CurvePoint>* y, int win){
	vector<vector<CurvePoint>* >* ret;
	vector<CurvePoint>* temp;
	long h,t, i,j,k,e;
	CurvePoint prod, tem;
        ZZ mod;
	mod = H.get_mod();
	vector<vector<int>* >* binary;

	h= y->size()/win;
        e = (1L << win);
	binary = to_binary(win);

	if((unsigned) h*win ==y->size()){
		ret = new vector<vector<CurvePoint>* >(h);
		for(i =0; i<h; i++){
			temp = new vector<CurvePoint>(e);
			for(j=0; j<e; j++){
				prod = curve_zeropoint();
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
		ret = new vector<vector<CurvePoint>* >(h+1);
		t= y->size()-h*win;
		for(i =0; i<h; i++){
			temp = new vector<CurvePoint>(e);
			for(j=0; j<e; j++){
				prod = curve_zeropoint();
				for(k=0; k<win; k++){
					PowerMod(tem,y->at(i*win+k), binary->at(j)->at(k), mod);
					MulMod(prod,prod,tem,mod);
				}
				temp->at(j)=prod;
			}
			ret->at(i)=temp;
		}
		temp = new vector<CurvePoint>(e);
		for(j=0; j<e; j++){
			prod = curve_zeropoint();
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
	Mod_p prod, tem;
	vector<vector<int>* >* binary;

	h= y->size()/win;
        e = (1L << win);
	binary = to_binary(win);

	if((unsigned)h*win == y->size()){
		ret = new vector<vector<Mod_p>* >(h);
		for(i =0; i<h; i++){
			temp = new vector<Mod_p>(e);
			for(j=0; j<e; j++){
                          // TODO this is never called?
                          assert(false);
				prod = Mod_p(curve_zeropoint(),G.get_mod());
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
				prod = Mod_p(curve_zeropoint(),G.get_mod());
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
			prod =  Mod_p(curve_zeropoint(),G.get_mod());;
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

void multi_expo::multi_expo_LL(Mod_p& ret, vector<Mod_p>* y, vector<ZZ>* e, int win){
        Mod_p temp;
        ZZ mod = H.get_mod();
        ret =  Mod_p(curve_zeropoint(),G.get_mod());;
        unsigned int i;
        for (i = 0; i < y->size(); i++) {
          Mod_p::expo(temp, y->at(i), e->at(i));
          Mod_p::mult(ret, ret, temp);
        }
        // TODO to be determined whether real LL method faster...
}

void multi_expo::multi_expo_LL(CurvePoint& ret, vector<CurvePoint>* y, vector<ZZ>* e, int win){
        CurvePoint temp;
        ZZ mod = H.get_mod();
        ret = curve_zeropoint();
        unsigned int i;
        for (i = 0; i < y->size(); i++) {
          PowerMod(temp, y->at(i), e->at(i), mod);
          MulMod(ret, ret, temp, mod);
        }
        // TODO to be determined whether real LL method faster...
}

void multi_expo::multi_expo_LL(Cipher_elg& ret, Cipher_elg c1, Cipher_elg c2, Cipher_elg c3, Cipher_elg c4, vector<ZZ>* e, int win){
	CurvePoint ret_u, ret_v;
	vector<CurvePoint>* c_u;
	vector<CurvePoint>* c_v;

	c_u = new vector<CurvePoint>(4);
	c_u->at(0) = c1.get_u();
	c_u->at(1) = c2.get_u();
	c_u ->at(2) = c3.get_u();
	c_u->at(3) = c4.get_u();

	c_v = new vector<CurvePoint>(4);
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

void multi_expo::multi_expo_sw(CurvePoint& prod, ZZ e_1, ZZ e_2, int omega_sw, vector<vector<CurvePoint>* >* gen_prec){
	long i;
	int t_1, t_2;
	vector<int>* E_1;
	vector<int>* E_2;
	long num_b;
	ZZ mod;
	mod = G.get_mod();
	num_b = NumBits(G.get_ord());
	E_1 = to_basis_sw(e_1, num_b, omega_sw);
	E_2 = to_basis_sw(e_2, num_b, omega_sw);
	prod = curve_zeropoint();
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
}
