/*
 * func_ver.cpp
 *
 *  Created on: 04.07.2012
 *      Author: stephaniebayer
 */

#include "func_ver.h"
#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "ElGammal.h"
#include "multi_expo.h"
#include <fstream>

#include <time.h>
#include "FakeZZ.h"
#include "CurvePoint.h"
NTL_CLIENT

#include <assert.h>

extern G_q G;
extern G_q H;
extern Pedersen Ped;
//extern ElGammal El;

func_ver::func_ver() {}

func_ver::~func_ver() {}

void func_ver::check_Dh_op(vector<Mod_p>* c_Dh, vector<ZZ>* chal, vector<ZZ>* D_h_bar, ZZ r_Dh_bar, long win_LL, Pedersen& ped, bool& b){
	Mod_p t_Dh, co_Dh;

	multi_expo::multi_expo_LL(t_Dh,c_Dh, chal, win_LL);
	co_Dh = ped.commit_opt(D_h_bar,r_Dh_bar);

	//cout<<"D_h "<<t_Dh<<" "<<co_Dh<<endl;
	b = (t_Dh == co_Dh);
}


void func_ver::check_D_op(Mod_p c_D0, Mod_p c_z, vector<Mod_p>* c_A, vector<Mod_p>* c_B, vector<ZZ>* chal_1, ZZ chal_2, vector<ZZ>* A_bar, ZZ r_A_bar, long n, Pedersen& ped, bool& b){
	int i;
	Mod_p t_D, co_D, temp, inv;
	long m = c_A->size()-1;
	ZZ ord = H.get_ord();
	vector<ZZ>* v_1 = new vector<ZZ>(n);

	t_D = c_D0;
	Mod_p::inv(inv, c_z);
	for (i=1; i<m;i++){
		Mod_p::expo(temp,c_A->at(i),chal_2);
		Mod_p::mult(temp, temp,c_B->at(i));
		Mod_p::mult(temp,temp,inv);
		Mod_p::expo(temp, temp, chal_1->at(i-1));
		Mod_p::mult(t_D,t_D,temp);
	}
	for(i=0; i<n;i++){
		NegateMod(v_1->at(i),to_ZZ(1),ord);
	}
	temp=ped.commit_opt(v_1,to_ZZ(0));
	Mod_p::expo(temp, temp, chal_1->at(m-1));
	Mod_p::mult(t_D,t_D,temp);
	co_D = ped.commit_opt(A_bar, r_A_bar);
	//cout<<"D "<<t_D<<" "<<co_D<<endl;
	delete v_1;
	b = (t_D == co_D);
}


void func_ver::check_Ds_op(vector<Mod_p>* c_Ds, vector<Mod_p>* c_Dh, Mod_p c_Dm, vector<ZZ>* chal_1, vector<ZZ>* chal_2, vector<ZZ>* Ds_bar, ZZ r_Ds_bar, Pedersen& ped, bool& b){
	long i,l;
	long m= c_Ds->size()-1;
	Mod_p t_Ds, co_Ds, temp, temp_1;

	l=m-1;
	for(i=0; i<l; i++){
		Mod_p::expo(c_Ds->at(i),c_Dh->at(i), chal_1->at(i));
	}
	if(m>1){
		Mod_p::expo(temp, c_Dh->at(1), chal_1->at(0));
		for(i=1;i<l; i++){
			Mod_p::expo(temp_1,c_Dh->at(i+1), chal_1->at(i));
			Mod_p::mult(temp,temp,temp_1);
		}
		c_Ds->at(l)=temp;
	}
	else{
           // TODO this is never called right?
          assert(false);
		// c_Ds->at(l)=Mod_p(1,G.get_mod());
	}

	c_Ds->at(m)=c_Dm;
	Mod_p::expo(t_Ds, c_Ds->at(0),chal_2->at(m-1));
	for(i=1; i<m; i++){
		Mod_p::expo(temp, c_Ds->at(i), chal_2->at(m-1-i));
		Mod_p::mult(t_Ds, t_Ds,temp);
	}
	Mod_p::mult(t_Ds,t_Ds,c_Ds->at(m));
	co_Ds = ped.commit_opt(Ds_bar, r_Ds_bar);
	//cout<<"Ds "<<t_Ds<<" "<<co_Ds<<endl;
	b = (t_Ds == co_Ds);
}

void func_ver::check_Dl_op(vector<Mod_p>* c_Dl, vector<ZZ>* chal, vector<ZZ>* A_bar, vector<ZZ>* Ds_bar, vector<ZZ>*  chal_1, ZZ r_Dl_bar, Pedersen& ped, bool& b){
	long i;
	Mod_p t_Dl, co_Dl, temp;
	ZZ temp_1;
	long l = c_Dl->size();
	long pos = (l-1)/2+1;
	ZZ mod = G.get_mod();

	t_Dl = c_Dl->at(0);
	for(i=1; i<l; i++){
		Mod_p::expo(temp, c_Dl->at(i),chal->at(i-1));
		//Mod_p::mult(t_Dl,t_Dl, temp);
		t_Dl = t_Dl*temp;
	}
	temp_1=Functions::bilinearMap(A_bar,Ds_bar,chal_1);
	co_Dl =ped.commit_sw(temp_1,r_Dl_bar);
	//cout<<"Dl "<<t_Dl<<" "<<co_Dl<<endl;

	temp= Mod_p(curve_zeropoint(), mod);
	b = ((t_Dl==co_Dl) & (c_Dl->at(pos)==temp));
}


void func_ver::check_d_op(vector<Mod_p>* c_Dh, Mod_p c_d, vector<ZZ>* chal, vector<ZZ>* d_bar, ZZ r_d_bar, Pedersen& ped, bool& b){
	Mod_p t_d, co_d, temp;
	long m = c_Dh->size();
	Mod_p::expo(temp, c_Dh->at(m-1), chal->at(0));
	Mod_p::mult(t_d, temp, c_d);
	co_d = ped.commit_opt(d_bar, r_d_bar);
	//cout<<"d "<<t_d<<" "<<co_d<<endl;
	b = (t_d==co_d);
}


void func_ver::check_Delta_op(Mod_p c_dh, Mod_p c_Delta, vector<ZZ>* chal, vector<ZZ>* Delta_bar, vector<ZZ>* d_bar, ZZ r_Delta_bar, ZZ chal_1, ZZ chal_2, ZZ chal_3, Pedersen& ped, bool& b){
	long i,j;
	Mod_p t_Delta, co_Delta, temp;
	ZZ t_1, t_2, t_3, prod, chal_temp;
	ZZ ord = H.get_ord();
	long m = (chal->size()-1)/2;
	long n = Delta_bar->size();
	vector<ZZ>* Delta_temp=0;

	Mod_p::expo(temp, c_dh, chal->at(0));
	Mod_p::mult(t_Delta, temp, c_Delta);

	Delta_temp = new vector<ZZ>(n-1);
	t_3= chal->at(0);
	for(i=0; i<n-1; i++){
		MulMod(t_1, Delta_bar->at(i), d_bar->at(i+1), ord);
		MulMod(t_2, t_3, Delta_bar->at(i+1),ord);
		SubMod(Delta_temp->at(i), t_2, t_1, ord);
	}

	co_Delta = ped.commit_opt(Delta_temp, r_Delta_bar);

	delete Delta_temp;
	//cout<<"Delta "<<t_Delta<<" "<<co_Delta<<endl;

	prod = to_ZZ(1);
	chal_temp =to_ZZ(1);
	for(i=1; i<=m; i++){
		for(j=1; j<=n; j++){
			MulMod(chal_temp, chal_temp, chal_1,ord);
			SubMod(t_1, chal_temp, chal_2,ord);
			t_3 = n*(i-1)+j;
			MulMod(t_3,t_3, chal_3, ord);
			AddMod(t_1,t_1, t_3, ord);
			MulMod(prod,prod, t_1, ord);
		}
	}
	MulMod(prod, prod, chal->at(0), ord);

	//cout<<"prod "<<prod<<" "<<Delta_bar->at(n-1)<<endl;
	//cout<<d_bar->at(0)<<" "<<Delta_bar->at(0)<<endl;
	b = false;
	if(t_Delta ==co_Delta)
		if((prod ==Delta_bar->at(n-1)) & (d_bar->at(0)==Delta_bar->at(0))){
			b = true;
		//return 1;
	}
	
	//return 0;
}

void func_ver::fill_vector(vector<ZZ>* t){
	ZZ temp;
	ZZ ord = H.get_ord();
	temp = RandomBnd(ord);
	return fill_vector(t, temp);
}

void func_ver::fill_vector(vector<ZZ>* t, ZZ& challenge) {
	long i,l;
	ZZ ord = H.get_ord();
	l= t->size();
	t->at(0)=challenge;
	for(i=1; i<l; i++){
		MulMod(t->at(i),t->at(i-1),challenge, ord);
	}
}

void func_ver::fill_x8(vector<ZZ>* chal_x8, vector<vector<long>* >* basis_chal_x8, vector<ZZ>* mul_chal_x8, long omega, ZZ& chal){
	long i, l;
	//ZZ chal;
	ZZ ord = H.get_ord();
	long num_b= NumBits(ord);

	l= chal_x8->size();
	//chal = RandomBnd(ord);

	chal_x8->at(0)= chal;
	basis_chal_x8->at(0) = multi_expo::to_basis(to_ZZ(1),num_b, omega);
	basis_chal_x8->at(1) = multi_expo::to_basis(chal_x8->at(0),num_b, omega);

	mul_chal_x8->at(0) =1;
	mul_chal_x8->at(1) =chal_x8->at(0);

	for (i = 1; i<l; i++){
		 MulMod(chal_x8->at(i),chal, chal_x8->at(i-1), ord);
		 basis_chal_x8->at(i+1) = multi_expo::to_basis(chal_x8->at(i),num_b, omega);
		 mul_chal_x8->at(i+1) = chal_x8->at(i);
	}
}
