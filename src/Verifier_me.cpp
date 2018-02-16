/*
 * Verifier_me.cpp
 *
 *  Created on: 16.02.2011
 *      Author: stephaniebayer
 */

#include "Verifier_me.h"
#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Functions.h"
#include "ElGammal.h"
#include "multi_expo.h"
#include "func_ver.h"
#include <fstream>

#include <time.h>
#include <NTL/ZZ.h>
NTL_CLIENT

//extern G_q G;
extern G_q H;
//extern Pedersen Ped;
//extern ElGammal El;
//extern double time_rw_v;


Verifier_me::Verifier_me() {
	// TODO Auto-generated constructor stub

}



Verifier_me::Verifier_me(map<string, long> num) {
	// sets the values of the matrix according to the input
	m = num["ciphertext_matrix_rows"]; //number of rows
	n = num["ciphertext_matrix_columns"]; //number of columns
	omega = num["window_size_multi_exponentiation_brickels"]; //windowsize for multi-expo-technique
	omega_sw = num["window_size_multi_exponentiation"]; //windowsize for multi-expo-technique sliding window
	omega_LL = num["window_size_multi_exponentiation_lim_lee"]; //windowsize for multi-expo-techniqueof Lim and Lee

	c_A = new vector<Mod_p>(m+1); //allocate the storage for the commitments of Y
	c_B = new vector<Mod_p>(m); //allocate the storage for the commitments of T
	chal_x6 = new vector<ZZ>(2*m);// allocate the storage for the vector of Vandermonde challenges t, ... t^n
	chal_y6 = new vector<ZZ>(n);// allocate the storage for the vector of Vandermonde challenges t, ... t^n
	//allocates the storage needed for the vector e
	chal_x8 = new vector<ZZ>(2*m +1); //challenge from round 8
	basis_chal_x8 = new vector<vector<long>*>(2*m+2);//basis of vector e for multi-expo technique
	mul_chal_x8 = new vector<ZZ>(2*m+2); //shifted vector e, e(0) = 1, used for multi-expo

	//Commitments vectors
	c_Dh= new vector<Mod_p>(m); //commitments to the matrix W
	c_Ds= new vector<Mod_p>(m+1); //contains a_W*t_1
	c_Dl= new vector<Mod_p>(2*m+1); //commitments to the values Cl
	c_a= new vector<Mod_p>(2*m); //commitment to the values in the matrix a
	//Vector of product of the diagonals of permuted Ciphertexts from round 5
	E = new vector<Cipher_elg>(2*m);


	D_h_bar = new vector<ZZ>(n);//sum over the rows in D_h
	d_bar = new vector<ZZ>(n);// chal_x8*D_h(m-1) +d
	Delta_bar = new vector<ZZ>(n);//chal_x8*d_h+Delta
	B_bar  = new vector<ZZ>(n); // sum over the rows in B multiplied by chal^i

	A_bar = new vector<ZZ>(n); //sum over the rows in Y times the challenges
	Ds_bar = new vector<ZZ>(n); // sum over the rows in U times thes challenges

}


Verifier_me::~Verifier_me() {

	delete c_A;
	delete c_B;
	delete chal_x6;
	delete chal_y6;
	delete chal_x8;
	delete mul_chal_x8;
	Functions::delete_vector(basis_chal_x8);
	delete c_Dh;
	delete c_Ds;
	delete c_Dl;
	delete c_a;
	delete E;
	delete D_h_bar;
	delete d_bar;
	delete Delta_bar;
	delete B_bar;
	delete A_bar;
	delete Ds_bar;
}


string Verifier_me::round_2(string in_name){
	long i;
	Mod_p temp;
	string name;
	ZZ ord=H.get_ord();
	time_t rawtime;
	time ( &rawtime );
	//sets a_Y to the values in the file name
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	for (i = 0; i<m; i++){
		ist >> c_A->at(i);
	}
	chal_x2 = RandomBnd(ord);

	name = "round_2 ";
	name = name + ctime(&rawtime);
	ofstream ost(name.c_str());

	ost<<chal_x2;

	return name;
}

string Verifier_me::round_4(string in_name){
	ZZ temp; //temporary value to construct the vector t
	long i;
	Mod_p tem;
	ZZ ord = H.get_ord();
	string name;
	time_t rawtime;
	time ( &rawtime );
	 //sets a_T to the values in the file
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	for (i = 0; i<m; i++){
		ist >> c_B->at(i);
	}

	//Set name of the output file and open stream
	name = "round_4 ";
	name = name + ctime(&rawtime);

	chal_z4 = RandomBnd(ord); //choose value alpha at random
	chal_y4 = RandomBnd(ord); //choose value lamda at random

	ofstream ost(name.c_str());
	ost<< chal_z4<<"\n";
	ost<<chal_y4;
	return name;
}

string Verifier_me::round_6(string in_name){
	ZZ tem;
	ZZ ord = H.get_ord();
	long i,l;
	Mod_p temp;
	string name;
	time_t rawtime;
	time ( &rawtime );

	l=2*m;
	//reads the values out of the file name
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	ist>>c_z;
	for (i = 0; i<m; i++){
		ist >>c_Dh->at(i);
	}
	ist>> c_B0 ;
	for (i = 0; i<l; i++){
		ist >>c_a->at(i);
	}
	for (i = 0; i<l; i++){
		ist >>E->at(i);
	}

	func_ver::fill_vector(chal_x6);
	func_ver::fill_vector(chal_y6);

	name = "round_6 ";
	name = name + ctime(&rawtime);

	ofstream ost(name.c_str());
	for (i=0; i<l; i++){
		ost << chal_x6->at(i)<<" ";
	}
	ost << "\n";
	for (i=0; i<n; i++){
		ost << chal_y6->at(i)<<" ";
	}
	ost << "\n";

	return name;

}

string  Verifier_me::round_8(string in_name){
	long l;;
	long i;

	Mod_p temp;
	Cipher_elg tem;
	string name;
	time_t rawtime;
	time ( &rawtime );
	//reads the values out of the file name

	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	l=2*m;
	for (i = 0; i<=l; i++){
		ist >> c_Dl->at(i) ;
	}
	ist>>c_D0;
	ist>>c_Dm;
	ist>>c_d;
	ist>>c_Delta;
	ist>>c_dh;
	for (i = 0; i<n; i++){
		ist >> B_bar->at(i);
	}
	ist>> r_B_bar;
	ist >>a_bar;
	ist >> r_a_bar;
	ist >> rho_bar;

	//sets chal_x8 as Vandermode vector with value chal
	func_ver::fill_x8(chal_x8, basis_chal_x8, mul_chal_x8, omega);
	l=chal_x8->size();
	name = "round_8 ";
	name = name + ctime(&rawtime);
	ofstream ost(name.c_str());
	for (i = 0; i<l; i++){
		ost << chal_x8->at(i)<< " ";
	}
	return name;

}


ZZ Verifier_me:: round_10(string in_name,vector<vector<Cipher_elg>* >* enc, vector<vector<Cipher_elg>* >* C){
	int b;
	long i;
	string name;
	time_t rawtime;
	time ( &rawtime );

	//reads the values out of the file name
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	for (i = 0; i<n; i++){
		ist >>D_h_bar->at(i);
	}
	ist>>r_Dh_bar;
	for(i=0; i<n; i++){
		ist>>d_bar->at(i);
	}
	ist>> r_d_bar;
	for(i=0; i<n; i++){
		ist>> Delta_bar->at(i);
	}
	ist>>r_Delta_bar;

	for (i = 0; i<n; i++){
		ist >>A_bar->at(i);
	}
	ist>>r_A_bar;
	for(i=0; i<n; i++){
		ist>>Ds_bar->at(i);
	}
	ist>>r_Ds_bar;
	ist>>r_Dl_bar;

	//Check that the Dhi's are constructed correctly
	b=func_ver::check_Dh_op(c_Dh, mul_chal_x8, D_h_bar, r_Dh_bar, omega_LL);
	if(b==1){
		//Check that matrix D is constructed correctly
		b=func_ver::check_D_op(c_D0, c_z, c_A, c_B, chal_x8, chal_y4, A_bar, r_A_bar, n);
		if(b==1){
			//Check that Ds is constructed correctly
			b=func_ver::check_Ds_op(c_Ds, c_Dh, c_Dm, chal_x6, chal_x8, Ds_bar, r_Ds_bar);
			if(b==1){
				//Check that the Dl's are correct
				b=func_ver::check_Dl_op(c_Dl, chal_x8, A_bar, Ds_bar, chal_y6, r_Dl_bar);
				if(b==1){
					//Check that vector d was constructed correctly
					b=func_ver::check_d_op(c_Dh, c_d, chal_x8, d_bar, r_d_bar);
					if(b==1){
						//Check that Deltas are constructed correctly
						b=func_ver::check_Delta_op(c_dh, c_Delta, chal_x8, Delta_bar, d_bar, r_Delta_bar, chal_x2, chal_z4, chal_y4);
						if(b==1 ){
							//Check that the commitments c_B contain the right values
							b=func_ver::check_B_op(c_B, c_B0, chal_x6, B_bar, r_B_bar, omega_LL);
							if(b==1){
								//Check that the reecncryption was done correctly
								b=func_ver::check_a_op(c_a, c_Dl, chal_x6, a_bar, r_a_bar);
								if(b==1){
									//Check that E->at(m) contains c
									b = func_ver::check_c_op(enc, E, chal_x2, omega);
									if(b==1){
										//Check correctness of the ciphertexts
										b =func_ver::check_E_op(C, E, chal_x6, B_bar, a_bar, rho_bar, omega);
										if(b==1 ){
											//cout<<"Accept";
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return to_ZZ(-1);
}




