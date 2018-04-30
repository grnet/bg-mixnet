/*
 * Verifier_toom.cpp
 *
 *  Created on: 25.04.2011
 *      Author: stephaniebayer
 */

#include "Verifier_toom.h"

#include<vector>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "multi_expo.h"
#include "func_ver.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <openssl/sha.h>

#include <time.h>
#include "FakeZZ.h"

#include <chrono>
using namespace std::chrono;


NTL_CLIENT

extern G_q G;
extern G_q H;
//extern ElGammal El;
extern long mu;
extern long mu_h;

//OpenMP config
extern bool parallel;
extern int num_threads;

Verifier_toom::Verifier_toom(long& mr, bool do_process) : m_r(mr), do_process_(do_process) {}


Verifier_toom::Verifier_toom(vector<long> num, int m_in, int n_in, long& mr, bool do_process, ElGammal* elgammal) : m_r(mr), do_process_(do_process), elgammal_(elgammal), ped_(n_in) {
	// sets the values of the matrix according to the input
	m = m_in; //number of rows
	n = n_in; //number of columns
	omega = num[3]; //windowsize for multi-expo-technique
	omega_sw = num[4]; //windowsize for multi-expo-technique sliding window and LL
	omega_LL = num[7]; //windowsize for multi-expo-technique of LL

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

void Verifier_toom::set_public_vector(istringstream& f, long n, int o1, int o2, int o3) {
	ped_.set_public_vector(f, n, o1, o2, o3);
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


ZZ Verifier_toom::make_challenge(ZZ* randomness) const {
	stringstream challenge_input;
	*randomness = RandomBnd(H.get_ord());
	challenge_input << *randomness;
	challenge_input << string("||");
	challenge_input << H.get_gen();
	
	unsigned char md[SHA256_DIGEST_LENGTH];
	Functions::sha256(challenge_input.str(), md);
	ZZ ret = ZZFromBytes(md, SHA256_DIGEST_LENGTH) % H.get_ord();
	return ret;
	
}
ZZ Verifier_toom::derive_from_challenge(ZZ& challenge, string id) {
	stringstream challenge_input;
	challenge_input << challenge << "||" << id;
	unsigned char md[SHA256_DIGEST_LENGTH];
	Functions::sha256(challenge_input.str(), md);
	return ZZFromBytes(md, SHA256_DIGEST_LENGTH) % H.get_ord();
}


bool Verifier_toom::check_challenge(ZZ& challenge, ZZ& randomness) const {
	stringstream challenge_input;
	challenge_input << randomness;
	challenge_input << string("||");
	challenge_input << H.get_gen();
	unsigned char md[SHA256_DIGEST_LENGTH];
	Functions::sha256(challenge_input.str(), md);
	ZZ res = ZZFromBytes(md, SHA256_DIGEST_LENGTH) % H.get_ord();
	if (res != challenge) {
		cout << "Fiat Shamir: challenge validation failed!" <<endl;
		return false;
	}
	return true;
}


string Verifier_toom::round_2(const string& input, ZZ& challenge) {
	if (do_process_) {
		long i;
	
		//sets a_Y to the values in the file name
		stringstream ist(input);
		for (i = 0; i<m; i++){
			ist >> c_A->at(i);
		}
	}
	chal_x2 = challenge;
	
	stringstream ost;
	ost << chal_x2;
	return ost.str();
}

string Verifier_toom::round_2(const string& input, ZZ* challenge, ZZ* random_out) {
	*challenge = make_challenge(random_out);
	return round_2(input, *challenge);
}

string Verifier_toom::round_2(const string& input, ZZ& challenge, ZZ& random_in) {
	if (!check_challenge(challenge, random_in)) {
		return string();
	}
	return round_2(input, challenge);
}

string Verifier_toom::round_4(const string& input,ZZ& challenge) {
	chal_z4 = derive_from_challenge(challenge, "chal_z4");
	chal_y4 = derive_from_challenge(challenge, "chal_y4");

        long i;
        stringstream ist(input);
        //sets a_T to the values in the file
        for (i = 0; i<m; i++){
          ist >> c_B->at(i);
        }
	//Set name of the output file and open stream
	
	stringstream ost;
	ost<< chal_z4<<"\n";
	ost<<chal_y4 ;
	return ost.str();
}
string Verifier_toom::round_4(const string& input, ZZ* challenge, ZZ* random_out) {
	*challenge = make_challenge(random_out);
	return round_4(input, *challenge);
}

string Verifier_toom::round_4(const string& input, ZZ& challenge, ZZ& random_in) {
	if (!check_challenge(challenge, random_in)) {
		return string();
	}
	return round_4(input, challenge);
}

string Verifier_toom::round_6(const string& input, ZZ& challenge) {
	long i,l;
        // TODO check if block of code below needs to be guarded by do_process_
        //reads the values out of the file name
        stringstream ist(input);
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
        // end block
	
	//sets the vector t to the values temp, temp^2,...
	func_ver::fill_vector(chal_x6, challenge);

	//sets the vector t to the values temp, temp^2,...
	func_ver::fill_vector(chal_y6, challenge);

	l=2*m;
	stringstream ost;
	for (i=0; i<l; i++){
		ost << chal_x6->at(i)<<" ";
	}
	ost << "\n";
	for (i=0; i<n; i++){
		ost << chal_y6->at(i)<<" ";
	}
	ost << "\n";
	return ost.str();
}
string Verifier_toom::round_6(const string& input, ZZ* challenge, ZZ* random_out) {
	*challenge = make_challenge(random_out);
	return round_6(input, *challenge);
}

string Verifier_toom::round_6(const string& input, ZZ& challenge, ZZ& random_in) {
	if (!check_challenge(challenge, random_in)) {
		return string();
	}
	return round_6(input, challenge);
}



string Verifier_toom::round_6_red(const string& input, vector<vector<Cipher_elg>* >* enc, ZZ& challenge){
	long i;
	Cipher_elg c;
	Mod_p temp;
	//reads the values out of the file name
	stringstream ist(input);
	for(i=0;i<mu_h;i++){
		ist>>C_c->at(i);
	}
	for(i=0; i<mu_h; i++){
		ist>>c_a_c ->at(i);
	}

        // TODO check if block of code below needs to be guarded by do_process_
	if (true) {
		calculate_c(c, enc);

		temp = Mod_p(curve_zeropoint(),H.get_mod()); //a_a_c->at(mu-1) should equal the commitment to 0
		if((c_a_c->at(mu-1)==temp) & (c == C_c->at(mu-1))){
			//sets the vector x to the values temp, temp^2,...
			func_ver::fill_vector(x, challenge);
		} else {
                  // explicit init
                  for (unsigned int i = 0; i < x->size(); i++) {
                    x->at(i) = ZZ(NTL::ZZ());
                  }
                }
	} else {
		func_ver::fill_vector(x, challenge);
	}
        // end block
	
	stringstream ost;
	for (i=0; i<mu_h; i++){
		ost << x->at(i)<<" ";
	}
	return ost.str();

}
string Verifier_toom::round_6_red(const string& input, vector<vector<Cipher_elg>* >* enc, ZZ* challenge, ZZ* random_out) {
	*challenge = make_challenge(random_out);
	return round_6_red(input, enc, *challenge);
}

string Verifier_toom::round_6_red(const string& input, vector<vector<Cipher_elg>* >* enc, ZZ& challenge, ZZ& random_in) {
	if (!check_challenge(challenge, random_in)) {
		return string();
	}
	return round_6_red(input, enc, challenge);
}


string Verifier_toom::round_6_red1(const string& input, ZZ& challenge){
	long i,l;
	Mod_p temp, com;
	Cipher_elg C;
	ZZ mod = G.get_mod();
        bool use_challenge = true;

	if (do_process_) {
		//calculates the product of the the old commitments a_a_c to the power of x
		calculate_ac(com);
		//Combines the committed values to B in the vector c_B_small with challenges x
		reduce_c_B();

                // explicit init of a_c_bar before calculating C
                a_c_bar = ZZ(NTL::ZZ());

		//calulates the new value C
		calculate_C(C, C_c, x);
	} else {
          // explicit init
          com = Mod_p(true);
          C = Cipher_elg(true);
          use_challenge = false;
        }
	
	stringstream ost;
        if (m_r <= 4) {
		//reads the values out of the file name
		stringstream ist(input);
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

		temp = Mod_p(curve_zeropoint(),mod);//a_a_c->at(mu-1) should equal the commitment to 0
		use_challenge = use_challenge &&
		  (c_a_c->at(mu-1)==temp) &&
		  (com == ped_.commit(a_c_bar, r_ac_bar)) &&
		  (C == C_c->at(mu-1));
		if(use_challenge) {
			//sets the vector chal_x6 to the values temp, temp^2,...
			ZZ challenge1 = derive_from_challenge(challenge, "chal_x6");
			func_ver::fill_vector(chal_x6, challenge1);

			//sets the vector chal_y6 to the values temp, temp^2,...
			ZZ challenge2 = derive_from_challenge(challenge, "chal_y6");
			func_ver::fill_vector(chal_y6, challenge2);
		} else {
			// explicit init
			for (unsigned int i = 0; i < chal_x6->size(); i++) {
				chal_x6->at(i) = ZZ(NTL::ZZ());
			}
			for (unsigned int i = 0; i < chal_y6->size(); i++) {
				chal_y6->at(i) = ZZ(NTL::ZZ());
			}
		}


		l=2*m;
		for (i=0; i<l; i++){
			ost << chal_x6->at(i)<<" ";
		}
		ost << "\n";
		for (i=0; i<n; i++){
			ost << chal_y6->at(i)<<" ";
		}
		ost << "\n";
	}
	return ost.str();
}
string Verifier_toom::round_6_red1(const string& input, ZZ* challenge, ZZ* random_out) {
	*challenge = make_challenge(random_out);
	return round_6_red1(input, *challenge);
}

string Verifier_toom::round_6_red1(const string& input, ZZ& challenge, ZZ& random_in) {
	if (!check_challenge(challenge, random_in)) {
		return string();
	}
	return round_6_red1(input, challenge);
}


string  Verifier_toom::round_8(const string& input, ZZ& challenge){
	long i,l;

	if (do_process_) {
		//reads the values out of the file name
		stringstream ist(input);

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
	}
	func_ver::fill_x8(chal_x8, basis_chal_x8, mul_chal_x8, omega, challenge);
	l= chal_x8->size();

	stringstream ost;
	for (i = 0; i<l; i++){
		ost << chal_x8->at(i)<< " ";
	}
	return ost.str();
}
string Verifier_toom::round_8(const string& input, ZZ* challenge, ZZ* random_out) {
	*challenge = make_challenge(random_out);
	return round_8(input, *challenge);
}

string Verifier_toom::round_8(const string& input, ZZ& challenge, ZZ& random_in) {
	if (!check_challenge(challenge, random_in)) {
		return string();
	}
	return round_8(input, challenge);
}

bool Verifier_toom:: round_10(const string& input,vector<vector<Cipher_elg>* >* enc, vector<vector<Cipher_elg>* >* C){
	bool b[11];

	long i;
	//reads the values out of the file name
	stringstream ist(input);
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

	if (c_a->at(4)==c_a_c->at(3)) return false;
	
	//Check that the D_hi's are constructed correctly
	thread t0(func_ver::check_Dh_op, c_Dh, mul_chal_x8, D_h_bar, r_Dh_bar, omega_LL, std::ref(ped_), std::ref(b[0]));
	//Check that matrix D is constructed correctly
	thread t1(func_ver::check_D_op, c_D0, c_z, c_A, c_B, chal_x8, chal_y4, A_bar, r_A_bar, n, std::ref(ped_), std::ref(b[1]));
	//Check that D_s is constructed correctly
	thread t2(func_ver::check_Ds_op, c_Ds, c_Dh, c_Dm, chal_x6, chal_x8, Ds_bar, r_Ds_bar, std::ref(ped_), std::ref(b[2]));
	//Check that the Dl's are correct
	thread t3(func_ver::check_Dl_op, c_Dl, chal_x8, A_bar, Ds_bar, chal_y6, r_Dl_bar, std::ref(ped_), std::ref(b[3]));
	//Check that vector d was constructed correctly
	thread t4(func_ver::check_d_op, c_Dh, c_d, chal_x8, d_bar, r_d_bar, std::ref(ped_), std::ref(b[4]));
	//Check that Deltas are constructed correctly
	thread t5(func_ver::check_Delta_op, c_dh, c_Delta, chal_x8, Delta_bar, d_bar, r_Delta_bar, chal_x2, chal_z4, chal_y4, std::ref(ped_), std::ref(b[5]));
	thread t6(&Verifier_toom::check_B, this, std::ref(b[6]));
	thread t7(&Verifier_toom::check_a, this, std::ref(b[7]));
	thread t8(&Verifier_toom::check_c, this, enc, std::ref(b[8])); //Both commitments shoud be com(0,0)
	thread t9(&Verifier_toom::check_E, this, C, std::ref(b[9]));
	thread t10(&Verifier_toom::check_ac, this, std::ref(b[10]));

	t0.join();
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
	t8.join();
	t9.join();
	t10.join();


        bool ret = true;
	for (int i = 0; i < 11; i++) {
		if (!b[i]) {
			cout << "failed on " << i <<endl;
			ret = false;
		}
	}
	return ret;

}

bool Verifier_toom:: round_10_red(const string& input,vector<vector<Cipher_elg>* >* enc, vector<vector<Cipher_elg>* >* C){
	bool b[11];
	long i;
	stringstream ist(input);
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
	
	
	if((c_a->at(4)!=c_a_c->at(3))) {
		return false;
	}
	
	thread t0(func_ver::check_Dh_op, c_Dh, mul_chal_x8, D_h_bar, r_Dh_bar, omega_sw, std::ref(ped_), std::ref(b[0]));
	thread t1(func_ver::check_D_op, c_D0, c_z, c_A, c_B, chal_x8, chal_y4, A_bar, r_A_bar, n, std::ref(ped_), std::ref((b[1])));
	thread t2(func_ver::check_Ds_op, c_Ds, c_Dh, c_Dm, chal_x6, chal_x8, Ds_bar, r_Ds_bar, std::ref(ped_), std::ref(b[2]));
	thread t3(func_ver::check_Dl_op, c_Dl, chal_x8, A_bar, Ds_bar, chal_y6, r_Dl_bar, std::ref(ped_), std::ref(b[3]));
	thread t4(func_ver::check_d_op, c_Dh, c_d, chal_x8, d_bar, r_d_bar, std::ref(ped_), std::ref(b[4]));
	thread t5(func_ver::check_Delta_op, c_dh, c_Delta, chal_x8, Delta_bar, d_bar, r_Delta_bar, chal_x2, chal_z4, chal_y4, std::ref(ped_), std::ref(b[5]));
	thread t6(&Verifier_toom::check_B_red, this, std::ref(b[6]));
	thread t7(&Verifier_toom::check_a, this, std::ref(b[7]));
	thread t8(&Verifier_toom::check_c_red, this, std::ref(b[8])); //Both commitments shoud be com(0,0)
	thread t9(&Verifier_toom::check_E_red, this, C,std::ref(b[9]));
	thread t10(&Verifier_toom::check_ac, this, std::ref(b[10]));

	t0.join();
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
	t8.join();
	t9.join();
	t10.join();
        bool ret = true;
	for (int i = 0; i < 11; i++) {
		if (!b[i]) {
			cout << "failed on " << i <<endl;
			ret = false;
		}
	}
	return ret;

}

void par_challenge(ZZ& chal_x2, ZZ& ord, int n, vector<ZZ>* v_chal) {
	ZZ chal_temp=to_ZZ(1);
	for(int j=0; j<n; j++){
		// here is the problem for paralelizing, we depend on the previous chal_temp value
		MulMod(chal_temp, chal_temp, chal_x2, ord);
		v_chal->at(j)=chal_temp;
	}
}

void par_expo_mult(Cipher_elg& temp, vector<Cipher_elg>* enc, vector<ZZ>* v_chal, int omega) {
	multi_expo::expo_mult(temp, enc, v_chal, omega);
}

// ygi: this is the bottleneck function!
void Verifier_toom::calculate_c(Cipher_elg& c, vector<vector<Cipher_elg>* >* enc){
	long i, j;
	ZZ chal_temp;
	ZZ ord = H.get_ord();
	vector<ZZ>* v_chal= new vector<ZZ>(n);

	chal_temp=to_ZZ(1);
	c=Cipher_elg(curve_zeropoint(),curve_zeropoint(),H.get_mod());
	
	//#pragma omp parallel for num_threads(num_threads) if(parallel)
	for(i=0; i<m ; i++){
		//par_challenge(chal_x2, ord, n, v_chal);
		for(j=0; j<n; j++){
			// here is the problem for paralelizing, we depend on the previous chal_temp value
			MulMod(chal_temp, chal_temp, chal_x2, ord);
			v_chal->at(j)=chal_temp;
		}
	}
	
	Cipher_elg temp[m];
	#pragma omp parallel for num_threads(num_threads) if(parallel)
	for(i=0; i<m ; i++){
		par_expo_mult(temp[i], enc->at(i), v_chal, omega);
		//multi_expo::expo_mult(temp, enc->at(i), v_chal, omega);
		//Cipher_elg::mult(c,c,temp);
	}
	for(i=0; i<m ; i++){
		Cipher_elg::mult(c,c,temp[i]);
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

        // note: a_c_bar is 0 the first time this is called (in round 6)
        // it is initialized at some later point and called again in round 10
	NegateMod(t_1, a_c_bar, ord);
	Mod_p::expo(temp, gen, t_1);
	C = elgammal_->encrypt(temp,to_ZZ(0));
	Cipher_elg::mult(C,C, C_c->at(0));
	for(i=1; i<mu_h; i++){
		Cipher_elg::expo(temp_1, C_c->at(i), x->at(i-1));
		Cipher_elg::mult(C,C, temp_1);
	}

}

void Verifier_toom::check_B(bool& b){
	long i,j;
	Mod_p temp, temp_1, t_B, co_B;
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

	co_B= ped_.commit_opt(B_bar,r_B_bar);
//	cout<<"B "<<t_B<<" "<<co_B<<endl;
	b = (t_B == co_B);
}

void Verifier_toom::check_B_red(bool& b){ 
	long i,j;
	Mod_p temp, temp_1, t_B, co_B;
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
	co_B= ped_.commit_opt(B_bar,r_B_bar);
//	cout<<"B "<<t_B<<endl;
//	cout<<"B "<<co_B<<endl;
	b = (t_B == co_B);
}


void Verifier_toom::check_a(bool& b){
	long i;
	Mod_p t_a, co_a;
	vector<ZZ>* chal_temp = new vector<ZZ>(8);

	chal_temp->at(0)=1;
	for(i=1; i<8; i++){
		chal_temp->at(i)=chal_x8->at(i-1);
	}
	multi_expo::multi_expo_LL(t_a, c_a, chal_temp, omega_sw);
	co_a = ped_.commit_sw(a_bar, r_a_bar);

	//cout<<"a "<<t_a<<" "<<co_a<<" "<<c_a->at(4)<<endl;
	delete chal_temp;
	b = (t_a == co_a);
}


void Verifier_toom::check_c(vector<vector<Cipher_elg>* >* enc, bool& b){
	Cipher_elg c,C;

	calculate_c(c, enc);

	calculate_C(C,C_c, chal_x6);

 	b = ((C_c->at(mu-1)==c) & (E->at(4)==C));
}

void Verifier_toom::check_c_red(bool& b){
	Cipher_elg C;

	calculate_C(C,C_c, chal_x6);
	//cout<<C<<endl;
	//cout<<E->at(4)<<endl;
 	b = (E->at(4)==C);
}


void Verifier_toom::check_E(vector<vector<Cipher_elg>* >* C, bool& b){
	long i,j;
	Mod_p temp;
	Mod_p gen = H.get_gen();
	Cipher_elg temp_1,  temp_2, t_D, c_D;
	vector<ZZ>* chal_1_temp= new vector<ZZ>(4);
	vector<ZZ>* chal_2_temp= new vector<ZZ>(4);

	for(i = 0; i<3; i++){
		chal_1_temp->at(i) = chal_x6->at(2-i);
	}
	chal_1_temp->at(3) = 1;

	for (i = 0; i < m_r; i++) {
		C_small->at(i) = new vector<Cipher_elg>(n);
	}
	
	{
        //PARALLELIZE
		#pragma omp parallel for collapse(2) num_threads(num_threads) if(parallel)
		for(i=0; i<m_r;i++){
            //#pragma omp parallel for num_threads(num_threads) if(parallel)
			for(j=0; j<n; j++){
	            multi_expo::multi_expo_LL(C_small->at(i)->at(j), C->at(4*i)->at(j), C->at(4*i+1)->at(j),C->at(4*i+2)->at(j),C->at(4*i+3)->at(j), chal_1_temp, omega_sw);
			}
		}
	}

	for(i=0; i<3;i++){
		chal_2_temp->at(i)=chal_x8->at(2-i);
	}
	chal_2_temp->at(3)=to_ZZ(1);


	Mod_p::expo(temp, gen,a_bar);
	temp_1 = elgammal_->encrypt(temp, rho_bar);
	multi_expo::expo_mult(temp_2, C_small, chal_2_temp, B_bar, omega);
	Cipher_elg::mult(c_D,temp_1,temp_2);
	//c_D=temp_1*temp_2;

	multi_expo::expo_mult(t_D, E, basis_chal_x8 , omega);

	delete chal_1_temp;
	delete chal_2_temp;
	Functions::delete_vector(C_small);
//	cout<<"E"<<t_D<<endl;
//	cout<<"E"<<c_D<<endl;
	b = (t_D==c_D);
}

void Verifier_toom::check_E_red(vector<vector<Cipher_elg>* >* C, bool& b){
	long i,j,l;
	Mod_p temp;
	Mod_p gen = H.get_gen();
	Cipher_elg temp_1,  temp_2, t_D, c_D;
	vector<ZZ>* x_temp= new vector<ZZ>(4);
	vector<ZZ>* chal_1_temp= new vector<ZZ>(4);
	vector<ZZ>* chal_2_temp= new vector<ZZ>(4);
	vector<vector<Cipher_elg>* >* C_small_temp=0;
	//vector<Cipher_elg>* row_C;

	for(i = 0; i<3; i++){
		x_temp->at(i) = x->at(2-i);
	}
	x_temp->at(3) = 1;

	l=mu*m_r;
	
	for (i = 0; i < l; i++) {
		C_small->at(i) = new vector<Cipher_elg>(n);
	}
	
	#pragma omp parallel for collapse(2) num_threads(num_threads) if(parallel)
	for(i=0; i<l;i++){
		//row_C = new vector<Cipher_elg>(n);
		for(j=0; j<n; j++){
			multi_expo::multi_expo_LL(C_small->at(i)->at(j), C->at(4*i)->at(j), C->at(4*i+1)->at(j),C->at(4*i+2)->at(j),C->at(4*i+3)->at(j), x_temp, omega_sw);
		}
		//C_small->at(i)=row_C;
	}
	
	for(i = 0; i<3; i++){
		chal_1_temp->at(i) = chal_x6->at(2-i);
	}
	chal_1_temp->at(3) = 1;

	C_small_temp = new vector<vector<Cipher_elg>* >(m_r);
	for(i=0; i<m_r;i++){
		C_small_temp->at(i) = new vector<Cipher_elg>(n);
	}
	
	#pragma omp parallel for collapse(2) num_threads(num_threads) if(parallel)
	for(i=0; i<m_r;i++){
		for(j=0; j<n; j++){
			multi_expo::multi_expo_LL(C_small_temp->at(i)->at(j), C_small->at(4*i)->at(j), C_small->at(4*i+1)->at(j),C_small->at(4*i+2)->at(j),C_small->at(4*i+3)->at(j), chal_1_temp, omega);
		}
	}

	for(i=0; i<3;i++){
		chal_2_temp->at(i)=chal_x8->at(2-i);
	}
	chal_2_temp->at(3)=to_ZZ(1);

	Mod_p::expo(temp, gen,a_bar);
	temp_2 = elgammal_->encrypt(temp, rho_bar);
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
	b = (t_D ==c_D);
}

void Verifier_toom::check_ac(bool& b){
	Mod_p t_a_c, co_a_c, temp;
	int i;

	t_a_c = c_a_c->at(0);
	for(i=1; i<7; i++){
		Mod_p::expo(temp, c_a_c->at(i), chal_x6->at(i-1));
		Mod_p::mult(t_a_c,t_a_c, temp);
	}
	co_a_c = ped_.commit_sw(a_c_bar, r_ac_bar);
//	cout<<"ac "<<t_a_c<<" "<<c_a_c;
	b = (t_a_c ==co_a_c);
}

