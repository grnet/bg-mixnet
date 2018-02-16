/*
 * fft.cpp
 *
 *  Created on: 13.12.2012
 *      Author: stephaniebayer
 */

#include "fft.h"

#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Functions.h"
#include "multi_expo.h"

#include <NTL/ZZ.h>
#include <NTL/mat_ZZ.h>
NTL_CLIENT

fft::fft() {
	// TODO Auto-generated constructor stub

}

fft::~fft() {
	// TODO Auto-generated destructor stub
}



ZZ fft::r_o_u(ZZ gen, long m, ZZ ord){
//	ZZ ord = H.get_ord();
	ZZ rou;
	ZZ pow;
	ZZ temp;
	long t = 2*m;
	if ((ord-1) % t  == 0)
	{
		temp = (ord-1)/t;
		PowerMod(rou,gen,temp , ord);

			if(GCD(rou,ord)==to_ZZ(1))
			{

				if (t&1)
				{
					PowerMod(pow,rou,t, ord);
					if(pow==1){
						//cout<<"rou: "<<ord<<" "<<rou;
						return rou;
					}
				}
				else
				{
					PowerMod(pow,rou,t/2,ord);
					if(pow == (ord-1))
					{
						//cout<<"rou "<<ord<<" "<<rou;
						return rou;
					}
				}
			}
	}
	else
	{
		cout << "There is no" << 2*m <<"-th root of unity"<< endl;
		return to_ZZ(1);
	}
	return to_ZZ(1);
}

long fft::to_long(vector<int>* bit_r){

	long  t, length;
	double two,i;
	two = 2;

	length =bit_r->size();
	t=0;
	for(i = 0; i<length; i++ ){
		t =t +bit_r->at(i)*pow(two,i);
	}
	return t;
}

void fft::bitreverse(long& z, long x, long d){
	long i;
	vector<int>* temp=0;
	temp = new vector<int>(d);
	for(i = 0; i<d; i++){
		if(bit(x,i)==1){
			temp->at(d-i-1)=1;
		}
		else{
			temp->at(d-i-1)=0;
		}
	}
	z = to_long(temp);
	delete temp;
}


void  fft::brevorder(vector<ZZ>* ret, vector<ZZ>* v){

	long i,k,d,l, lv;
	ZZ temp;

	lv = v->size();
	l=ret->size();
	d = NumBits(l)-1;
	for(i = 0; i<lv; i++){
		ret->at(i)=v->at(i);
	}
	for(i = 0; i<l; i++){
		bitreverse(k,i,d);
		if(i<k){
			temp = ret->at(i);
			ret->at(i) = ret->at(k);
			ret->at(k)= temp;
		}
	}
}


void fft::FFT(vector<ZZ>* fft, vector<ZZ>* v , long N, ZZ rootofunity, ZZ ord){

	int n, m, m2, i, i0, i1, k, l;
	ZZ rho, rr, z;
	double two = 2;
	l= v->size();

	//cout<<fft->size()<< "  "<<N;
	brevorder(fft,v);

    n = NumBits(N)-1;	//(* N = 2**n *)
    for(k= 0; k<n; k++){
    	//cout<<k<<" ";
   		m =  pow(two,k);
    	m2 = 2*m;
    	rr = 1;
    	PowerMod(rho,rootofunity,pow(two, n-k-1),ord);
    //	cout<<m2<<" rho "<<rho<<" "<<m<<" ";
        for (i = 0; i<m; i++){
        	i0=i;
			//cout<<" rr "<<rr;
				while( i0 < N){
					i1=i0+m; //cout<<" : "<<i0<<" "<<i1<<" ";
					MulMod(z, fft->at(i1), rr, ord);        //(* rr = rho**i *)
					//cout<<" z "<<z<<" ";
					SubMod(fft->at(i1),fft->at(i0), z, ord);
					//cout<<fft->at(i0)<<" "<<fft->at(i1)<<" ";
					AddMod(fft->at(i0), fft->at(i0), z, ord);
					//cout<<fft->at(i0)<<" "<<fft->at(i1)<<" ";
					i0 = i0 + m2;
				}
				MulMod(rr ,rr,rho, ord) ;

        }
      //  cout<<endl;
    }

}

void fft::FFTinv(vector<ZZ>* ret, vector<ZZ>* points, long N, ZZ rootofunity, ZZ ord){
long  i;
ZZ s, omega;
	InvMod(omega, rootofunity, ord);
    InvMod(s,to_ZZ(N),ord);
    FFT(ret, points,N,omega,ord);
    for (i = 0; i< N; i++){
        MulMod(ret->at(i), s , ret->at(i), ord);
    }
}


void fft::fft_in(vector<ZZ>* ret, vector<ZZ>* v, ZZ rootofunity, ZZ ord, ZZ mod){
	int nb,t,t2,i,i0,i1,k,l;
	ZZ rho, rr, z;
	double two = 2;
	l = v->size();

	brevorder(ret, v);
	nb = NumBits(l)-1;

	for(k = 0 ;  k<nb; k++){
		t =  pow(two,k);
		t2 = 2*t;
		rr = 1;
		PowerMod(rho,rootofunity,pow(two, nb-k-1),ord);
		for(i = 0; i<t; i ++){
			i0 = i;
			if(rr != 1){
				while (i0 < l){
					i1=i0+t;
					PowerMod(z,ret->at(i1), rr, mod);
					MulMod(ret->at(i1),ret->at(i0), InvMod(z,mod),mod);
					MulMod(ret->at(i0),ret->at(i0), z, mod);
					i0 = i0 + t2;

				}
			}
			else{
				while (i0 < l){
					i1=i0+t;
					z = ret->at(i1);
					MulMod(ret->at(i1),ret->at(i0), InvMod(z,mod),mod);
					MulMod(ret->at(i0),ret->at(i0), z, mod);
					i0 = i0 + t2;

				}
			}
			 MulMod(rr, rr,rho,ord);
		}
	}
}


void fft::fft_sum_in(vector<ZZ>* ret, vector<ZZ>* v, ZZ rootofunity, ZZ ord){
	int nb,t,t2,i,i0,i1,k,l;
	ZZ rho, rr, z;
	double two = 2;
	l = v->size();

	brevorder(ret, v);
	nb = NumBits(l)-1;
	for(k = 0 ;  k<nb; k++){
		t =  pow(two,k);
		t2 = 2*t;
		rr = 1;
		PowerMod(rho,rootofunity,pow(two, nb-k-1),ord);
		for(i = 0; i<t; i ++){
			i0 = i;
			while (i0 < l){
				i1=i0+t;
				MulMod(z,ret->at(i1), rr, ord);
				SubMod(ret->at(i1),ret->at(i0), z, ord);
				AddMod(ret->at(i0),ret->at(i0), z, ord);
				i0 = i0 + t2;

			}
			 MulMod(rr,rr,rho,ord);
		}
	}
}


void fft::fft_mult_cipher(vector<vector<vector<ZZ>* >*>* ret, vector<vector<Cipher_elg>* >* v, ZZ rootofunity, ZZ ord, ZZ mod){

	vector<vector<ZZ>* >* fft = 0;
	vector<ZZ>* temp_u = 0;
	vector<ZZ>* temp_v = 0;
	long i,j, m, n,l;

	n= v->at(0)->size();
	m = v->size();
	l=2*m;

	for(i = 0; i<n; i++){
		fft =new vector<vector<ZZ>* >(2);
		temp_u = new vector<ZZ>(l);
		temp_v = new vector<ZZ>(l);
		temp_u ->at(0) = to_ZZ(1);
		temp_v ->at(0) = to_ZZ(1);
		for(j = 1; j <=m; j++){
			temp_u->at(j)=v->at(j-1)->at(i).get_u();
			temp_v->at(j)=v->at(j-1)->at(i).get_v();
		}
		for(j = m+1; j<l; j++){
			temp_u->at(j) = to_ZZ(1);
			temp_v->at(j) = to_ZZ(1);
		}
		fft->at(0)=new vector<ZZ>(l);
		fft->at(1)=new vector<ZZ>(l);

		fft_in(fft->at(0), temp_u, rootofunity, ord, mod);
		fft_in(fft->at(1), temp_v,rootofunity, ord, mod);
		ret->at(i)=fft;
		delete temp_u;
		delete temp_v;
	}
}


void fft::fft_matrix(vector<vector<ZZ>* >*  ret, vector<vector<ZZ>* >* T, ZZ rootofunity, ZZ ord){

	vector<ZZ>*  fft = 0;
	vector<ZZ>* temp = 0;
	long i,j, m, n,l;
	double two = 2;
	int e;
//	cout<<"in sumT "<< T->size();
	n= T->at(0)->size();
	m = T->size();
	//In the case of the Ciphertexts m is a power of 2, in the normal setting m is odd in our case
	if(m%2==0){
		e = NumBits(2*m+1)-	1;
	}
	else{
		e = NumBits(2*m+1);
	}
	l = pow(two, e);
	for(i = 0; i<n; i++){
		temp = new vector<ZZ>(m);
		for(j = 0; j <m; j++){
			temp->at(j)=T->at(j)->at(i);
		}
		fft=new vector<ZZ>(l);
		FFT(fft, temp, l, rootofunity, ord );
		//fft_sum_in(fft, temp, rootofunity, ord);
		ret->at(i)=fft;
		delete temp;
	}
//	cout<<endl;
}

void fft::fft_matrix_inv(vector<vector<ZZ>* >*  ret, vector<vector<ZZ>* >* T, ZZ rootofunity, ZZ ord){

	vector<ZZ>*  fft = 0;
	vector<ZZ>* temp = 0;
	long i,j, m, n,l;
	double two = 2;
	int e;
	//cout<<"in sumT "<< ret->size();
	//In the case of the Ciphertexts m is a power of 2, in the normal setting m is odd in our case
	n= T->at(0)->size();
	m = T->size();
	if(m%2==0){
		e = NumBits(2*m+1)-	1;
	}
	else{
		e = NumBits(2*m+1);
	}
	l = pow(two, e);
//	cout<<" l ist "<<l<<endl;
	for(i = 0; i<n; i++){
		temp = new vector<ZZ>(m);
		for(j = 0; j <m; j++){
			temp->at(j)=T->at(m-j-1)->at(i);
		}
		fft=new vector<ZZ>(l);
		FFT(fft, temp, l, rootofunity, ord );
		//fft_sum_in(fft, temp, rootofunity, ord);
		ret->at(i)=fft;
		delete temp;
	}
	//cout<<endl;
}


void fft::sum_t(vector<vector<ZZ>*>* ret, vector<vector<ZZ>* >* T, ZZ rootofunity, ZZ ord){
	vector<ZZ>* tem =  0;
	ZZ temp, t;
	long m,n,k,i,j,te,l;
	m = T->size();
	n = T->at(0)->size();
	l=2*m;
	for(k= 0; k<l; k++){
		tem = new vector<ZZ>(n);
		for(j = 0; j<n;j++){
			temp = 0;
			for (i = m-1; i>= 0; i--){
				PowerMod(t,rootofunity, i*(k+1),ord);
				te =fabs(i-m+1);
				MulMod(t,t,T->at(te)->at(j),ord);
				AddMod(temp, temp, t,ord);
			}
			tem->at(j) = temp;
		}
		ret->at(k) = tem;

	}
}



 void fft::calc_Pk(vector<vector<ZZ>*>* ret, vector<vector<Cipher_elg>* >* v, vector<vector<ZZ>* >* T, ZZ rootofunity, ZZ ord, ZZ mod, int omega_sw){
	long j,m,n,l;
	vector<vector<vector<ZZ>* >*>* fft_t=0;
	vector<vector<ZZ>* >* sumT=0;
	ZZ temp_u, te_u, temp_v, te_v,temp ;
	m = T->size();
	n= T->at(0)->size();
	vector<ZZ>* ret_u = new vector<ZZ>(2*m);
	vector<ZZ>* ret_v = new vector<ZZ>(2*m);

	fft_t = new vector<vector<vector<ZZ>* >*>(n);
	fft::fft_mult_cipher(fft_t, v,rootofunity, ord, mod);

	sumT = new vector<vector<ZZ>* >(n);
	fft::fft_matrix_inv(sumT, T, rootofunity, ord);

	l=2*m-1;
	for (j = 0; j<l; j++){

		multi_expo::multi_expo_LL(temp_u,fft_t, sumT, omega_sw ,j+1,0);
		multi_expo::multi_expo_LL(temp_v,fft_t, sumT, omega_sw ,j+1,1);
		ret_u->at(j) = temp_u;
		ret_v->at(j) = temp_v;
	}


	multi_expo::multi_expo_LL(temp_u ,fft_t, sumT, omega_sw ,0,0);
	multi_expo::multi_expo_LL(temp_v ,fft_t, sumT, omega_sw ,0,1);
	ret_u->at(l) = temp_u;
	ret_v->at(l) = temp_v;

	ret->at(0) = ret_u;
	ret->at(1) = ret_v;

	Functions::delete_vector(fft_t);
	Functions::delete_vector(sumT);

}

void fft::calc_m( vector<vector<ZZ>*>* M , long m, ZZ rootofunity, ZZ ord){
	vector<ZZ>* Rootofunity;
	vector<ZZ>* div;
	ZZ prod,temp,temp_1;
	long i,j,l;
	l=2*m;
	vector<ZZ>* Rootofun = new vector<ZZ>(l);
	div =new vector<ZZ>(l);

	Rootofun->at(0) = rootofunity;
	for(i = 1; i<l; i++){
		MulMod(Rootofun->at(i), Rootofun->at(i-1), rootofunity,ord);
	}
	for(i = 0 ; i<l; i++){
		prod = 1;
		for(j = 0; j<l; j++){
			if (i!=j){
				MulMod(prod,prod, SubMod(Rootofun->at(i),Rootofun->at(j),ord),ord);
			}
		}
		div->at(i)= prod;
	}
	for(i = 0; i<l; i++){
		Rootofunity = new vector<ZZ>(l);
		Rootofunity->at(2*m-1) = to_ZZ(1);
		PowerMod(temp,rootofunity, i+1,ord);

		for(j = l-2; j>=0; j--){
			 MulMod(Rootofunity->at(j),Rootofunity->at(j+1),temp,ord);
		}
		for(j = l-1; j>=0; j--){
			 MulMod(Rootofunity->at(j),Rootofunity->at(j), InvMod(div->at(i),ord),ord);
		}
		M->at(i) = Rootofunity;
	}
	delete Rootofun;
	delete div;
}




