/*
 * func_pro.cpp
 *
 *  Created on: 05.07.2012
 *      Author: stephaniebayer
 */

#include "func_pro.h"
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Functions.h"
#include "ElGammal.h"
#include "multi_expo.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>

#include "FakeZZ.h"
NTL_CLIENT

extern G_q G;
extern G_q H;
/*extern ElGammal El;
extern double time_rw_p;
extern double time_cm;
extern long mu;
extern long mu_h;
extern long m_r;*/

func_pro::func_pro() {}

func_pro::~func_pro() {}


//Generates a matrix, containing the values 1 to N ordered
void func_pro::set_X(vector<vector<ZZ>*>* X, long m,  long n){
	long i,j, val;
	vector<ZZ>* r = 0;
	val =1;
	for (i=0; i<m; i++ ){
		r = new vector<ZZ>(n);
		for(j=0; j<n; j++){
			r->at(j)=val;
			val++;
		}
		X->at(i)=r;
	}

}

//Permutes the matrix X regarding the permutation pi, to set the matrix A
void func_pro::set_A(vector<vector<ZZ>*>* A,  vector<vector<vector<long>* >* >* pi, long m, long n){
	long i,j,  row, col;
	vector<vector<ZZ>* >* X=0;
	//Creates the matrixs X, containing 1 to N
	X= new vector<vector<ZZ>* >(m);
	set_X(X, m,n);

	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"matrix A "<<endl;*/
	vector<ZZ>* r;
	for (i=0; i<m; i++ ){
		r = new vector<ZZ>(n);
		for(j=0; j<n; j++){
			row = pi->at(i)->at(j)->at(0);
			col = pi->at(i)->at(j)->at(1);
			r->at(j)=X->at(row)->at(col);
		//	ost<<r->at(j)<<" ";
		}
	//	ost<<endl;
		A->at(i)=r;
	}
	Functions::delete_vector(X);

}

void func_pro::set_x2(vector<vector<ZZ>* >* chal_x2, ZZ x2, long m, long n){
	vector<ZZ>* r= 0;
	ZZ temp;
	long i,j;
	ZZ ord = G.get_ord();

	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"matrix X, challenges to power "<<endl;*/
	temp = to_ZZ(1);
	for (i = 0; i<m; i++){
		r = new vector<ZZ>(n);
		for(j = 0; j<n;j++){
			MulMod(temp, temp,x2,ord);
			r->at(j)=temp;
		//	ost<<temp<<" ";
		}
		//ost<<endl;
		chal_x2->at(i) = r;
	}

}

void func_pro::set_B_op(vector<vector<ZZ>* >* B, vector<vector<vector<long>* >* >* basis_B, vector<vector<ZZ>* >* chal_x2,vector<vector<vector<long>* >*>* pi, long omega_mulex){
	long i,j;
	long row, col;
	vector<ZZ>* r =0;
	vector<vector<long>* >* basis_vec = 0;
	long num_b =NumBits(G.get_ord());
	long m = B->size();
	long n = pi->at(0)->size();

	//permute the exponents in s using the permutation  pi
	for (i=0; i<m; i++ ){
		r = new vector<ZZ>(n);

		basis_vec =  new vector<vector<long>* >(n);
		for(j=0; j<n; j++){
			row = pi->at(i)->at(j)->at(0);
			col = pi->at(i)->at(j)->at(1);
			r->at(j)=chal_x2->at(row)->at(col);
			basis_vec->at(j) = multi_expo::to_basis(r->at(j), num_b,omega_mulex);
		}
		B->at(i) = r;
		basis_B->at(i)=basis_vec;
	}

}

void func_pro::set_B(vector<vector<ZZ>* >* B, vector<vector<ZZ>* >* chal_x2,vector<vector<vector<long>* >*>* pi){
	long i,j;
	long row, col;
	vector<ZZ>* r =0;
	long m = B->size();
	long n = pi->at(0)->size();

	//permute the exponents in s using the permutation  pi
	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"matrix B, permuted challenges "<<endl;*/
	for (i=0; i<m; i++ ){
		r = new vector<ZZ>(n);

		for(j=0; j<n; j++){
			row = pi->at(i)->at(j)->at(0);
			col = pi->at(i)->at(j)->at(1);
			r->at(j)=chal_x2->at(row)->at(col);
		//	ost<<r->at(j)<<" ";
		}
	//	ost<<endl;
		B->at(i) = r;
	}

}

void func_pro::set_D(vector<vector<ZZ>* >* D, vector<vector<ZZ>* >* A, vector<vector<ZZ>* >* B, ZZ chal_z4, ZZ chal_y4){
	long i, j;
	vector<ZZ>* row;
	ZZ temp, temp_1;
	ZZ ord = G.get_ord();
	long m = A->size();
	long n = A->at(0)->size();

	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"set vector D = yA_ij +B_ij "<<endl;*/
	for (i= 0; i<m; i++){
		row = new vector<ZZ>(n);
		for (j = 0; j<n; j++){
			MulMod(temp, chal_y4,A->at(i)->at(j),ord);
			SubMod(temp_1, B->at(i)->at(j), chal_z4,ord);
			AddMod(row->at(j),temp ,temp_1, ord);
		//	ost<<temp<<" ";
		}
		//ost<<endl;
		D->at(i)= row;
	}

	Functions::delete_vector(A);
}

void func_pro::set_D_h(vector<vector<ZZ>* >* D_h, vector<vector<ZZ>* >* D){
	long i;
	vector<ZZ>* row;
	long m = D_h->size();
	long n = D->at(0)->size();
	row = new vector<ZZ>(n);
	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"Hadamard products over D, 1 row = D_0, D_0¡D_1,... "<<endl;*/
	for(i=0; i<n; i++){
		row->at(i)=D->at(0)->at(i);
	//	ost<<row->at(i)<<" ";
	}
	D_h->at(0)=row;
	for( i=1; i<m;i++){
		row = new vector<ZZ>(n);
		Functions::Hadamard(row, D_h->at(i-1),D->at(i));
		D_h->at(i)=row;
	//	ost<<endl;
	/*	for(j=0; j<n; j++){
			ost<<D_h->at(i)->at(j)<<" ";
		}*/
	}
//	ost<<endl;

}

void func_pro::commit_B0_op(vector<ZZ>* B_0, vector<vector<long>* >* basis_B0, ZZ &r_B0, Mod_p &c_B0, long omega_mulex, Pedersen& ped){
	long i,num_b;
	ZZ ord= H.get_ord();
	long n = B_0->size();

	num_b = NumBits(ord);
	r_B0  =  RandomBnd(ord);
	for (i = 0; i<n; i++){
		B_0 ->at(i) = RandomBnd(ord);
		basis_B0->at(i)= multi_expo::to_basis(B_0->at(i), num_b, omega_mulex);
	}

	c_B0 = ped.commit_opt(B_0, r_B0);

}

void func_pro::set_Rb(vector<vector<ZZ>* >* B, vector<vector<ZZ>*>* R, ZZ &R_b){
	long i,j;
	ZZ temp;
	ZZ ord = H.get_ord();
	long m = B->size();
	long n = B->at(0)->size();

	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"R_b =sum sum r_ij*B_ij  ";*/
	R_b = 0;
	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){
			MulMod(temp, B->at(i)->at(j),R->at(i)->at(j), ord);
			AddMod(R_b,R_b, temp ,ord);
		}
	}
	//ost<<R_b<<endl;
}


void func_pro::commit_a_op(vector<ZZ>* a, vector<ZZ>* r_a, vector<Mod_p>* c_a, Pedersen& ped){
	long i,l;
	ZZ ord = H.get_ord();
	long m = a->size()/2;

	for(i= 0; i<m; i++){
		a->at(i) = RandomBnd(ord);
		r_a->at(i) = RandomBnd(ord);
		c_a->at(i) = ped.commit_sw(a->at(i),r_a->at(i));
	}
	a->at(m) = to_ZZ(0);
	r_a->at(m) = to_ZZ(0);
	c_a->at(m) = ped.commit_sw(a->at(m),r_a->at(m));
	l= 2*m;
	for(i= m+1; i<l; i++){
		a->at(i) = RandomBnd(ord);
		r_a->at(i) = RandomBnd(ord);
		c_a->at(i) = ped.commit_sw(a->at(i),r_a->at(i));
	}
}

void func_pro::set_D_s(vector<vector<ZZ>* >* D_s, vector<vector<ZZ>* >* D_h, vector<vector<ZZ>* >* D, vector<ZZ>* chal, ZZ & r_Dl_bar){
	long i, j, l;
	ZZ temp;
	ZZ ord = H.get_ord();
	vector<ZZ>* row=0;
	vector<ZZ>* Ds_temp=0;
	long m = D_h->size();
	long n = D_h->at(0)->size();
	l=m-1;
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);*/
	for(i=0; i<l; i++){
		temp=chal->at(i);
		row = new vector<ZZ>(n);
		for(j=0; j<n; j++){
			MulMod(row->at(j),temp, D_h->at(i)->at(j), ord );
		//	ost<<row->at(j)<<" ";
		}
		D_s->at(i)=row;
		//ost<<endl;
	}
	Ds_temp = new vector<ZZ>(n);
	for(i=0; i<n;i++){
		Ds_temp->at(i)=0;
	}
	for(i=0; i<l; i++){
		temp = chal->at(i);
		for(j=0; j<n; j++){
			MulMod(r_Dl_bar,temp, D_h->at(i+1)->at(j),ord);
			AddMod(Ds_temp->at(j),Ds_temp->at(j),r_Dl_bar,ord);
		//	ost<<Ds_temp->at(j)<<" ";
		}
	}
	//l=m-1
	D_s->at(l)=Ds_temp;
	//ost<<endl;

	row = new vector<ZZ>(n);
	for(i=0; i<n; i++){
		row->at(i)=RandomBnd(ord);
		//ost<<row->at(i)<<" ";
		D->at(0)->at(i) = RandomBnd(ord);
	}
	D_s->at(m)=row;
//	ost<<endl;
/*	ost<<"D(0) ist "<<endl;
	for(i=0; i<n; i++){
		ost<<D->at(0)->at(i)<<" ";
	}
	ost<<endl;*/
}

void func_pro::commit_Dl_op(vector<Mod_p>* c_Dl, vector<ZZ>* Dl, vector<ZZ>* r_Dl, vector<vector<ZZ>* >* D, vector<vector<ZZ>* >* D_s, vector<ZZ>* chal, Pedersen& ped){
	long i,j,l, len;
	ZZ temp;
	ZZ ord = H.get_ord();
	long m = D->size()-1;

	//Calculate the D_l's
	len = 2*m+1;
	for (l=0; l<len; l++){
		temp =0;
		for (i = 0; i<=m; i++){
			j = m+i-l;
			if ((j>=0) & (j<=m)){
				AddMod(temp,temp,Functions::bilinearMap(D->at(i), D_s->at(j), chal), ord);
			}
		}
		Dl->at(l) = temp;
	}

	//Commits to the Dls
	for (i=0; i<len;i++){
		r_Dl->at(i) =  RandomBnd(ord);
	}
	r_Dl->at(m+1)=0;

	for(l = 0; l<len; l++){
		c_Dl->at(l)= ped.commit_sw(Dl->at(l),r_Dl->at(l));
	}

}

void func_pro::commit_d_op(vector<ZZ>* d, ZZ &r_d, Mod_p& c_d, Pedersen& ped){
	long i;
	ZZ ord = H.get_ord();
	long n=d->size();

	for(i=0; i<n; i++){
		d->at(i)=RandomBnd(ord);
	}

	Functions::commit_op(d, r_d, c_d, ped);
}

void func_pro::commit_Delta_op(vector<ZZ>* Delta, vector<ZZ>* d, ZZ &r_Delta, Mod_p &c_Delta, Pedersen& ped){
	long i,l;
	ZZ temp;
	ZZ ord = H.get_ord();
	vector<ZZ>* Delta_temp;
	long n = Delta->size();

	for(i=0; i<n; i++){
		Delta->at(i)=RandomBnd(ord);
	}
	Delta->at(0)=d->at(0);
	Delta->at(n-1)=0;

	l=n-1;
	Delta_temp= new vector<ZZ>(n-1);
	for(i=0; i<l; i++){
		MulMod(temp, Delta->at(i), d->at(i+1),ord);
		NegateMod(Delta_temp->at(i), temp, ord);
	}

	Functions::commit_op(Delta_temp, r_Delta, c_Delta, ped);

	delete Delta_temp;
}

void func_pro::commit_d_h_op(vector<vector<ZZ>* >* D_h, vector<ZZ>* d_h, vector<ZZ>* d, vector<ZZ>* Delta, ZZ & r_d_h, Mod_p &c_d_h, Pedersen& ped){
	long i,l;
	ZZ temp, temp_1;
	ZZ ord = G.get_ord();
	long m = D_h->size();
	long n = d_h->size();

	//setting the vectors to prove the product
	d_h->at(0) = D_h->at(m-1)->at(0);
	for(i=1;i<n;i++){
		MulMod(d_h->at(i), d_h->at(i-1), D_h->at(m-1)->at(i), ord);
	}

	l=n-1;
	vector<ZZ>* d_h_temp = new vector<ZZ>(l);
	for(i=0; i<l; i++){
		MulMod(temp, d_h->at(i), d->at(i+1), ord);
		MulMod(temp_1, D_h->at(m-1)->at(i+1),Delta->at(i),ord);
		SubMod(temp_1, Delta->at(i+1), temp_1, ord);
		SubMod(d_h_temp->at(i), temp_1, temp, ord);
	}

	Functions::commit_op(d_h_temp, r_d_h, c_d_h, ped);

	delete d_h_temp;
}

void func_pro::calculate_B_bar(vector<ZZ>* B_0, vector<vector<ZZ>* >* B, vector<ZZ>* chal, vector<ZZ>* B_bar){
	long i,j;
	ZZ temp, temp_1;
	ZZ ord = G.get_ord();
	long m = B->size();
	long n = B_0->size();
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
ost<<"B_bar ";*/
	for (j = 0; j<n; j++){
		temp= B_0->at(j);
		for (i = 0; i<m; i++){
			 MulMod(temp_1, B->at(i)->at(j),chal->at(i), ord);
			AddMod(temp, temp ,temp_1, ord);
		}
		B_bar->at(j) = temp;
		//		ost<<temp<<" ";
	}
	//	ost<<endl;
}

void func_pro::calculate_r_B_bar(vector<ZZ>* r_B, vector<ZZ>* chal, ZZ r_B0, ZZ &r_B_bar){
	long i;
	ZZ temp;
	ZZ ord = H.get_ord();
	long m = r_B->size();
	r_B_bar = r_B0;

/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"r_B_bar ";*/
	for (i = 0; i<m; i++){
		MulMod(temp, r_B->at(i), chal->at(i), ord);
		AddMod( r_B_bar, r_B_bar , temp, ord);
	}
	//	ost<<r_B_bar<<endl;
}

void func_pro::calculate_a_bar(vector<ZZ>* a, vector<ZZ>* chal, ZZ & a_bar){
	long i;
	ZZ temp;
	ZZ ord = G.get_ord();
	long m = a->size();

/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"c_bar ";*/
	a_bar = a->at(0);
	for (i = 1; i<m; i++){
		MulMod(temp, a->at(i),chal->at(i-1), ord);
		AddMod(a_bar,a_bar, temp, ord);
	}
//	ost<<a_bar<<endl;
}

void func_pro::calculate_r_a_bar(vector<ZZ>* r_a, vector<ZZ>* chal, ZZ & r_a_bar){
	long i;
	ZZ temp;
	ZZ ord = G.get_ord();
	long m = r_a->size();

	r_a_bar= r_a->at(0);
	for (i = 1; i<m; i++){
		MulMod(temp, r_a->at(i),chal->at(i-1), ord);
		AddMod(r_a_bar,r_a_bar,temp, ord);
	}
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"r_a_bar "<<r_a_bar<<endl;*/
}

void func_pro::calculate_rho_a_bar(vector<ZZ>* rho_a, vector<ZZ>* chal, ZZ & rho_a_bar){
	long i;
	ZZ temp;
	ZZ ord = G.get_ord();
	long m = rho_a->size();

	rho_a_bar = rho_a->at(0);
	for (i = 1; i<m; i++){
		MulMod (temp,rho_a->at(i),chal->at(i-1),ord);
		AddMod(rho_a_bar, rho_a_bar, temp, ord);
	}
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"r_a_bar "<<rho_a_bar<<endl;*/
}


void func_pro::calculate_D_h_bar(vector<ZZ>* D_h_bar, vector<vector<ZZ>* >* D_h, vector<ZZ>* chal){
	long i,j;
	ZZ temp, temp_1;
	ZZ ord = G.get_ord();
	long m = D_h->size();
	long n = D_h_bar->size();
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
ost<<"D_h_bar ";*/
	for (j = 0; j<n;j++){
		D_h_bar->at(j) =  D_h->at(0)->at(j);
	}
	for(i = 1; i<m; i++){
		temp= chal->at(i-1);
		for (j = 0; j<n;j++){
			MulMod(temp_1,temp,D_h->at(i)->at(j),ord);
			AddMod(D_h_bar->at(j), D_h_bar->at(j),temp_1, ord);
		}
	}/*
	for(i=0; i<n; i++){
		ost<<D_h_bar->at(i)<<" ";
	}
	ost<<endl;*/
}

void func_pro::calculate_r_Dh_bar(vector<ZZ>* r_D_h, vector<ZZ>* chal, ZZ & r_Dh_bar ){
	long i;
	ZZ temp;
	ZZ ord = G.get_ord();
	long m = r_D_h->size();

	r_Dh_bar= r_D_h->at(0);
	for (i = 1;i<m;i++){
		MulMod(temp,chal->at(i-1),r_D_h->at(i),ord);
		AddMod(r_Dh_bar,r_Dh_bar, temp,ord);
	}
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"r_Dh_bar "<<r_Dh_bar<<endl;*/
}

void func_pro::calculate_dbar_rdbar(vector<vector<ZZ>* >* D_h, vector<ZZ>* chal, vector<ZZ>* d_bar, vector<ZZ>* d, vector<ZZ>* r_D_h, ZZ r_d, ZZ & r_d_bar){
	long i;
	ZZ temp, temp_1;
	ZZ ord = G.get_ord();
	long m = D_h->size();
	long n = d_bar->size();

/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);*/
	temp_1 = chal->at(0);
	//	ost<<"d_bar ";
	for(i=0; i<n; i++){
		MulMod(temp, temp_1, D_h->at(m-1)->at(i), ord);
		AddMod(d_bar->at(i), temp, d->at(i), ord);
		//		ost<<d_bar->at(i)<<" ";
	}
	//ost<<endl;
	MulMod(temp, temp_1, r_D_h->at(m-1), ord);
	AddMod(r_d_bar, temp, r_d,ord);
	//ost<<"r_d_bar "<<r_d_bar<<endl;
}

void func_pro::calculate_Deltabar_rDeltabar(vector<ZZ>* d_h, vector<ZZ>* chal, vector<ZZ>* Delta_bar, vector<ZZ>* Delta, ZZ r_d_h, ZZ r_Delta, ZZ & r_Delta_bar){
	long i;
	ZZ temp, temp_1;
	ZZ ord = G.get_ord();
	long n = Delta_bar->size();

/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);*/
	temp_1 = chal->at(0);
	//ost<<"Delta_bar ";
	for(i = 0; i<n; i++){
		MulMod(temp, temp_1, d_h->at(i), ord);
		AddMod(Delta_bar->at(i),temp, Delta->at(i), ord);
		//	ost<<Delta_bar->at(i)<<" ";
	}

	//	ost<<endl;
	MulMod(temp, temp_1, r_d_h,ord);
	AddMod(r_Delta_bar, temp, r_Delta, ord);
	//	ost<<"r_delta_bar "<<r_Delta_bar<<endl;
}

void func_pro::calculate_A_bar(vector<vector<ZZ>* >* D, vector<ZZ>* A_bar, vector<ZZ>* chal){
	long i,j;
	ZZ ord = G.get_ord();
	ZZ temp, temp_1;
	long m = D->size()-1;
	long n = A_bar->size();

/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"A_bar ";*/
	for (i = 0; i<n;i++){
		A_bar->at(i)= D->at(0)->at(i);
	}
	for (i= 1; i<=m; i++){
		temp=chal->at(i-1);
		for (j = 0; j<n;j++){
			MulMod(temp_1,D->at(i)->at(j),temp,ord);
			AddMod(A_bar->at(j), A_bar->at(j) ,temp_1,ord);
		}
	}
/*	for(i=0; i<n; i++){
		ost<<A_bar->at(i)<<" ";
	}
	ost<<endl;*/
}

void func_pro::calculate_D_s_bar(vector<vector<ZZ>* >* D_s, vector<ZZ>* D_s_bar, vector<ZZ>* chal){
	long i,j;
	ZZ ord = G.get_ord();
	ZZ temp, temp_1;
	long m = D_s->size()-1;
	long n = D_s_bar->size();

	temp = chal->at(m-1);
	for (i = 0; i<n;i++){
		MulMod(D_s_bar->at(i), D_s->at(0)->at(i),temp,ord);
	}
	for (i= 1; i< m; i++){
		temp=chal->at(m-i-1);
		for (j = 0; j<n;j++){
			 MulMod( temp_1, D_s->at(i)->at(j),temp,ord);
			 AddMod(D_s_bar->at(j), D_s_bar->at(j),temp_1,ord);
		}
	}
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"D_s_bar ";*/
	for (i = 0; i<n; i++){
		AddMod(D_s_bar->at(i),D_s_bar->at(i),D_s->at(m)->at(i),ord);
		//		ost<<D_s_bar->at(i)<<" ";
	}
	//	ost<<endl;
}

void func_pro::calculate_r_A_bar(ZZ r_D0, vector<ZZ>* r_A, vector<ZZ>* r_B, vector<ZZ>* chal, ZZ r_z, ZZ chal_y4, ZZ & r_A_bar){
	long i;
	ZZ ord = G.get_ord();
	ZZ temp, temp_1;
	long m = r_A->size()-1;

	r_A_bar=r_D0;
	for (i = 1; i<m;i++){
		MulMod(temp,chal_y4,r_A->at(i),ord);
		SubMod(temp_1,r_B->at(i),r_z,ord);
		AddMod(temp,temp,temp_1 ,ord );
		MulMod(temp,temp , chal->at(i-1),ord);
		AddMod(r_A_bar, r_A_bar,temp,ord);
	}
	MulMod(temp,r_A->at(m),chal->at(m-1),ord);
	AddMod(r_A_bar,r_A_bar ,temp,ord);
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"r_A_bar "<<r_A_bar<<endl;*/
}

void func_pro::calculate_r_Ds_bar(vector<ZZ>* r_D_h,  vector<ZZ>* chal_1, vector<ZZ>* chal_2, ZZ & r_Ds_bar, ZZ r_Dm){
	long i,l;
	ZZ ord = G.get_ord();
	ZZ temp, temp_1;
	long m = r_D_h->size();
	vector<ZZ>* r_D_s = new vector<ZZ>(m);

	l=m-1;
	for(i=0; i<l; i++){
		MulMod(r_D_s->at(i),r_D_h->at(i),chal_1->at(i),ord);
	}
	temp_1=0;
	for(i=0; i<l; i++){
		MulMod(temp, chal_1->at(i), r_D_h->at(i+1),ord);
		AddMod(temp_1,temp_1,temp,ord);
	}
	r_D_s->at(l)=temp_1;

	MulMod(r_Ds_bar,r_D_s->at(0),chal_2->at(l),ord);
	for (i = 1; i<m;i++){
		MulMod(temp, r_D_s->at(i) , chal_2->at(m-i-1),ord);
		AddMod(r_Ds_bar, r_Ds_bar,temp,ord);
	}
	AddMod (r_Ds_bar, r_Ds_bar, r_Dm,ord);
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
ost<<"r_Ds_bar "<<r_Ds_bar<<endl;*/
	delete r_D_s;
}

void func_pro::calculate_r_Dl_bar(vector<ZZ>* r_C, vector<ZZ>* chal, ZZ &r_Dl_bar){
	long i;
	ZZ ord = G.get_ord();
	ZZ temp;
	long m = r_C->size();

	r_Dl_bar = r_C->at(0);
	for (i = 1; i<m;i++){
		MulMod(temp,r_C->at(i) , chal->at(i-1),ord);
		AddMod(r_Dl_bar, r_Dl_bar,temp,ord);
	}
/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"r_Dl_bar"<<r_Dl_bar<<endl;*/
}
