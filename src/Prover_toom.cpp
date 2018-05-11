/*
 * Prover_toom.cpp
 *
 *  Created on: 24.04.2011
 *      Author: stephaniebayer
 */

#include "Prover_toom.h"
#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "ElGammal.h"
#include "multi_expo.h"
#include "func_pro.h"
#include <fstream>
#include <time.h>
#include <sstream>
#include <pthread.h>
#include "FakeZZ.h"
#include <chrono>
NTL_CLIENT

//extern G_q G;
extern G_q H;
//extern ElGammal El;
extern long mu;
extern long mu_h;

//OpenMP config
extern bool parallel;
extern int num_threads;

//extern long m_r;

using namespace std::chrono;


Prover_toom::Prover_toom(long& m_r_) : m_r(m_r_) {
// TODO remove containing stack -- this is never called
}

Prover_toom::Prover_toom(long& mr, vector<vector<Cipher_elg>* >* Cin,vector<vector<ZZ>*>* Rin, vector<vector<vector<long>* >* >* piin, vector<long> num, int m_in, int n_in, ElGammal* elgammal) : m_r(mr), elgammal_(elgammal), ped_(n_in) {

	// set the dimensions of the row and columns according to the user input
	m = m_in; //number of rows
	n = n_in; //number of columns
	C = Cin; //sets the reencrypted chipertexts to the input
	R = Rin; //sets the random elements to the input
	pi = piin; // sets the permutation to the input
	omega_mulex = num[3]; //windowsize for sliding-window technique
	omega_sw = num[4]; //windowsize for multi-expo technique
	omega_LL = num[7]; //windowsize for multi-expo technique

	ped_.set_omega(omega_mulex, omega_LL, omega_sw);
	//Creates the matrices A
	A = new vector<vector<ZZ>* >(m);
	func_pro::set_A(A,pi, m, n);


	//Allocate the storage needed for the vectors
	chal_x6 = new vector<ZZ>(2*m); //x6, x6^2, ... challenges from round 6
	chal_y6 = new vector<ZZ>(n); //y6, y6^2, ... challenges form round 6
	chal_x8 = new vector<ZZ>(2*m +1); //x8, x8^2, ... challenges from round 8

	//Allocate the storage needed for the vectors
	c_A = new vector<Mod_p>(m+1); //commitments to the rows in A
	r_A = new vector<ZZ>(m+1); //random elements used for the commitments

	D = new vector<vector<ZZ>* >(m+1); //vector containing in the first row random values and in all others y*A(ij) + B(ij)-z
	D_h = new vector<vector<ZZ>* >(m); //Vector of the Hadamare products of the rows in D
	D_s = new vector<vector<ZZ>* >(m+1); //Shifted rows of D_h
	d = new vector<ZZ>(n); //containing random elements to proof product of D_hm
	Delta = new vector<ZZ>(n); //containing random elements to proof product of D_hm
	d_h = new vector<ZZ>(n); // vector containing the last row of D-h

	r_D_h = new vector<ZZ>(m);//random elements for commitments to D_h
	c_D_h = new vector<Mod_p>(m+2);//commitments to the rows in D_h
	C_small = new vector<vector<Cipher_elg>* >(m_r); //matrix of reduced ciphertexts

	B = new vector<vector<ZZ>* >(m);//matrix of permuted exponents, exponents are x2^i, i=1, ..N
	basis_B = new vector<vector<vector<long>* >* >(m); //basis for the multi-expo, containing the Bij
	B_small = new vector<vector<ZZ>* >(m_r); //matrix of reduced exponents
	B_0 = new vector<ZZ>(n); //vector containing random exponents
	basis_B0 = new vector<vector<long>* >(n); //basis for multi-expo, containing  the B0j
	r_B = new vector<ZZ>(m); //random elements used to commit to B
	r_B_small = new vector<ZZ>(m_r); //random elements for commitments to B_small
	c_B = new vector<Mod_p>(m); //vector of commitments to rows in T

	C_c = new vector<Cipher_elg>(mu_h);  //Ciphertexts to prove correctness of reduction
	c_a_c= new vector<Mod_p>(mu_h); //vector containing the commitments to value used for the reencryption of E_c
	a_c= new vector<ZZ>(mu_h); //vector containing the values used for reecnrcyption
	r_c= new vector<ZZ>(mu_h); //random elements used to commit to a_c
	rho_c = new vector<ZZ>(mu_h); //random elements used in the reencryption

	a = new vector<ZZ>(2*mu); //elements used for reencryption in round 5
	r_a = new vector<ZZ>(2*mu); //random elements to commit to elements in a
	c_a = new vector<Mod_p>(2*mu); //commitments to elements a
	E = new vector<Cipher_elg>(2*mu); //vector of the products of the diogonals of Y^T generated in round 9
	rho_a = new vector<ZZ>(2*mu); //contains random elements used for the reencryption in 9


	Dl = new vector<ZZ>(2*m+1); //bilinear_map(Y_pi, U, chal_t)
	r_Dl = new vector<ZZ>(2*m+1); //random elements to commit to the C_ls
	c_Dl = new vector<Mod_p>(2*m +1); //commitments to the C_ls

	d_bar = new vector<ZZ>(n);// chal_x8*D_h(m-1) +d
	Delta_bar = new vector<ZZ>(n);//chal_x8*d_h+Delta
	D_h_bar = new vector<ZZ>(n);//sum over the rows in D_h

	B_bar  = new vector<ZZ>(n); // sum over the rows in B multiplied by chal^i
	A_bar = new vector<ZZ>(n); //sum over the rows in A times the challenges
	D_s_bar = new vector<ZZ>(n); // sum over the rows in D_s times the challenges

}

//Destructor deletes all pointers and frees the storage
Prover_toom::~Prover_toom() {
	delete chal_x6;
	delete chal_y6;
	delete chal_x8;
	delete c_A;
	delete r_A;

	Functions::delete_vector(D);
	Functions::delete_vector(D_h);
	Functions::delete_vector(D_s);
	delete d;
	delete Delta;
	delete d_h;

	delete r_D_h;
	delete c_D_h;

	if (A != NULL) Functions::delete_vector(A);
	if (B != NULL) Functions::delete_vector(B);
	if (basis_B != NULL) Functions::delete_vector(basis_B);
	delete B_0;
	if (basis_B0 != NULL) Functions::delete_vector(basis_B0);
	delete r_B;
	delete r_B_small;
	delete c_B;
	delete a;
	delete r_a;
	delete c_a;
	delete rho_a;

	delete Dl;
	delete r_Dl;
	delete c_Dl;

	delete D_h_bar;
	delete d_bar;
	delete Delta_bar;
	delete B_bar;
	delete A_bar;
	delete D_s_bar;

	delete C_c;
	delete c_a_c; //vector containing the commitments to value used for the reencryption of E_low_up
	delete a_c; //vector containing the exponents
	delete r_c;
	delete rho_c;
	delete E;
}


string Prover_toom::get_public_vector() {
	return ped_.get_public_vector();
}

//round_1 picks random elements and commits to the rows of A
string Prover_toom::round_1(){
	long i;
	//calculates commitments to rows of A
	Functions::commit_op(A,r_A,c_A, ped_);

	//ofstream ost(name.c_str());
	stringstream out_stream;
	for (i=0; i<m; i++){
		out_stream << c_A->at(i)<< " ";
	}
	return out_stream.str();
}

//round_3, permuted the exponents in s,  picks random elements and commits to values
string Prover_toom::round_3(const string& input){
	long i;
	ZZ x2;
	vector<vector<ZZ>* >* chal_x2 = new vector<vector<ZZ>* >(m);

	stringstream ist(input);
	ist >> x2;

	//creates a matrix with entries x2,..., x2^N
	func_pro::set_x2(chal_x2,x2, m,n);

	//permutes chal_x2 according pi to create B
	func_pro::set_B_op(B, basis_B, chal_x2, pi , omega_mulex);

	//commits to the rows in B
	Functions::commit_op(B,r_B,c_B, ped_);

	//write data in the file name
	stringstream out_stream;
	for (i=0; i<m;i++){
		out_stream << c_B->at(i) <<" ";
	}

	Functions::delete_vector(chal_x2);
	return out_stream.str();
}


//round_5a calculates D and the commitments to the vectors chal_z, D_h
void Prover_toom::round_5a(){
	long i;
	vector<ZZ>* r = new vector<ZZ>(n);
	vector<ZZ>* v_z = new vector<ZZ>(n); //row containing the challenge alpha
	ZZ ord = H.get_ord();
	time_t rawtime;
	time ( &rawtime );

	//calculate for each value in the first m rows in D: y* A_ij + A_ij -z
	func_pro::set_D(D, A,B, chal_z4, chal_y4);

	//Set the matrix D_h as the Hadamard product of the rows in D
	func_pro::set_D_h(D_h, D);

	const ZZ one = to_ZZ(1);

	{
        //PARALLELIZE
        #pragma omp parallel for num_threads(num_threads) if(parallel)
		for( i=0; i<n;i++){
			v_z->at(i) = chal_z4; //fills the vector alpha with the challenge alpha
			NegateMod(r->at(i), one, ord);
		}
	}

	//Sets the additional row in D to contain -1
	D->at(m) = r;
	//random number to commit to last row in A
	r_A->at(m) = 0;

	//calculate commitment to alpha
	Functions::commit_op(v_z, r_z, c_z, ped_);
	//calculate commitment to the rows in D_h
	Functions::commit_op(D_h,r_D_h,c_D_h, ped_);

	delete v_z;
}


void Prover_toom::round_5b(){
	func_pro::set_Rb(B, R, R_b);
	commit_ac();
	calculate_Cc(C,basis_B);
}

string Prover_toom::round_5(const string& input){
	long i;

	//reads the values out of the file
	stringstream ist(input);
	ist>>chal_z4;
	ist >> chal_y4;
	
	round_5a();
	round_5b();

	stringstream ost;
	//writes the commitments in the file
	ost<<  c_z<< "\n";
	for (i = 0; i<m ; i++){
		ost << c_D_h ->at(i)<< " ";
	}
	ost<<"\n";

	for(i=0; i<mu_h; i++){
		ost<<C_c->at(i)<<" ";
	}
	ost<<"\n";
	for(i=0; i<mu_h; i++){
		ost<<c_a_c->at(i)<<" ";
	}
	return ost.str();
}

string Prover_toom::round_5_red(const string& input){
	long i;
	
	stringstream ist(input);
	ist>>chal_z4;
	ist >> chal_y4;

	func_pro::set_Rb(B,R,R_b);
	commit_ac();
	calculate_Cc(C,basis_B);

	stringstream ost;
	for(i=0; i<mu_h; i++){
		ost<<C_c->at(i)<<" ";
	}
	ost<<"\n";
	for(i=0; i<mu_h; i++){
		ost<<c_a_c->at(i)<<" ";
	}
	return ost.str();

}

string Prover_toom::round_5_red1(const string& input){
	long i;
	stringstream ist(input);
	x = new vector<ZZ>(mu_h);

	//reads challenges x
	for(i=0; i<mu_h; i++){
		ist>> x->at(i);
	}

	//Call of round 5a
	round_5a();

	//calculate F_c and Z_c for the first reduction
	calculate_ac_bar(x);
	calculate_r_ac_bar(x);

	//reduction from m rows to m_r rows
	auto tstart= high_resolution_clock::now();
	reduce_C(C, B, r_B, x, 4*m_r);
	auto tstop = high_resolution_clock::now();
	time_di = time_di+ duration<double>(tstop-tstart).count();

	set_Rb1(x);
	commit_ac();
	calculate_Cc(C_small, B_small);

	delete x;

	stringstream ost;
	//writes the commitments in the file
	ost<<  c_z<< "\n";
	for (i = 0; i<m ; i++){
		ost << c_D_h ->at(i)<< " ";
	}
	ost<<"\n";
	for(i=0; i<mu_h; i++){
		ost<<C_c->at(i)<<" ";
	}
	ost<<"\n";
	for(i=0; i<mu_h; i++){
		ost<<c_a_c->at(i)<<" ";
	}
	ost<<a_c_bar<<endl;
	ost<<r_ac_bar<<endl;
	return ost.str();
}

void Prover_toom::round_7a(){

	//Set the rows in D_s as D_s(i) = chal_t_1^i+1*D_h(i) for i<m-1 and D_s(m-1) = sum(chal_x6^i+1 * D_s(i+1) and set last row of D_s to random values and also D(0)
	func_pro::set_D_s(D_s,D_h,D,chal_x6,r_Dl_bar);

	//calculate the values Dls as Dl(l) = sum(D(i)*D_s->at(i)*chal_y6) for j=n+i-l and commits to the values
	func_pro::commit_Dl_op(c_Dl,Dl, r_Dl, D, D_s, chal_y6, ped_);

	//commitments to D(0) and D_s(m)
	Functions::commit_op(D->at(0),r_D0,c_D0, ped_);
	
	Functions::commit_op(D_s->at(m), r_Dm, c_Dm, ped_);

	//commitments to prove that the product over the elements in D_h->at(m) is the desired product of n *y + x2n -z
	func_pro::commit_d_op(d,r_d,c_d, ped_);
	
	func_pro::commit_Delta_op(Delta, d, r_Delta, c_Delta, ped_);
	
	func_pro::commit_d_h_op(D_h,d_h,d,Delta, r_d_h, c_d_h, ped_);
}

void Prover_toom::round_7b(){
	calculate_ac_bar(chal_x6);
	calculate_r_ac_bar(chal_x6);

}

void  Prover_toom::round_7c(){
	vector<Cipher_elg>* e = 0;

	auto tstart = high_resolution_clock::now();
	reduce_C(C, B, r_B, chal_x6, m_r);
	set_Rb1(chal_x6);
	auto tstop = high_resolution_clock::now();
	//cout << "7c: reduce_C " << duration<double>(tstop - tstart).count() << endl;
	time_di = time_di+duration<double>(tstop - tstart).count();

	tstart = high_resolution_clock::now();
	func_pro::commit_a_op(a, r_a, c_a, ped_);
	tstop = high_resolution_clock::now();
	//cout << "7c: commit_a " << duration<double>(tstop - tstart).count() << endl;
	func_pro::commit_B0_op(B_0, basis_B0, r_B0, c_B0, omega_mulex, ped_);

	tstart = high_resolution_clock::now();
	e = calculate_e();
	tstop = high_resolution_clock::now();
	//cout << "7c: e " << duration<double>(tstop - tstart).count() << endl;
	time_di = time_di+duration<double>(tstop - tstart).count();
	//cout<<"To calculate the di's took "<<time_di<<" sec."<<endl;

	calculate_E(e);

	delete e;
	Functions::delete_vector(C_small);
}

void  Prover_toom::round_7c_red(){
	vector<Cipher_elg>* e = 0;
	vector<vector<Cipher_elg>* >* C_small_temp = 0;
	vector<vector<ZZ>* >* B_small_temp = 0;
	vector<ZZ>* r_B_small_temp = 0;

	auto tstart= high_resolution_clock::now();
		C_small_temp = copy_C();
		B_small_temp = copy_B();
		r_B_small_temp = copy_r_B();

		C_small = new vector<vector<Cipher_elg>* >(m_r);
		B_small = new vector<vector<ZZ>* >(m_r);
		r_B_small = new vector<ZZ>(m_r);

		reduce_C(C_small_temp, B_small_temp, r_B_small_temp, chal_x6, m_r);
		set_Rb1(chal_x6);
	auto tstop = high_resolution_clock::now();
	time_di = time_di+ duration<double>(tstop-tstart).count();


	func_pro::commit_a_op(a,r_a,c_a, ped_);
	func_pro::commit_B0_op(B_0, basis_B0, r_B0, c_B0, omega_mulex, ped_);

	tstart = high_resolution_clock::now();
	e= calculate_e();
	tstop = high_resolution_clock::now();
	time_di = time_di+duration<double>(tstop-tstart).count();
	//cout<<"To calculate the di's took "<<time_di<<" sec."<<endl;

	calculate_E(e);

	Functions::delete_vector(C_small);
	Functions::delete_vector(C_small_temp);
	Functions::delete_vector(B_small_temp);
	delete r_B_small_temp;
	delete e;
}

string Prover_toom::round_7(const string& input){
	long i,l;
	//reads the values out of the file
	stringstream ist(input);
	//reads the vector t_1
	l=2*m;
	for (i = 0; i<l; i++){
		ist >> chal_x6->at(i);
	}
	//reads the vector t
	for (i = 0; i<n; i++){
		ist >> chal_y6->at(i);
	}

	round_7a();
	round_7b();
	round_7c();

	//Set name of the output file and open stream

	stringstream ost;
	for (i = 0; i<=l ; i++) {
		ost << c_Dl ->at(i)<< " ";
	}
	ost << "\n";
	ost<<c_D0<<"\n";
	ost <<c_Dm<<"\n";
	ost<<c_d<<"\n";
	ost<<c_Delta<<"\n";
	ost<<c_d_h<<"\n";
	ost<<a_c_bar<<"\n";
	ost<<r_ac_bar<<"\n";
	for(i=0; i<8; i++){
		ost<<E->at(i)<<" ";
	}
	ost<<"\n";
	ost<<c_B0<<"\n";
	for(i=0; i<8; i++){
		ost<<c_a->at(i)<<" ";
	}
	ost<<"\n";


	return ost.str();
}

string Prover_toom::round_7_red(const string& input){
	long i,l;
	//reads the values out of the file
	stringstream ist(input);
	//reads the vector t_1
	l=2*m;
	for (i = 0; i<l; i++){
		ist >> chal_x6->at(i);
	}
	//reads the vector t
	for (i = 0; i<n; i++){
		ist >> chal_y6->at(i);
	}

	round_7a();
	round_7b();
	round_7c_red();



	stringstream ost;
	for (i = 0; i<=l ; i++){
		ost << c_Dl ->at(i)<< " ";
	}
	ost << "\n";
	ost<<c_D0<<"\n";
	ost<<c_Dm<<"\n";
	ost<<c_d<<"\n";
	ost<<c_Delta<<"\n";
	ost<<c_d_h<<"\n";

	ost<<a_c_bar<<"\n";
	ost<<r_ac_bar<<"\n";
	for(i=0; i<8; i++){
		ost<<E->at(i)<<" ";
	}
	ost<<"\n";
	ost<<c_B0<<"\n";
	for(i=0; i<8; i++){
		ost<<c_a->at(i)<<" ";
	}
	ost<<"\n";
	return ost.str();
}

void Prover_toom::round_9a(){

	//Calculate D_h_bar = sum(chal^i*D_h(row(i)))
	func_pro::calculate_D_h_bar(D_h_bar,D_h,chal_x8);

	//calculate r_Dh_bar = sum(chal^i*r_Dh_bar(i)), opening to prove correctness of D_h
	func_pro::calculate_r_Dh_bar(r_D_h, chal_x8, r_Dh_bar);

	//calculate d_bar, r_d_bar, Delta_bar, r_Delta_bar, openings to prove product over elements in D_h->at(m-1)
	func_pro::calculate_dbar_rdbar(D_h, chal_x8, d_bar,d,r_D_h, r_d, r_d_bar);
	func_pro::calculate_Deltabar_rDeltabar(d_h, chal_x8, Delta_bar, Delta, r_d_h, r_Delta, r_Delta_bar);


}

void  Prover_toom::round_9b(){

	//A_bar and r_A_bar, openings to prove permutation in D
	func_pro::calculate_A_bar(D, A_bar, chal_x8);
	func_pro::calculate_r_A_bar(r_D0, r_A, r_B, chal_x8, r_z, chal_y4, r_A_bar);

	//D_s_bar and r_Ds_bar, openings to prove correctness of D_s
	func_pro::calculate_D_s_bar(D_s, D_s_bar, chal_x8);
	func_pro::calculate_r_Ds_bar(r_D_h, chal_x6, chal_x8, r_Ds_bar, r_Dm);

	//sum of the random values used to commit to the Dl's, to prover correctness of them
	func_pro::calculate_r_Dl_bar(r_Dl, chal_x8, r_Dl_bar);

}

void  Prover_toom::round_9c(){
	//calculate B_bar
	func_pro::calculate_B_bar(B_0, B_small,chal_x8, B_bar );
	Functions::delete_vector(B_small);

	//calculate r_B_bar
	func_pro::calculate_r_B_bar(r_B_small, chal_x8,r_B0, r_B_bar );

	//calculate a_bar
	func_pro::calculate_a_bar(a, chal_x8, a_bar);

	//calculate r_a_bar
	func_pro::calculate_r_a_bar(r_a, chal_x8, r_a_bar);

	//calculate rho_a_bar
	func_pro::calculate_rho_a_bar(rho_a, chal_x8, rho_bar);
}

string Prover_toom::round_9(const string& input){
	long i;
	long l = chal_x8->size();

	//reads the values out of the file
	stringstream ist(input);

	//reads the vector e
	for (i = 0; i<l ; i++){
		ist >> chal_x8->at(i);
	}

	round_9a();
	round_9b();
	round_9c();

	//Set name of the output file and open stream
	stringstream ost;

	for (i = 0; i<n; i++){
		ost << D_h_bar->at(i)<<" ";
	}
	ost <<"\n";

	ost<< r_Dh_bar;
	ost <<"\n";

	for(i=0; i<n;i++){
		ost<<d_bar->at(i)<<" ";
	}
	ost<<"\n";
	ost<< r_d_bar <<"\n";
	for(i=0; i<n; i++){
		ost<<Delta_bar->at(i)<<" ";
	}
	ost<<"\n";
	ost<<r_Delta_bar <<"\n";

	for (i = 0; i<n; i++){
		ost << A_bar->at(i)<<" ";
	}
	ost<<"\n";
	ost<<r_A_bar<<"\n";
	for(i=0; i<n; i++){
		ost<<D_s_bar->at(i)<<" ";
	}
	ost<<"\n";
	ost<<r_Ds_bar<<"\n";
	ost<<r_Dl_bar<<"\n";
	for (i = 0; i<n; i++){
		ost << B_bar->at(i)<<" ";
	}
	ost <<"\n";

	ost<< r_B_bar;
	ost <<"\n";

	ost<< a_bar;
	ost <<"\n";

	ost<< r_a_bar;
	ost <<"\n";

	ost<< rho_bar;
	ost <<"\n";

	return ost.str();
}



void Prover_toom::commit_ac(){
	long i;
	ZZ ord = H.get_ord();
    //does not need parallelization
	for(i= 0; i<mu_h; i++){
		a_c->at(i) = RandomBnd(ord);
		r_c->at(i) = RandomBnd(ord);
		rho_c->at(i) = RandomBnd(ord);
	}
	a_c->at(mu-1) = to_ZZ(0);
	r_c->at(mu-1) = to_ZZ(0);
	NegateMod(rho_c->at(mu-1),R_b, ord);

	for(i= 0; i<mu_h; i++){
		c_a_c->at(i) = ped_.commit_sw(a_c->at(i),r_c->at(i));
	}

}


void Prover_toom::calculate_Cc(vector<vector<Cipher_elg>* >* C, vector<vector<vector<long>* >* >* B){
	long i, j, l,k;
	ZZ mod = H.get_mod();
	CurvePoint gen = H.get_gen().get_val();
	Cipher_elg temp, temp_1;
	CurvePoint t_1;
    high_resolution_clock::time_point t1, t2;

    //cout << "mu " << mu << endl;
    //cout << "mu_h " << mu_h << endl;
    //cout << "m_r " << m_r << endl;
    //cout << "omega_mulex " << omega_mulex << endl;
	
	const Cipher_elg one_enced(curve_zeropoint(),curve_zeropoint(),mod);
	//int count = 0;
	t1 = high_resolution_clock::now();
	for(k=0; k<mu_h; k++){
		temp = one_enced;
		{
		for(i=0; i<mu; i++) {
			j=k+1-mu+i;
			if((j>=0) & (j<mu)) {
                //PARALLELIZE
                #pragma omp parallel for num_threads(num_threads) if(parallel)
				for(l=0; l<m_r; l++) {
	                Cipher_elg c;
	                multi_expo::expo_mult(c, C->at(4*l+i), B->at(4*l+j), omega_mulex);
                    #pragma omp critical(Cc1)
                    {
	                    Cipher_elg::mult(temp, temp, c);
                    }
				}
			}
		}
		}
		PowerMod(t_1, gen, a_c->at(k), mod);
		temp_1 = elgammal_->encrypt(t_1, rho_c->at(k));
		Cipher_elg::mult(C_c->at(k),temp, temp_1);
	}
	t2 = high_resolution_clock::now();
    //duration<double> par1 = duration_cast<duration<double>>(t2-t1);
    //cout << "Calculate_Cc par 1 " << par1.count() << endl;
}

void Prover_toom::calculate_Cc(vector<vector<Cipher_elg>* >* C, vector<vector<ZZ>* >* B){
	long i, j, l,k;
	ZZ mod = H.get_mod();
	CurvePoint gen = H.get_gen().get_val();
	Cipher_elg temp, temp_1;
	CurvePoint t_1;
    high_resolution_clock::time_point t1, t2;

	t1 = high_resolution_clock::now();
	for(k=0; k<mu_h; k++){
		temp = Cipher_elg(curve_zeropoint(),curve_zeropoint(),mod);
		{
		for(i=0; i<mu; i++){
			j=k+1-mu+i;
			if((j>=0) & (j<mu)){
                //PARALLELIZE
                #pragma omp parallel for private(temp_1) num_threads(num_threads) if(parallel)
				for(l=0; l<m_r; l++){
					multi_expo::expo_mult(temp_1, C->at(4*l+i), B->at(4*l+j), omega_mulex);
                    #pragma omp critical(Cc2)
                    {
					    Cipher_elg::mult(temp, temp, temp_1);
                    }
				}
			}
		}
		}
		PowerMod(t_1, gen, a_c->at(k), mod);
		temp_1 = elgammal_->encrypt(t_1, rho_c->at(k));
		Cipher_elg::mult(C_c->at(k),temp, temp_1);
	}
	t2 = high_resolution_clock::now();
    //duration<double> par1 = duration_cast<duration<double>>(t2-t1);
    //cout << "Calculate_Cc par 2 " << par1.count() << endl;
}


void Prover_toom::calculate_ac_bar(vector<ZZ>* x){
	long i;
	ZZ temp;
	ZZ ord = H.get_ord();
    //does not need parallelization
	a_c_bar = a_c->at(0);
	for(i=1; i<mu_h; i++){
		MulMod(temp, a_c->at(i), x->at(i-1), ord);
		AddMod(a_c_bar, a_c_bar, temp, ord);
	}
}

void Prover_toom::calculate_r_ac_bar(vector<ZZ>* x){
	long i;
	ZZ temp;
	ZZ ord = H.get_ord();
    //does not need parallelization
	r_ac_bar = r_c->at(0);
	for(i=1; i<mu_h; i++){
		MulMod(temp, r_c->at(i), x->at(i-1), ord);
		AddMod(r_ac_bar, r_ac_bar, temp, ord);
	}
}


static void __do_reduce_C_external(vector<vector<Cipher_elg>*>* C, vector<vector<ZZ>* >* B, vector<ZZ>* r_B, vector<vector<Cipher_elg>* >* C_small, vector<vector<ZZ>* >* B_small, vector<ZZ>* r_B_small, vector<ZZ>* x_temp, ZZ& ord, int omega_LL, long i, int n) {
	ZZ temp, temp_1;
	vector<Cipher_elg>* row_C;
	vector<ZZ>* row_B;

	row_C = new vector<Cipher_elg>(n);
	row_B = new vector<ZZ>(n);
	{
        //PARALLELIZE
        #pragma omp parallel for num_threads(num_threads) if(parallel)
		for(long j=0; j<n; j++){
	        ZZ temp, temp_1;
	        multi_expo::multi_expo_LL(row_C->at(j), C->at(4*i)->at(j), C->at(4*i+1)->at(j),C->at(4*i+2)->at(j),C->at(4*i+3)->at(j), x_temp, omega_LL);
	        temp = B->at(4*i)->at(j);
	        MulMod(temp_1, B->at(4*i+1)->at(j), x_temp->at(2), ord);
	        AddMod(temp, temp, temp_1, ord);
	        MulMod(temp_1, B->at(4*i+2)->at(j), x_temp->at(1), ord);
	        AddMod(temp, temp, temp_1, ord);
	        MulMod(temp_1, B->at(4*i+3)->at(j), x_temp->at(0), ord);
	        AddMod(temp, temp, temp_1, ord);
	        row_B->at(j) = temp;
		}
	}
	C_small->at(i)=row_C;
	B_small->at(i)=row_B;
	temp = r_B->at(4*i);
	MulMod(temp_1, r_B->at(4*i+1), x_temp->at(2), ord);
	AddMod(temp, temp, temp_1, ord);
	MulMod(temp_1, r_B->at(4*i+2), x_temp->at(1), ord);
	AddMod(temp, temp, temp_1, ord);
	MulMod(temp_1, r_B->at(4*i+3), x_temp->at(0), ord);
	AddMod(temp, temp, temp_1, ord);
	r_B_small->at(i) = temp;
}

void Prover_toom::reduce_C(vector<vector<Cipher_elg>*>* C, vector<vector<ZZ>* >* B, vector<ZZ>* r_B, vector<ZZ>* x, long length){
	ZZ ord  = H.get_ord();
	vector<ZZ>* x_temp = new vector<ZZ>(4);

	auto tstart= high_resolution_clock::now();
	x_temp->at(3)=1;
	x_temp->at(2)= x->at(0);
	x_temp->at(1) = x->at(1);
	x_temp->at(0)= x->at(2);

	{
		for(long i=0; i<length;i++){
			__do_reduce_C_external(C, B, r_B, C_small, B_small, r_B_small, x_temp, ord, omega_LL, i, n);
		}
	}
	auto tstop = high_resolution_clock::now();
	time_di = time_di + duration<double>(tstop-tstart).count();
	delete x_temp;
}

void Prover_toom::set_Rb1(vector<ZZ>* x){
	long i;
	ZZ temp;
	ZZ ord = H.get_ord();
    //does not need parallelization
	R_b = rho_c->at(0);
	for(i=1; i<mu_h; i++){
		MulMod(temp, rho_c->at(i), x->at(i-1), ord);
		AddMod(R_b, R_b, temp, ord);
	}
}



vector<Cipher_elg>* Prover_toom::calculate_e(){
	long k,l;
	Cipher_elg temp;
	vector<Cipher_elg>* dt = 0;
	vector<Cipher_elg>* e = new vector<Cipher_elg>(2*m);
    //high_resolution_clock::time_point t1, t2;
	
	dt = toom4_pow(C_small, B_small);
	    //PARALLELIZE	
	    //t1 = high_resolution_clock::now();
        #pragma omp parallel for private(temp) num_threads(num_threads) if(parallel)
		for (k =0; k<mu; k++){
			if (k == 0) {
				multi_expo::expo_mult(e->at(0),C_small->at(mu-1), basis_B0, omega_mulex);
			} else {
				multi_expo::expo_mult(temp , C_small->at(mu-k-1), basis_B0, omega_mulex);
				Cipher_elg::mult(e->at(k) ,temp,dt->at(2*mu-k-1));
			}
		}
        //t2 = high_resolution_clock::now();
	l=2*mu;
    #pragma omp parallel for num_threads(num_threads) if(parallel)
	for (k = mu; k<l; k++){
		e->at(k) = dt->at(2*mu-k-1);
	}

    //duration<double> par1 = duration_cast<duration<double>>(t2-t1);
    //cout << "Calculate_e par 1 " << par1.count() << endl;

	delete dt;
	return e;
}

void Prover_toom::calculate_E(vector<Cipher_elg>* e){
	long i,l;
	Mod_p t;
	Mod_p gen = H.get_gen();
	ZZ ord = H.get_ord();
    //does not need parallelism
	l=2*mu;
	for (i = 0; i<l; i++){
		rho_a->at(i)= RandomBnd(ord);
	}
	rho_a->at(mu)=R_b ;
	for (i = 0; i<l; i++){
		 t = gen.expo(a->at(i));
		 E->at(i) = elgammal_->encrypt(t,rho_a->at(i))*e->at(i);
	}
}


vector<vector<Cipher_elg>* >* Prover_toom::copy_C(){
	long i,j,l;
	vector<Cipher_elg>* row_C;
	l=mu*m_r;
	vector<vector<Cipher_elg>* >* C_small_temp = new vector<vector<Cipher_elg>* >(l);

	for(i=0; i<l; i++){
		row_C = new vector<Cipher_elg>(n);
		for(j=0; j<n; j++){
			row_C->at(j)= C_small->at(i)->at(j);
		}
		C_small_temp ->at(i)= row_C;
		delete C_small->at(i);
		C_small->at(i)=0;
	}
	delete C_small;
	C_small=0;

	return C_small_temp;
}

vector<vector<ZZ>* >* Prover_toom::copy_B(){
	long i, j;
	long l = mu*m_r;
	vector<vector<ZZ>* >* B_small_temp =new vector<vector<ZZ>* >(l);
	vector<ZZ>* row_B;

	for(i=0; i<l; i++){
		row_B = new vector<ZZ>(n);
		for(j=0; j<n; j++){
			row_B->at(j)=B_small->at(i)->at(j);
		}
		B_small_temp->at(i)=row_B;
		delete B_small->at(i);
		B_small->at(i)=0;
	}
	delete B_small;

	return B_small_temp;
}

vector<ZZ>* Prover_toom::copy_r_B(){
	long i;
	long l=mu*m_r;
	vector<ZZ>* r_B_small_temp = new vector<ZZ>(l);
	for(i=0; i<l; i++){
		r_B_small_temp->at(i)=r_B_small->at(i);
	}
	delete r_B_small;
	r_B_small=0;

	return r_B_small_temp;
}




vector<vector<ZZ>*>* Prover_toom::evulation(vector<vector<ZZ>*>* p){
	vector<vector<ZZ>*>* ret;
	vector<ZZ>* row;
	ZZ p0,p1,p2,p3,ord,temp,temp_1;
	long l,i;
	l= p->at(0)->size();
	ord = H.get_ord();
	ret = new vector<vector<ZZ>*>(l);
    //does not need parallelization
	for(i = 0; i<l; i++){
		row = new vector<ZZ>(7);
			AddMod(p0,p->at(2)->at(i), p->at(0)->at(i),ord);
			AddMod(p1 ,p->at(3)->at(i) , p->at(1)->at(i), ord);
			MulMod(temp, p->at(2)->at(i),2,ord);
			MulMod(temp_1, p->at(0)->at(i), 8, ord);
			AddMod(p2 , temp , temp_1,ord);
			MulMod(temp, p->at(1)->at(i), 4, ord);
			AddMod(p3 ,p->at(3)->at(i) , temp,ord);

			row->at(0) = p->at(3)->at(i);
			MulMod(temp_1, p->at(1)->at(i), 2,ord);
			AddMod(temp,temp_1 , p->at(0)->at(i), ord);
			MulMod(temp_1, p->at(2)->at(i),4, ord);
			AddMod(temp,temp,temp_1,ord);
			MulMod(temp_1, p->at(3)->at(i), 8, ord);
			AddMod(row->at(1), temp,temp_1, ord);
			AddMod(row->at(2) , p0,p1,ord);
			SubMod(row->at(3) , p0,p1,ord);
			AddMod(row ->at(4) , p2,p3,ord);
			SubMod(row ->at(5),p2,p3,ord);
			row->at(6) = p->at(0)->at(i);
			ret->at(i) = row;
	}
	return ret;
}


vector<vector<vector<CurvePoint>*>*>* Prover_toom::evulation_pow(vector<vector<Cipher_elg>*>* p){
	vector<vector<vector<CurvePoint>*>*>* ret;
	vector<vector<CurvePoint>* >* ret_u;
	vector<vector<CurvePoint>* >* ret_v;
	vector<CurvePoint>* row_u;
	vector<CurvePoint>* row_v;
	CurvePoint p0_u,p1_u,p2_u,p3_u,temp_u, temp_1_u;
	CurvePoint p0_v,p1_v,p2_v,p3_v,temp_v, temp_1_v;
	ZZ mod = H.get_mod();
	long l, i;
	l = p->at(0)->size();

	ret = new vector<vector<vector<CurvePoint>*>*>(2);
	ret_u = new vector<vector<CurvePoint>*>(l);
	ret_v = new vector<vector<CurvePoint>*>(l);
    //does not need parallelization
	for(i = 0; i<l; i++){
		row_u = new vector<CurvePoint>(7);
		row_v = new vector<CurvePoint>(7);
		MulMod(p0_u,p->at(1)->at(i).get_u(), p->at(3)->at(i).get_u(),mod);
		MulMod(p0_v,p->at(1)->at(i).get_v(), p->at(3)->at(i).get_v(),mod);
		MulMod(p1_u ,p->at(0) ->at(i).get_u(), p->at(2)->at(i).get_u(), mod);
		MulMod(p1_v ,p->at(0) ->at(i).get_v(), p->at(2)->at(i).get_v(), mod);
		PowerMod(temp_u, p->at(1)->at(i).get_u(), 2,mod);
		PowerMod(temp_v, p->at(1)->at(i).get_v(), 2,mod);
		PowerMod(temp_1_u, p->at(3)->at(i).get_u(), 8,mod);
		PowerMod(temp_1_v, p->at(3)->at(i).get_v(), 8,mod);
		MulMod(p2_u ,temp_u , temp_1_u,mod);
		MulMod(p2_v ,temp_v , temp_1_v,mod);
		PowerMod(temp_u, p->at(2)->at(i).get_u(), 4, mod);
		PowerMod(temp_v, p->at(2)->at(i).get_v(), 4, mod);
		MulMod(p3_u ,p->at(0)->at(i).get_u() , temp_u,mod);
		MulMod(p3_v ,p->at(0)->at(i).get_v() , temp_v,mod);

		row_u->at(0) = p->at(0)->at(i).get_u();
		PowerMod(temp_u, p->at(2)->at(i).get_u(), 2,mod);
		MulMod(temp_u,temp_u , p->at(3)->at(i).get_u(), mod);
		PowerMod(temp_1_u, p->at(1)->at(i).get_u(), 4,mod);
		MulMod(temp_u,temp_u,temp_1_u,mod);
		PowerMod(temp_1_u, p->at(0)->at(i).get_u(), 8, mod);
		MulMod(row_u->at(1), temp_u,temp_1_u,mod);
		MulMod(row_u->at(2) , p0_u,p1_u,mod);
		InvMod(temp_u, p1_u, mod);
		MulMod(row_u->at(3) , p0_u,temp_u,mod);
		MulMod(row_u ->at(4) , p2_u,p3_u,mod);
		InvMod(temp_u, p3_u, mod);
		MulMod(row_u ->at(5),p2_u,temp_u,mod);
		row_u->at(6) = p->at(3)->at(i).get_u();

		row_v->at(0) = p->at(0)->at(i).get_v();
		PowerMod(temp_v, p->at(2)->at(i).get_v(), 2,mod);
		MulMod(temp_v,temp_v , p->at(3)->at(i).get_v(), mod);
		PowerMod(temp_1_v, p->at(1)->at(i).get_v(), 4,mod);
		MulMod(temp_v,temp_v,temp_1_v,mod);
		PowerMod(temp_1_v, p->at(0)->at(i).get_v(), 8, mod);
		MulMod(row_v->at(1), temp_v,temp_1_v,mod);
		MulMod(row_v->at(2) , p0_v,p1_v,mod);
		InvMod(temp_v, p1_v, mod);
		MulMod(row_v->at(3) , p0_v,temp_v,mod);
		MulMod(row_v ->at(4) , p2_v,p3_v,mod);
		InvMod(temp_v, p3_v, mod);
		MulMod(row_v ->at(5),p2_v,temp_v,mod);
		row_v->at(6) = p->at(3)->at(i).get_v();

		ret_u->at(i) = row_u;
		ret_v->at(i) = row_v;
	}

	ret->at(0) = ret_u;
	ret->at(1) = ret_v;
	return ret;
}

vector<vector<vector<CurvePoint>*>*>* Prover_toom::point_pow(vector<vector<vector<CurvePoint>*>*>* points_p, vector<vector<ZZ>*>* points_q){
	long i,l;
	vector<vector<vector<CurvePoint>*>*>* ret;
	vector<vector<CurvePoint>*>* ret_u;
	vector<vector<CurvePoint>*>* ret_v;
	//vector<ZZ>* row_u;
	//vector<ZZ>* row_v;
	ZZ mod = H.get_mod();
	l = points_p->at(0)->size();

    high_resolution_clock::time_point t1, t2, t3, t4;

	ret = new vector<vector<vector<CurvePoint>*>*>(2);
	ret_u = new vector<vector<CurvePoint>*>(l);
	ret_v = new vector<vector<CurvePoint>*>(l);

    
	for (long j = 0; j < l; j++) {
		ret_u->at(j) = new vector<CurvePoint>(7);
		ret_v->at(j) = new vector<CurvePoint>(7);
	}
	
	{
        //PARALLELIZE
        #pragma omp parallel for collapse(2) num_threads(num_threads) if(parallel)
		for(long j = 0; j<l; j++){
			for(i=0; i<7; i++){
				PowerMod(ret_u->at(j)->at(i), points_p->at(0)->at(j)->at(i), points_q->at(j)->at(i),mod);
                PowerMod(ret_v->at(j)->at(i), points_p->at(1)->at(j)->at(i), points_q->at(j)->at(i),mod);
			}
		}
	}
	
	ret->at(0)= ret_u;
	ret->at(1) = ret_v;
	for(i = 0; i< l ; i++){
		delete points_p->at(0)->at(i);
		points_p->at(0)->at(i)=0;
		delete points_p->at(1)->at(i);
		points_p->at(1)->at(i)=0;
	}
	delete points_p->at(0);
	delete points_p->at(1);
	delete points_p;
	for(i = 0; i< l ; i++){
		delete points_q->at(i);
		points_q->at(i)=0;
	}
	delete points_q;

	return ret;
}


vector<vector<CurvePoint>*>* Prover_toom::mult_points(vector<vector<vector<CurvePoint>* >*>* points){
	long i,l,j;
	vector<vector<CurvePoint>*>* ret = new vector<vector<CurvePoint>*>(2);
	vector<CurvePoint>* ret_u = new vector<CurvePoint>(7);
	vector<CurvePoint>* ret_v = new vector<CurvePoint>(7);
	l = points->at(0)->size();
	CurvePoint temp_u, temp_v;
	ZZ mod = H.get_mod();
    //does not need parallelization
	for(i = 0; i<7; i++){
		temp_u = curve_zeropoint();
		temp_v = curve_zeropoint();
		for(j = 0; j<l; j++){
			MulMod(temp_u, temp_u, points->at(0)->at(j)->at(i),mod);
			MulMod(temp_v, temp_v, points->at(1)->at(j)->at(i),mod);
		}
		ret_u->at(i) = temp_u;
		ret_v->at(i) = temp_v;
	}
	for(i = 0; i<l;i++){
		delete points->at(0)->at(i);
		points->at(0)->at(i) = 0;
		delete points->at(1)->at(i);
		points->at(1)->at(i) = 0;
	}
	delete points->at(0);
	points->at(0) =0;
	delete points->at(1);
	points->at(1) =0;
	delete points;
	ret->at(0) = ret_u;
	ret->at(1) = ret_v;
	return ret;
}

vector<CurvePoint>* Prover_toom::interpolation_pow(vector<CurvePoint>* points){
	vector<CurvePoint>* ret = new vector<CurvePoint>(7);
	CurvePoint r1,r2,r3,r4,r5,r6,r7,temp_pt;
        ZZ temp_zz;
	ZZ ord = H.get_ord();
	ZZ mod = H.get_mod();

	r1 = points->at(0);
	r2 = points->at(1);
	r3 = points->at(2);
	r4 = points->at(3);
	r5 = points->at(4);
	r6 = points->at(5);
	r7 = points->at(6);

	MulMod(r2 ,r2, r5, mod);
	InvMod(temp_pt,r5,mod);
	MulMod(r6 , r6,temp_pt, mod);
	InvMod(temp_pt, r3, mod);
	MulMod(r4 , r4,temp_pt,mod);
	InvMod(temp_pt, r1, mod);
	MulMod(r5,r5,temp_pt,mod);
	PowerMod(temp_pt, r7, 64,mod);
	InvMod(temp_pt, temp_pt, mod);
	MulMod(r5, r5,temp_pt,mod);
	InvMod(temp_zz,to_ZZ(2),ord);
	PowerMod(r4,r4,temp_zz,mod);
	MulMod(r3, r3,r4,mod);
	PowerMod(temp_pt, r5,2, mod);
	MulMod(r5 , temp_pt , r6,mod);

	PowerMod(temp_pt, r3, 65,mod);
	InvMod(temp_pt, temp_pt, mod);
	MulMod(r2 , r2,temp_pt,mod);
	InvMod(r4 ,r4,mod);
	InvMod(r6 , r6,mod);
	InvMod(temp_pt, r7, mod);
	MulMod(r3, r3, temp_pt,mod);
	InvMod(temp_pt, r1, mod);
	MulMod(r3 , r3,temp_pt,mod);
	PowerMod(temp_pt, r3, 45, mod);
	MulMod(r2 , r2,temp_pt,mod);
	PowerMod(temp_pt, r3, 8, mod);
	InvMod(temp_pt, temp_pt, mod);
	MulMod(r5 , r5,temp_pt,mod);

	InvMod(temp_zz,to_ZZ(24),ord);
	PowerMod(r5 , r5,temp_zz, mod);
	InvMod(temp_pt, r2, mod);
	MulMod(r6 , r6,temp_pt,mod);
	PowerMod(temp_pt, r4, 16, mod);
	InvMod(temp_pt, temp_pt, mod);
	MulMod(r2 , r2,temp_pt,mod);
	InvMod(temp_zz,to_ZZ(18), ord);
	PowerMod(r2 ,r2, temp_zz, mod);
	InvMod(temp_pt, r5, mod);
	MulMod(r3 , r3,temp_pt,mod);
	InvMod(temp_pt, r2, mod);
	MulMod(r4, r4,temp_pt,mod);
	PowerMod(temp_pt, r2, 30, mod);
	MulMod(r6 , r6,temp_pt,mod);
	InvMod(temp_zz, to_ZZ(60), ord);
	PowerMod(r6,  r6, temp_zz, mod);
	InvMod(temp_pt, r6, mod);
	MulMod(r2 , r2 ,temp_pt,mod);

	ret->at(0)= r1;
	ret->at(1) = r2;
	ret->at(2) = r3;
	ret ->at(3) = r4;
	ret->at(4) = r5;
	ret->at(5) = r6;
	ret->at(6) = r7;

	return ret;
}

// p is a list of points, q is a list of scalars
vector<Cipher_elg>* Prover_toom::toom4_pow(vector<vector<Cipher_elg>*>* p, vector<vector<ZZ>*>* q){
	vector<vector<vector<CurvePoint>*>*>* points_p;
	vector<vector<ZZ>*>* points_q;
	vector<vector<vector<CurvePoint>*>*>* points_temp;
	vector<vector<CurvePoint>*>* points;
	vector<CurvePoint>* ret_u;
	vector<CurvePoint>* ret_v;
	vector<Cipher_elg>* ret = new vector<Cipher_elg>(7);
	long i,l;
	ZZ mod = H.get_mod();
	//auto begin= high_resolution_clock::now();
	points_p = evulation_pow(p);
	points_q = evulation(q);
	points_temp = point_pow(points_p, points_q);
	//auto tstart= high_resolution_clock::now();
	//cout << "toom4, begin: " << duration<double>(tstart - begin).count() << endl;

	points = mult_points(points_temp);
	ret_u = interpolation_pow(points->at(0));
	//auto tstart2= high_resolution_clock::now();
	//cout << "interpolation1 time " << duration<double>(tstart2 - tstart).count() << endl;
	ret_v = interpolation_pow(points->at(1));
	//auto end= high_resolution_clock::now();
	//cout << "interpolation2 time " << duration<double>(end - tstart2).count() << endl;
	l = points->size();
	for(i = 0; i<l; i++){
		delete points->at(i);
		points->at(i)=0;
	}
	delete points;

	for(i = 0; i<7; i++){
		ret->at(i)= Cipher_elg(ret_u->at(i), ret_v->at(i), mod);
	}
	delete ret_u;
	delete ret_v;
	return ret;
}



