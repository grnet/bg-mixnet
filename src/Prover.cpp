
/*
 * Prover.cpp
 *
 *  Created on: 26.10.2010
 *      Author: stephaniebayer
 */

#include "Prover.h"

#include <vector>
#include <map>
#include "Cipher_elg.h"
#include "G_q.h"
#include "Mod_p.h"
#include "Functions.h"
#include "ElGammal.h"
#include "func_pro.h"
#include "fft.h"
#include<fstream>

#include <time.h>
#include <NTL/ZZ.h>
NTL_CLIENT

extern G_q H;
extern ElGammal El;

extern unsigned long commitment_multiplies;
extern unsigned long commitment_lifts;
extern unsigned long commitment_multi_lifts;
#if DEBUG
extern bool debug;
#else
extern bool debug;
#endif

Prover::Prover() {
	// TODO Auto-generated constructor stub

}

Prover::Prover(vector<vector<Cipher_elg>* >* Cin, vector<vector<ZZ>* >* Rin, 
		vector<vector<vector<long>* >* >* piin, 
		map<string, long> num, ZZ genq){

	// set the dimensions of the row and columns according to the user input
	m = num["ciphertext_matrix_rows"]; //number of rows
	n = num["ciphertext_matrix_columns"]; //number of columns
	C = Cin; //sets the reencrypted cipertexts to the input
	R = Rin; //sets the random elements to the input
	pi = piin; // sets the permutation to the input


	fft *fft_new = new fft;
	//o is a root of unity
	o = fft_new->r_o_u(genq, m, H.get_ord());


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

	B = new vector<vector<ZZ>* >(m);//matrix of permuted exponents, exponents are x2^i, i=1, ..N
	B_0 = new vector<ZZ>(n); //vector containing random exponents
	r_B = new vector<ZZ>(m); //random elements used to commit to T
	c_B = new vector<Mod_p>(m); //vector of commitments to rows in T
	a = new vector<ZZ>(2*m); //elements used for reencryption in round 5
	r_a = new vector<ZZ>(2*m); //random elements to commit to elements in a
	c_a = new vector<Mod_p>(2*m); //commitments to elements a
	E = new vector<Cipher_elg>(2*m); //
	rho_a = new vector<ZZ>(2*m); //

	Dl = new vector<ZZ>(2*m+1); //bilinear_map(Y_pi, U, chal_t)
	r_Dl = new vector<ZZ>(2*m+1); //random elements to commit to the C_ls
	c_Dl = new vector<Mod_p>(2*m +1); //commitments to the C_ls

	d_bar = new vector<ZZ>(n);// chal_x8*D_h(m-1) +d
	Delta_bar = new vector<ZZ>(n);//chal_x8*d_h+Delta
	D_h_bar = new vector<ZZ>(n);//sum over the rows in D_h

	B_bar  = new vector<ZZ>(n); // sum over the rows in T multiplied by chal^i

	A_bar = new vector<ZZ>(n); //sum over the rows in Y times the challenges
	D_s_bar = new vector<ZZ>(n); // sum over the rows in U times thes challenges

}
//Destructor deletes all pointers and frees the storage
Prover::~Prover() {
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
	Functions::delete_vector(B);
	delete B_0;
	delete r_B;
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
}



//round_1 picks random elements and commits to the rows of Y
string Prover::round_1(){
	long i;
	string name;
	time_t rawtime;
	time ( &rawtime );

	//calculates commitments to rows of Y
	name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"commitments to A and random values "<<endl;
	ost.close();
	Functions::commit(A,r_A, c_A);

	//write commitments in the file name
	name = "round_1 ";
	name = name + ctime(&rawtime);
	ost.open(name.c_str());
	for (i=0; i<m; i++){
		ost << c_A->at(i)<< " ";
	}
	if (debug) {
		cout << "Commitment multiplies in round 1: " 
		<< commitment_multiplies << endl;
		cout << "Commitment lifts in round 1: " 
		<< commitment_lifts << endl;
		cout << "Commitment multi_lifts in round 1: " 
		<< commitment_multi_lifts << endl;
	}
	return name;

}

//round_3, permuted the exponents in s,  picks random elements and commits to values
string Prover::round_3(string in_name){
	long i;
	ZZ x2;
	vector<vector<ZZ>* >* chal_x2 = new vector<vector<ZZ>* >(m);

	string name;
	time_t rawtime;
	time ( &rawtime );

	//reads in values of s
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	ist >> x2;
	name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"x ist "<<x2<<endl;
	//creates a matrix with entries x2,..., x2^N
	func_pro::set_x2(chal_x2,x2, m,n);

	//permutes chal_x2 according pi to create B
	func_pro::set_B(B, chal_x2, pi);

	//commits to the rows in B
	ost<<"commitment to B "<<endl;
	Functions::commit(B,r_B,c_B);
	ost.close();

	name = "round_3 ";
	name = name + ctime(&rawtime);

	//write data in the file name
	 ost.open(name.c_str());
	for (i=0; i<m;i++){
		ost << c_B->at(i) <<" ";
	}

	Functions::delete_vector(chal_x2);
	if (debug) {
		cout << "Commitment multiplies in round 3: " 
		<< commitment_multiplies << endl;
		cout << "Commitment lifts in round 3: " 
		<< commitment_lifts << endl;
		cout << "Commitment multi_lifts in round 3: " 
		<< commitment_multi_lifts << endl;
	}
	return name;
}

//round_5a calculates Y_pi and the commitments to the vectors alpha, W
void Prover::round_5a(){
	long i;
	ZZ temp, t; //temporary variables
	vector<ZZ>* r = new vector<ZZ>(n);
	vector<ZZ>* v_z = new vector<ZZ>(n); //row containing the challenge alpha
	ZZ ord = H.get_ord();
	time_t rawtime;
	time ( &rawtime );

	//calculate for each value in the first m rows in D: y* A_ij + A_ij -z
	func_pro::set_D(D, A,B, chal_z4, chal_y4);

	//Set the matrix D_h as the Hadamard product of the rows in D
	func_pro::set_D_h(D_h, D);

	for( i=0; i<n;i++){
		v_z->at(i) = chal_z4; //fills the vector z with the challenge z
		NegateMod(r->at(i),to_ZZ(1),ord);
	}

	//Sets the additional row in D to contain -1
	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"Last row in D ist -1 "<<endl;
	D->at(m) = r;
	//random number to commit to last row in A
	ost<<"r_A to commit to last row in A ist "<<0<<endl;
	r_A->at(m) = 0;

	//calculate commitment to
	ost<<"commitment  to vector z"<<endl;
	Functions::commit(v_z, r_z, c_z);
	//calculate commitment to the rows in D_h
	ost<<"Commitments to Hadamard product over D "<<endl;
	Functions::commit(D_h,r_D_h,c_D_h);

	delete v_z;
}

void Prover::round_5b(){

	//picks random values to set B0 and commits to it
	func_pro::commit_B0(B_0, r_B0, c_B0);
	//picks random values to set A and commits to them;
	func_pro::commit_a(a, r_a, c_a);

}

void Prover::round_5c(){
	vector<Cipher_elg>*  e = 0;
	double tstart, tstop, ttime;

	//calculates the value R_t as sum(T(ij)*r(ij));
	func_pro::set_Rb(B,R,R_b);

	tstart = (double)clock()/CLOCKS_PER_SEC;
		e= calculate_e();
	tstop = (double)clock()/CLOCKS_PER_SEC;
	ttime= tstop-tstart;
	//cout << "To calculate the di's took " << ttime << " second(s)." << endl;

	calculate_E(e);
	delete e;
}

string Prover::round_5(string in_name ){
	long i;
	string name;
	time_t rawtime;
	time ( &rawtime );

	//reads the values out of the file
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	ist>> chal_z4;
	ist >> chal_y4;

	 name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"z "<<chal_z4<<" y "<<chal_y4<<endl;
	ost.close();
	round_5a();
	round_5b();
	round_5c();
	//Set name of the output file and open stream
	name = "round_5 ";
	name = name + ctime(&rawtime);


	ost.open(name.c_str());

	//writes the commitments in the file
	ost<<  c_z<< "\n";
	for (i = 0; i<m ; i++){
		ost << c_D_h ->at(i)<< " ";
	}
	ost << c_B0<< "\n ";
	for (i = 0; i<2*m ; i++){
		ost << c_a ->at(i)<< " ";
	}
	ost << "\n";
	for (i = 0; i<2*m ; i++){
		ost << E ->at(i)<< " ";
	}

	delete E;

	if (debug) {
		cout << "Commitment multiplies in round 5: " 
		<< commitment_multiplies << endl;
		cout << "Commitment lifts in round 5: " 
		<< commitment_lifts << endl;
		cout << "Commitment multi_lifts in round 5: " 
		<< commitment_multi_lifts << endl;
	}
	return name;
}

void Prover::round_7a(){
	//Set the rows in D_s as D_s(i) = chal_t_1^i+1*D_h(i) for i<m-1 and D_s(m-1) = sum(chal_x6^i+1 * D_s(i+1) and set last row of D_s to random values and also D(0)

	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"matrix D_s = chal_t_1^i+1*D_h(i) for i<m-1 and D_s(m-1) = sum(chal_x6^i+1 * D_s(i+1) "<<endl;
	func_pro::set_D_s(D_s,D_h,D,chal_x6,r_Dl_bar);

	//calculate the values Dls as Dl(l) = sum(D(i)*D_s->at(i)*chal_y6) for j=n+i-l and commits to the values
	func_pro::commit_Dl(c_Dl,Dl, r_Dl, D, D_s, chal_y6, o);

	//commitments to D(0) and D_s(m)
	ost<<"commitment to D(0) and D_s(m) "<<endl;
	Functions::commit(D->at(0),r_D0,c_D0);
	Functions::commit(D_s->at(m), r_Dm, c_Dm);

	//commitments to prove that the product over the elements in D_h->at(m) is the desired product of n *y + x2n -z
	ost<<"commitment to d "<<endl;
	func_pro::commit_d(d,r_d,c_d);
	func_pro::commit_Delta(Delta, d, r_Delta, c_Delta);
	func_pro::commit_d_h(D_h,d_h,d,Delta, r_d_h, c_d_h);
}

void Prover::round_7b(){
	//calculate B_bar = sum(chal_x6^i B(i)), opening to prove knowledge of B
	func_pro::calculate_B_bar(B_0, B,chal_x6, B_bar );

	//calculate r_B_bar= sum(chal_x6^i r_B(i)), opening to prove knowledge of B
	func_pro::calculate_r_B_bar(r_B, chal_x6,r_B0, r_B_bar );

	//calculate a_bar = sum(chal_x6 ^i a(i)), opening to prove reencryption
	func_pro::calculate_a_bar(a, chal_x6, a_bar);

	//calculate r_a_bar= sum(chal_x6 ^i r_a(i)), opening to prove reencryption
	func_pro::calculate_r_a_bar(r_a, chal_x6, r_a_bar);

	//calculate rho_a_bar = sum(chal_x6 ^i rho_a(i)), opening to prove reencryption
	func_pro::calculate_rho_a_bar(rho_a, chal_x6, rho_bar);
}

string Prover::round_7(string in_name){
	long i;
	long l=2*m;
	string name;
	time_t rawtime;
	time ( &rawtime );
	//reads the values out of the file
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	//reads the vector t_1 and t
	for(i=0; i<l; i++){
		ist>>chal_x6->at(i);
	}
	for (i = 0; i<n; i++){
		ist >> chal_y6->at(i);
	}

	round_7a();
	round_7b();


	//Set name of the output file and open stream
	name = "round_7 ";
	name = name + ctime(&rawtime);


	ofstream ost(name.c_str());
	for (i = 0; i<=l ; i++){
		ost << c_Dl ->at(i)<< " ";
	}
	ost << "\n";
	ost<<c_D0<<"\n";
	ost <<c_Dm<<"\n";
	ost<<c_d<<"\n";
	ost<<c_Delta<<"\n";
	ost<<c_d_h<<"\n";
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

	if (debug) {
		cout << "Commitment multiplies in round 7: " 
		<< commitment_multiplies << endl;
		cout << "Commitment lifts in round 7: " 
		<< commitment_lifts << endl;
		cout << "Commitment multi_lifts in round 7: " 
		<< commitment_multi_lifts << endl;
	}
	return name;
}

void Prover::round_9a(){

	//Calculate D_h_bar = sum(chal^i*D_h(row(i)))
	func_pro::calculate_D_h_bar(D_h_bar,D_h,chal_x8);

	//calculate r_Dh_bar = sum(chal^i*r_Dh_bar(i)), opening to prove correctness of D_h
	func_pro::calculate_r_Dh_bar(r_D_h, chal_x8, r_Dh_bar);

	//calculate d_bar, r_d_bar, Delta_bar, r_Delta_bar, openings to prove product over elements in D_h->at(m-1)
	func_pro::calculate_dbar_rdbar(D_h, chal_x8, d_bar,d,r_D_h, r_d, r_d_bar);
	func_pro::calculate_Deltabar_rDeltabar(d_h, chal_x8, Delta_bar, Delta, r_d_h, r_Delta, r_Delta_bar);
}

void  Prover::round_9b(){
	//A_bar and r_A_bar, openings to prove permutation in D
	func_pro::calculate_A_bar(D, A_bar, chal_x8);
	func_pro::calculate_r_A_bar(r_D0, r_A, r_B, chal_x8, r_z, chal_y4, r_A_bar);

	//D_s_bar and r_Ds_bar, openings to prove correctness of D_s
	func_pro::calculate_D_s_bar(D_s, D_s_bar, chal_x8);
	func_pro::calculate_r_Ds_bar(r_D_h, chal_x6, chal_x8, r_Ds_bar, r_Dm);

	//sum of the random values used to commit to the Dl's, to prover correctness of them
	func_pro::calculate_r_Dl_bar(r_Dl, chal_x8, r_Dl_bar);
}

string Prover::round_9(string in_name){
	long i;
	string name;
	time_t rawtime;
	time ( &rawtime );

	//reads the values out of the file
	ifstream ist(in_name.c_str());
	if(!ist) cout<<"Can't open "<< in_name;
	//reads the vector e
	for (i = 0; i<(signed) chal_x8->size(); i++){
		ist >> chal_x8->at(i);
	}

	name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);

	ost<<"chal_x8 "<< chal_x8->at(0)<<endl;
	ost.close();

	round_9a();
	round_9b();

	//Set name of the output file and open stream
	name = "round_9 ";
	name = name + ctime(&rawtime);

	ost.open(name.c_str());

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

	if (debug) {
		cout << "Commitment multiplies in round 9: " 
		<< commitment_multiplies << endl;
		cout << "Commitment lifts in round 9: " 
		<< commitment_lifts << endl;
		cout << "Commitment multi_lifts in round 9: " 
		<< commitment_multi_lifts << endl;
	}
	return name;
}


Cipher_elg Prover::calculate_e_1(long pos){
	long i;
	Cipher_elg temp, temp_1;

	Cipher_elg::expo(temp, C->at(pos)->at(0), B_0->at(0));
	for (i = 1; i<n; i++){
		Cipher_elg::expo(temp_1, C->at(pos)->at(i), B_0->at(i));
		Cipher_elg::mult(temp,temp, temp_1);
	}

	return temp;
}

Cipher_elg Prover::calculate_e_2(long k){
	long i,j,l;
	Cipher_elg temp, temp_1;
	ZZ mod = H.get_mod();

	temp=Cipher_elg(1,1,mod);
	for (i = 1; i<=m; i++){
		l = k-m+i;
		if((l>0) & (l<=m)){
			for(j = 0; j<n;j++){
				Cipher_elg::expo(temp_1,C->at(i-1)->at(j), B->at(l-1)->at(j));
				Cipher_elg::mult(temp,temp,temp_1);
			}
		}
	}

	return temp;
}

vector<Cipher_elg>* Prover::calculate_e(){
	long k,l;
	Cipher_elg temp, temp_1;
	vector<Cipher_elg>* e = new vector<Cipher_elg>(2*m);

	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"calculate the product of the diagonlas, values E "<<endl;
	e->at(0)= calculate_e_1(m-1);
	ost<<e->at(0)<<" ";

	l=m-1;
	for (k =1; k<=l; k++){
		temp_1 = calculate_e_1(m-k-1);
		temp = calculate_e_2(k);
		Cipher_elg::mult(temp,temp,temp_1);
		e->at(k) = temp;
		ost<<e->at(k)<<" ";
	}
	l=2*m-1;
	for (k = m; k<=l; k++){
		e->at(k) = calculate_e_2(k);
		ost<<e->at(k)<<" ";
	}
	ost<<endl;
	return e;
}

void Prover::calculate_E(vector<Cipher_elg>* e){
	long i,l;
	Mod_p t;
	Mod_p gen = H.get_gen();
	ZZ ord = H.get_ord();
	l=2*m;
	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"reencrypt values in e, pick random rho "<<endl;
	for (i = 0; i<l; i++){
		rho_a->at(i)= RandomBnd(ord);
		ost<<rho_a->at(i)<<" ";
	}
	ost<<endl;
	NegateMod(rho_a->at(m),R_b,ord);
	ost<<"rho(m) = -R_b  m"<<rho_a->at(m)<<" "<<m<<endl;
	ost<<"t=g^a encrypt(t, rho)e "<<endl;
	for (i = 0; i<l; i++){
		 t = gen.expo(a->at(i));
		 E->at(i) = El.encrypt(t,rho_a->at(i))*e->at(i);
		 ost<<t<<" "<<E->at(i)<<endl;
	}
}
