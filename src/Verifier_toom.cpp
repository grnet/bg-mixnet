/*
 * Verifier_toom.cpp
 *
 *  Created on: 25.04.2011
 *      Author: stephaniebayer
 */

#include "Verifier_toom.h"

#include <vector>
#include <map>
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

extern G_q G;
extern G_q H;
extern Pedersen Ped;
extern ElGammal El;
extern long mu;
extern long mu_h;
extern long m_r;

Verifier_toom::Verifier_toom() {
	// TODO Auto-generated constructor stub

}


Verifier_toom::Verifier_toom(vector<vector<Cipher_elg>* >* cc,
				vector<vector<Cipher_elg>* >* CC,
				map<string, long> num) {
	c = cc;
	C = CC;
	// sets the values of the matrix according to the input
	m = num["ciphertext_matrix_rows"]; //number of rows
	n = num["ciphertext_matrix_columns"]; //number of columns
	N = num["number_of_ciphertexts"]; //added for non-interactive
	omega = num["window_size_multi_exponentiation_brickels"]; //windowsize for multi-expo-technique
	omega_sw = num["window_size_multi_exponentiation"]; //windowsize for multi-expo-technique sliding window and LL
	omega_LL = num["window_size_multi_exponentiation_lim_lee"]; //windowsize for multi-expo-technique of LL

	c_A = new vector<Mod_p>(m+1); //allocate the storage for the commitments of Y
	c_B = new vector<Mod_p>(m); //allocate the storage for the commitments of T
	c_B_small = new vector<Mod_p>(m_r);//commitments after reduction with challenges x
	C_small = new vector<vector<Cipher_elg>* >(m_r); //reduced Ciphertexte, with challenges x

	chal_x6 = new vector<ZZ>(2*m);// allocate the storage for the vector of Vandermonde challenges t, ... t^n
	chal_y6 = new vector<ZZ>(n);// allocate the storage for the vector of Vandermonde challenges t, ... t^n
	chal_x8 = new vector<ZZ>(2*m +1); //challenge from round 8
	basis_chal_x8 = new vector<vector<long>*>(2*m+2);//basis of vector e for multi-expo technique
	mul_chal_x8 = new vector<ZZ>(2*m+2); //shifted vector e, e(0) = 1, used for multi-expo
	x = new vector<ZZ>(mu_h); //challenges to reduce ciphertexts


	//Commitments vectors
	c_Dh= new vector<Mod_p>(m); //commitments to the matrix W
	c_Ds= new vector<Mod_p>(m+1); //contains a_W*t_1
	c_Dl= new vector<Mod_p>(2*m+1); //commitments to the values Cl
	c_a_c= new vector<Mod_p>(mu_h); //commitment to the values used to reencrypt the E_x
	c_a= new vector<Mod_p>(2*mu); //commitment to the values in the matrix a
	//Vector of product of the diagonals of permuted Ciphertexts from round 5
	E = new vector<Cipher_elg>(2*mu);
	C_c = new vector<Cipher_elg>(mu_h); //Ciphertexts to prove correctness of reduction


	D_h_bar = new vector<ZZ>(n);//sum over the rows in D_h
	d_bar = new vector<ZZ>(n);// chal_x8*D_h(m-1) +d
	Delta_bar = new vector<ZZ>(n);//chal_x8*d_h+Delta
	B_bar  = new vector<ZZ>(n); // sum over the rows in B multiplied by chal^i

	A_bar = new vector<ZZ>(n); //sum over the rows in Y times the challenges
	Ds_bar = new vector<ZZ>(n); // sum over the rows in U times thes challenges
}


Verifier_toom::~Verifier_toom() {

	delete c_A;
	delete c_B;
	delete chal_x6;
	delete chal_y6;
	delete chal_x8;
	delete mul_chal_x8;
	Functions::delete_vector(basis_chal_x8);
	delete x;
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

	delete c_B_small;
	delete c_a_c;
	delete C_c;
}


string Verifier_toom::round_2(string in_name){
	long i;
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
	
	/* instead of the interactive version:
	 * chal_x2 = RandomBnd(ord);
         * hash the c_A matrix to get a random value.
	 */
	chal_x2 = func_ver::hash_cipher_Pedersen_ElGammal(
						 c, C, // ciphertexts
						 n, // Pedersen
						 omega, omega_LL, omega_sw,
						 m, N, // ElGammal
						 c_A); // commitment of A
	cout << "chal_x2 hash: " << chal_x2 << endl;
	cout << "hash input: c, C, n, omega, omega_LL, omega_sw, m, N, c_A." 
	<< endl;


	name = "round_2 ";
	name = name + ctime(&rawtime);
	ofstream ost(name.c_str());
	ost << chal_x2;

	return name;
}

string Verifier_toom::round_4(string in_name){
	long i;
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

	// non-interactive
	chal_z4 = func_ver::hash_chal_x2_c_B(chal_x2, c_B);
	cout << "chal_z4 hash: " << chal_z4 << endl;
	cout << "hash input: chal_x2, c_B." << endl;

	// non-interactive
	chal_y4 = func_ver::hash_chal_z4(chal_z4);
	cout << "chal_y4 hash: " << chal_y4 << endl;
	cout << "hash input: chal_z4." << endl;

	ofstream ost(name.c_str());
	ost<< chal_z4<<"\n";
	ost<<chal_y4 ;
	return name;
}

string Verifier_toom::round_6(string in_name){
	ZZ tem;
	ZZ ord = H.get_ord();
	long i,l;
	string name;
	time_t rawtime;
	time ( &rawtime );

	//reads the values out of the file name
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	ist>>c_z;
	for (i = 0; i<m; i++){
		ist >>c_Dh->at(i);
	}
	for(i=0;i<mu_h;i++){
		ist>>C_c->at(i);
	}
	for(i=0; i<mu_h; i++){
		ist>>c_a_c ->at(i);
	}

	//sets the vector t to the values temp, temp^2,...
	func_ver::hash_fill_commits_cipher(ZZ(INIT_VAL, 0), c_Dh, c_a_c, C_c, 
								chal_x6);
	cout << "chal_x6 hash: " << chal_x6->at(0) << endl;
	cout << "hash input: 0, c_Dh, c_a_c, C_c." << endl;

	//sets the vector t to the values temp, temp^2,...
	func_ver::hash_fill_chals(chal_y4, chal_x6->at(0), chal_y6);
	cout << "chal_y6 hash: " << chal_y6->at(0) << endl;
	cout << "hash input: chal_y4, chal_x6" << endl;

	name = "round_6 ";
	name = name + ctime(&rawtime);
	l=2*m;
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

string Verifier_toom::round_6_red(string in_name,vector<vector<Cipher_elg>* >* enc){
	long i;
	ZZ tem;
	ZZ ord = H.get_ord();
	Cipher_elg c;
	Mod_p temp;
	string name;
	time_t rawtime;
	time ( &rawtime );


	//reads the values out of the file name
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	for(i=0;i<mu_h;i++){
		ist>>C_c->at(i);
	}
	for(i=0; i<mu_h; i++){
		ist>>c_a_c ->at(i);
	}

	//calculate the value of c
	calculate_c(c, enc);

	temp = Mod_p(1,H.get_mod()); //a_a_c->at(mu-1) should equal the commitment to 0
	if(c_a_c->at(mu-1)==temp & c == C_c->at(mu-1)){
		//sets the vector x to the values temp, temp^2,...
		func_ver::hash_fill_commits_cipher(ZZ(INIT_VAL, 0), NULL, c_a_c, 									C_c, x);
		cout << "x hash: " << x->at(0) << endl;
		cout << "hash input: 0, c_a_c, C_c." << endl;

		//sets the vector x to the values temp, temp^2,...
		func_ver::fill_vector(x);
	}
	name = "round_6 ";
	name = name + ctime(&rawtime);

	ofstream ost(name.c_str());
	for (i=0; i<mu_h; i++){
		ost << x->at(i)<<" ";
	}
	return name;

}

string Verifier_toom::round_6_red1(string in_name){
	long i,l;
	Mod_p temp, com;
	Cipher_elg C;
	ZZ mod = G.get_mod();
	string name;
	time_t rawtime;
	time ( &rawtime );


	//calculates the product of the the old commitments a_a_c to the power of x
	calculate_ac(com);

	//Combines the committed values to B in the vector c_B_small with challenges x
	reduce_c_B();

	//calulates the new value C
	calculate_C(C, C_c, x);

	//reads the values out of the file name
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	ist>>c_z;
	for (i = 0; i<m; i++){
		ist >>c_Dh->at(i);
	}
	for(i=0;i<mu_h;i++){
		ist>>C_c->at(i);
	}
	for(i=0; i<mu_h; i++){
		ist>>c_a_c ->at(i);
	}
	ist>>a_c_bar;
	ist>>r_ac_bar;

	temp = Mod_p(1,mod);//a_a_c->at(mu-1) should equal the commitment to 0
	if(c_a_c->at(mu-1)==temp  & com == Ped.commit(a_c_bar, r_ac_bar)& C == C_c->at(mu-1)){
		//sets the vector chal_x6 to the values temp, temp^2,...
		func_ver::hash_fill_commits_cipher(ZZ(INIT_VAL, 0), c_Dh, 
							c_a_c, C_c, chal_x6);
		cout << "chal_x6 hash: " << chal_x6->at(0) << endl;
		cout << "hash input: 0, c_Dh, c_a_c, C_c." << endl;

		//sets the vector chal_y6 to the values temp, temp^2,...
		func_ver::hash_fill_chals(chal_y4, chal_x6->at(0), chal_y6);
		cout << "chal_y6 hash: " << chal_y6->at(0) << endl;
		cout << "hash input: chal_y4, chal_x6" << endl;
	}

	name = "round_6 ";
	name = name + ctime(&rawtime);

	l=2*m;
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

string  Verifier_toom::round_8(string in_name){
	long i,l;
	string name;
	time_t rawtime;
	time ( &rawtime );
	//reads the values out of the file name

	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;

	for (i = 0; i<=2*m; i++){
		ist >> c_Dl->at(i) ;
	}
	ist>>c_D0;
	ist>>c_Dm;
	ist>>c_d;
	ist>>c_Delta;
	ist>>c_dh;
	ist>>a_c_bar;
	ist>>r_ac_bar;
	for(i=0; i<8; i++){
		ist>>E->at(i);
	}
	ist>>c_B0;
	for(i=0; i<8; i++){
		ist>>c_a->at(i);
	}

	func_ver::hash_fill_x8(chal_x6->at(0), c_a, E, chal_x8, 
				basis_chal_x8, mul_chal_x8, omega);
	cout << "chal_x8 hash: " << chal_x8->at(0) << endl;
	cout << "hash input: chal_x6, c_a, E." << endl;

	l= chal_x8->size();

	name = "round_8 ";
	name = name + ctime(&rawtime);
	ofstream ost(name.c_str());
	for (i = 0; i<l; i++){
		ost << chal_x8->at(i)<< " ";
	}
	return name;
}

ZZ Verifier_toom:: round_10(string in_name,vector<vector<Cipher_elg>* >* enc, vector<vector<Cipher_elg>* >* C){
	int b;
	long i;
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

	for (i = 0; i<n; i++){
		ist >>B_bar->at(i);
	}
	ist>> r_B_bar;
	ist >>a_bar;
	ist >> r_a_bar;
	ist >> rho_bar;


	//Check that the D_hi's are constructed correctly
	b=func_ver::check_Dh_op(c_Dh, mul_chal_x8, D_h_bar, r_Dh_bar, omega_LL);
	if(b==1){
		//Check that matrix D is constructed correctly
		b=func_ver::check_D_op(c_D0, c_z, c_A, c_B, chal_x8, chal_y4, A_bar, r_A_bar, n);
		if(b==1){
			//Check that D_s is constructed correctly
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
							//Check that the commitments a_T contain the right values
							b=check_B();
							if(b==1){
								//Check that the reecncryption was done correctly
								b=check_a();
								if(b==1){
									//Check that E_c->at(mu-1) contains c and D->at(4) = C
									b = check_c(enc); //Both commitments shoud be com(0,0)
									if(b==1 & c_a->at(4)==c_a_c->at(3)){
										//Check correctness of the chiphertexts
										b =check_E(C);
										if(b==1 ){
											//Check the the reencryption of the E_c is correct
											b = check_ac();
											if(b==1){
												cout << "Accept!" << endl;
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
	}
	return to_ZZ(-1);
}

ZZ Verifier_toom:: round_10_red(string in_name,vector<vector<Cipher_elg>* >* enc, vector<vector<Cipher_elg>* >* C){
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

	for (i = 0; i<n; i++){
		ist >>B_bar->at(i);
	}
	ist>> r_B_bar;
	ist >>a_bar;
	ist >> r_a_bar;
	ist >> rho_bar;
	//Check that the Dhi's are constructed correctly
	b=func_ver::check_Dh_op(c_Dh, mul_chal_x8, D_h_bar, r_Dh_bar, omega_sw);
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
							b=check_B_red();
							if(b==1){
								//Check that the reecncryption was done correctly
								b=check_a();
								if(b==1){
									// D->at(4) = C
									b = check_c_red(); //Both commitments shoud be com(0,0)
										if(b==1 & c_a->at(4)==c_a_c->at(3)){
										//Check correctness of the ciphertexts
										b =check_E_red(C);
										if(b==1 ){
											//Check the the reencryption of the c_c is correct
											b = check_ac();
											if(b==1){
												cout << "Accept!" << endl;
											}
											else{
												return to_ZZ(-1);
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
	}
	return to_ZZ(-1);
}


void Verifier_toom::calculate_c(Cipher_elg& c, vector<vector<Cipher_elg>* >* enc){
	long i, j;
	Cipher_elg temp;
	ZZ chal_temp;
	ZZ ord = H.get_ord();
	vector<ZZ>* v_chal=0;

	chal_temp=to_ZZ(1);
	c=Cipher_elg(1,1,H.get_mod());
	v_chal = new vector<ZZ>(n);
	for(i=0; i<m ; i++){
		for(j=0; j<n; j++){
			MulMod(chal_temp, chal_temp, chal_x2, ord);
			v_chal->at(j)=chal_temp;
		}
		multi_expo::expo_mult(temp, enc->at(i), v_chal, omega);
		Cipher_elg::mult(c,c,temp);
	}

	delete v_chal;
}

void Verifier_toom::calculate_ac(Mod_p& com){
	long i;
	Mod_p temp;

	com = c_a_c->at(0);
	for(i=1; i<mu_h; i++){
		Mod_p::expo(temp,c_a_c->at(i), x->at(i-1));
		Mod_p::mult(com, com, temp);
	}
}

void Verifier_toom::reduce_c_B(){
	long i,j;
	Mod_p temp, temp_1;

	for(i = 0; i<4*m_r; i++){
		temp = c_B->at(4*i);
		for(j = 1; j<mu; j++){
			Mod_p::expo(temp_1, c_B->at(4*i+j), x->at(j-1));
			Mod_p::mult(temp, temp, temp_1);
		}
		c_B_small->at(i)=temp;
	}
}

void Verifier_toom::calculate_C(Cipher_elg& C, vector<Cipher_elg>* C_c, vector<ZZ>* x){
	long i;
	ZZ t_1;
	ZZ ord = H.get_ord();
	Mod_p temp;
	Mod_p gen = H.get_gen();
	Cipher_elg temp_1;

	NegateMod(t_1, a_c_bar, ord);
	Mod_p::expo(temp, gen, t_1);
	C = El.encrypt(temp,to_ZZ(0));
	Cipher_elg::mult(C,C, C_c->at(0));
	for(i=1; i<mu_h; i++){
		Cipher_elg::expo(temp_1, C_c->at(i), x->at(i-1));
		Cipher_elg::mult(C,C, temp_1);
	}

}

int Verifier_toom::check_B(){
	long i,j;
	Mod_p temp, temp_1, t_B, co_B;
	ZZ mod = G.get_mod();
	vector<Mod_p>* c_B_small = new vector<Mod_p>(5);
	vector<Mod_p>* c_B_temp = new vector<Mod_p>(4);

	c_B_small->at(0)= c_B0;
	for(i = 0; i<m_r; i++){
		temp =  c_B->at(4*i);
		for(j = 1; j<4; j++){
			Mod_p::expo(temp_1, c_B->at(4*i+j), chal_x6->at(j-1));
			Mod_p::mult(temp, temp, temp_1);
		}
		c_B_small->at(i+1)=temp;
	}
	t_B = c_B_small->at(0);
	for(i=1; i<5;i++){
		Mod_p::expo(temp, c_B_small->at(i), chal_x8->at(i-1));
		Mod_p::mult(t_B, t_B, temp);
	}

	delete c_B_temp;
	delete c_B_small;

	co_B= Ped.commit_opt(B_bar,r_B_bar);
//	cout<<"B "<<t_B<<" "<<co_B<<endl;
	if(t_B == co_B){
		return 1;
	}
	return 0;
}

int Verifier_toom::check_B_red(){
	long i,j;
	Mod_p temp, temp_1, t_B, co_B;
	ZZ mod = G.get_mod();
	vector<Mod_p>* c_B_temp = new vector<Mod_p>(m_r+1);

	c_B_temp->at(0)= c_B0;
	for(i = 0; i<m_r; i++){
		temp =  c_B_small->at(4*i);
		for(j = 1; j<mu; j++){
			Mod_p::expo(temp_1, c_B_small->at(4*i+j), chal_x6->at(j-1));
			Mod_p::mult(temp, temp, temp_1);
		}
		c_B_temp->at(i+1)=temp;
	}
	t_B = c_B_temp->at(0);
	for(i=1; i<m_r+1;i++){
		Mod_p::expo(temp, c_B_temp->at(i), chal_x8->at(i-1));
		Mod_p::mult(t_B, t_B, temp);
	}
	delete c_B_temp;
	co_B= Ped.commit_opt(B_bar,r_B_bar);
//	cout<<"B "<<t_B<<endl;
//	cout<<"B "<<co_B<<endl;

	if(t_B == co_B){
		return 1;
	}
	return 0;
}


int Verifier_toom::check_a(){
	long i;
	Mod_p t_a, co_a;
	vector<ZZ>* chal_temp = new vector<ZZ>(8);

	chal_temp->at(0)=1;
	for(i=1; i<8; i++){
		chal_temp->at(i)=chal_x8->at(i-1);
	}
	multi_expo::multi_expo_LL(t_a, c_a, chal_temp, omega_sw);
	co_a = Ped.commit_sw(a_bar, r_a_bar);

	//cout<<"a "<<t_a<<" "<<co_a<<" "<<c_a->at(4)<<endl;
	delete chal_temp;
	if(t_a == co_a){
		return 1;
	}
	return 0;
}


int Verifier_toom::check_c(vector<vector<Cipher_elg>* >* enc){
	Cipher_elg c,C;

	calculate_c(c, enc);

	calculate_C(C,C_c, chal_x6);
//	cout<<"C "<<C_c->at(mu-1)<<" "<<c<<endl;
//	cout<<"C "<<E->at(4)<<" "<<C<<endl;
 	if(C_c->at(mu-1)==c & E->at(4)==C){
 		return 1;
 	}
 	return 0;
}

int Verifier_toom::check_c_red(){
	Cipher_elg C;

	calculate_C(C,C_c, chal_x6);
//	cout<<C<<endl;
//	cout<<E->at(4)<<endl;
 	if(E->at(4)==C){
 		return 1;
 	}
 	return 0;
}


int Verifier_toom::check_E(vector<vector<Cipher_elg>* >* C){
	long i,j;
	Mod_p temp;
	Mod_p gen = H.get_gen();
	Cipher_elg temp_1,  temp_2, t_D, c_D;
	vector<ZZ>* chal_1_temp= new vector<ZZ>(4);
	vector<ZZ>* chal_2_temp= new vector<ZZ>(4);
	vector<Cipher_elg>* row_C;

	for(i = 0; i<3; i++){
		chal_1_temp->at(i) = chal_x6->at(2-i);
	}
	chal_1_temp->at(3) = 1;

	for(i=0; i<m_r;i++){
		row_C = new vector<Cipher_elg>(n);
		for(j=0; j<n; j++){
			multi_expo::multi_expo_LL(row_C->at(j), C->at(4*i)->at(j), C->at(4*i+1)->at(j),C->at(4*i+2)->at(j),C->at(4*i+3)->at(j), chal_1_temp, omega_sw);
		}
		C_small->at(i)=row_C;
	}

	for(i=0; i<3;i++){
		chal_2_temp->at(i)=chal_x8->at(2-i);
	}
	chal_2_temp->at(3)=to_ZZ(1);


	Mod_p::expo(temp, gen,a_bar);
	temp_1 = El.encrypt(temp, rho_bar);
	multi_expo::expo_mult(temp_2, C_small, chal_2_temp, B_bar, omega);
	Cipher_elg::mult(c_D,temp_1,temp_2);
	//c_D=temp_1*temp_2;

	multi_expo::expo_mult(t_D, E, basis_chal_x8 , omega);

	delete chal_1_temp;
	delete chal_2_temp;
	Functions::delete_vector(C_small);
//	cout<<"E"<<t_D<<endl;
//	cout<<"E"<<c_D<<endl;
	if(t_D==c_D){
		return 1;
	}
	return 0;
}

int Verifier_toom::check_E_red(vector<vector<Cipher_elg>* >* C){
	long i,j,l;
	Mod_p temp;
	Mod_p gen = H.get_gen();
	Cipher_elg temp_1,  temp_2, t_D, c_D;
	vector<ZZ>* x_temp= new vector<ZZ>(4);
	vector<ZZ>* chal_1_temp= new vector<ZZ>(4);
	vector<ZZ>* chal_2_temp= new vector<ZZ>(4);
	vector<vector<Cipher_elg>* >* C_small_temp=0;
	vector<Cipher_elg>* row_C;

	for(i = 0; i<3; i++){
		x_temp->at(i) = x->at(2-i);
	}
	x_temp->at(3) = 1;

	l=mu*m_r;
	for(i=0; i<l;i++){
		row_C = new vector<Cipher_elg>(n);
		for(j=0; j<n; j++){
			multi_expo::multi_expo_LL(row_C->at(j), C->at(4*i)->at(j), C->at(4*i+1)->at(j),C->at(4*i+2)->at(j),C->at(4*i+3)->at(j), x_temp, omega_sw);
		}
		C_small->at(i)=row_C;
	}

	for(i = 0; i<3; i++){
		chal_1_temp->at(i) = chal_x6->at(2-i);
	}
	chal_1_temp->at(3) = 1;

	C_small_temp = new vector<vector<Cipher_elg>* >(m_r);
	for(i=0; i<m_r;i++){
		row_C = new vector<Cipher_elg>(n);
		for(j=0; j<n; j++){
			multi_expo::multi_expo_LL(row_C->at(j), C_small->at(4*i)->at(j), C_small->at(4*i+1)->at(j),C_small->at(4*i+2)->at(j),C_small->at(4*i+3)->at(j), chal_1_temp, omega);
		}
		C_small_temp->at(i)=row_C;
	}


	for(i=0; i<3;i++){
		chal_2_temp->at(i)=chal_x8->at(2-i);
	}
	chal_2_temp->at(3)=to_ZZ(1);

	Mod_p::expo(temp, gen,a_bar);
	temp_2 = El.encrypt(temp, rho_bar);
	multi_expo::expo_mult(temp_1, C_small_temp, chal_2_temp, B_bar, omega);
	Cipher_elg::mult(c_D,temp_1,temp_2);
	//c_D=temp_2*temp_1;

	multi_expo::expo_mult(t_D, E, basis_chal_x8 , omega);
	Functions::delete_vector(C_small);
	Functions::delete_vector(C_small_temp);
	delete chal_2_temp;
	delete chal_1_temp;
	delete x_temp;
	//cout<<"E"<<t_D<<endl;
	//cout<<"E"<<c_D<<endl;
	if(t_D ==c_D){
		return 1;
	}
	return 0;
}

int Verifier_toom::check_ac(){
	Mod_p t_a_c, co_a_c, temp;
	int i;

	t_a_c = c_a_c->at(0);
	for(i=1; i<7; i++){
		Mod_p::expo(temp, c_a_c->at(i), chal_x6->at(i-1));
		Mod_p::mult(t_a_c,t_a_c, temp);
	}
	co_a_c = Ped.commit_sw(a_c_bar, r_ac_bar);
//	cout<<"ac "<<t_a_c<<" "<<c_a_c;
	if(t_a_c ==co_a_c){
		return 1;
	}
	return 0;
}

