/*
 * Functions.cpp
 *
 *  Created on: 26.10.2010
 *      Author: stephaniebayer
 */

#include "Functions.h"
#include "G_q.h"
#include "ElGammal.h";
#include "Pedersen.h"
#include "Cipher_elg.h"

#include <NTL/ZZ.h>
NTL_CLIENT

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include <time.h>
#include <fstream>

extern G_q G;
extern G_q H;
extern ElGammal El;
extern Pedersen Ped;

Functions::Functions() {
	// TODO Auto-generated constructor stub

}

Functions::~Functions() {
	// TODO Auto-generated destructor stub
}


void Functions::read_config(map<string,long> & num, ZZ & genq) {
	ifstream ist, ist1;
	string name;
	string line;
	string token;
	string group_description_file_name;
	string number_of_bits_prime_order_q;
	string number_of_bits_prime_p;
	string number_of_bits_prime_p1;
	vector<string> lines;
	vector<ZZ>* pq;
	ZZ ran;
	long i, lq, lp, lp1;

	name = "config";
	ist.open (name.c_str());
	if (!ist1) cout << "Can't open " << name.c_str();

	while (getline(ist, line)) {
		istringstream iss(line);
		getline(iss, token, '=');
		if (token == "optimization_method") {
			getline(iss, token, '=');
			num["optimization_method"] = tolong(token);
		} else if (token == "number_of_ciphertexts") {
			getline(iss, token, '=');
			num["number_of_ciphertexts"] = tolong(token);
		} else if (token == "ciphertext_matrix_rows") {
			getline(iss, token, '=');
			num["ciphertext_matrix_rows"] = tolong(token);
		} else if (token == "ciphertext_matrix_columns") {
			getline(iss, token, '=');
			num["ciphertext_matrix_columns"] = tolong(token);
		} else if (token == "window_size_multi_exponentiation") {
			getline(iss, token, '=');
			num["window_size_multi_exponentiation"] = tolong(token);
		} else if (token == "window_size_multi_exponentiation_lim_lee"){
			getline(iss, token, '=');
			num["window_size_multi_exponentiation_lim_lee"] = 
								tolong(token);
		} else if (token ==  
				 "window_size_multi_exponentiation_brickels") {
			getline(iss, token, '=');
			num["window_size_multi_exponentiation_brickels"] = 
								tolong(token);
		} else if (token == "modular_groups") {
			getline(iss, token, '=');
			num["modular_groups"] = tolong(token);
		} else if (token == "group_description_file_name") {
			getline(iss, token, '=');
			group_description_file_name.assign(token);
		} else if (token == "number_of_bits_prime_order_q") {
			getline(iss, token, '=');
			number_of_bits_prime_order_q.assign(token);
		} else if (token == "number_of_bits_prime_p") {
			getline(iss, token, '=');
			number_of_bits_prime_p.assign(token);
		} else if (token == "number_of_bits_prime_p1") {
			getline(iss, token, '=');
			number_of_bits_prime_p1.assign(token);
		}
	}
	ist.close();
	if (group_description_file_name != "0") {
		cout << "if ";
		if (num["modular_groups"] == 0) {
			pq = new vector<ZZ>(4);
			ifstream ist1(group_description_file_name.c_str(), 
								ios::in);
			if (!ist1) cout << "Can't open " << token.c_str();
			for (i = 0; i < 4; i++) {
				ist1 >> pq->at(i);
			}
			ist1.close();
			cout << NumBits(pq->at(1)) << " " <<
				NumBits(pq->at(0)) << endl;
			G = G_q(pq->at(2), pq->at(1), pq->at(0));
			H = G_q(pq->at(2), pq->at(1), pq->at(0));
		} else {
			pq = new vector<ZZ>(6);
			ifstream ist1(line.c_str(), ios::in);
			if (!ist1) cout << "Can't open " << line.c_str();
			for (i = 0; i < 6; i++) {
				ist1 >> pq->at(i);
			}
			ist1.close();
			cout << NumBits(pq->at(1)) << " " <<
				NumBits(pq->at(0)) << " " <<
				NumBits(pq->at(4)) << endl;
			G = G_q(pq->at(2), pq->at(1), pq->at(0));
			H = G_q(pq->at(5), pq->at(1), pq->at(4));
		}
	} else {
		if (num["modular_groups"] == 0) {
			lq = tolong(number_of_bits_prime_order_q);
			lp = tolong(number_of_bits_prime_p);

			pq = new vector<ZZ>(4);

			SetSeed(to_ZZ(time(0)));
			//find_group(pq, lq, lp, num[1]);
			find_group(pq, lq, lp, 262144);
			//find_group(pq, lq, lp, 2);

			G = G_q(pq->at(2), pq->at(1), pq->at(0));
			H = G_q(pq->at(2), pq->at(1), pq->at(0));
		} else {
			cout << "else ";

			lq = tolong(number_of_bits_prime_order_q);
			lp = tolong(number_of_bits_prime_p);
			lp1 = tolong(number_of_bits_prime_p1);

			pq = new vector<ZZ>(6);

			SetSeed(to_ZZ(time(0)));
			find_groups(pq, lq, lp, lp1, 
					num["ciphertext_matrix_rows"]);
			//find_groups(pq, lq, lp, lp1, 262144 );

			G = G_q(pq->at(2), pq->at(1), pq->at(0));
			H = G_q(pq->at(5), pq->at(1), pq->at(4));
		}
	}
	El.set_group(H);
	ran = RandomBnd(H.get_ord());
	El.set_sk(ran);
	genq = pq->at(3);
	delete pq;
}


long Functions::tolong(string s){
	 //using namespace std;
	long n;
	stringstream ss(s); // Could of course also have done ss("1234") directly.


	 if( (ss >> n).fail() )
	 {
	    //error
	 }


	 return n;

}

string Functions::tostring(long n){

	stringstream ss;
	ss<<n;
	return ss.str();
}


//Creates a matrix of N random elements, if N<n*m 1 is encrypted in the last elements
vector<vector<Cipher_elg>* >* Functions:: createCipher(map<string, long> num){
	long N = num["number_of_ciphertexts"];
	long m = num["ciphertext_matrix_rows"];
	long n = num["ciphertext_matrix_columns"];
	vector<vector<Cipher_elg>* >* C=new vector<vector<Cipher_elg>* >(m);
	vector<Cipher_elg>* r = 0;
	Cipher_elg temp;
	ofstream ost;
	ZZ ran_2,ord;
	Mod_p ran_1;
	long count;
	long i,j;

	count = 1;
	ord=H.get_ord();
	ost.open("cipher_c.txt");
	for (i=0; i<m; i++){
		r  = new vector<Cipher_elg>(n);

		for (j = 0; j <n; j++){
			if (count <= N){
				ran_2 = RandomBnd(ord);
				ran_1 = H.random_el(0);
				temp = El.encrypt(ran_1, ran_2);
				r->at(j)=temp;
				ost<<temp<<endl;
				count ++;
			}
			else
			{
				ran_2 = RandomBnd(ord);
				temp = El.encrypt(1,ran_2);
				r->at(j)=temp;
				count++;
			}
		}
		C->at(i)=r;
	}
ost.close();

	return C;
}

void Functions:: createCipher(vector<vector<Cipher_elg>* >* C, 
							map<string, long> num) {
	long N = num["number_of_ciphertexts"];
	long m = num["ciphertext_matrix_rows"];
	long n = num["ciphertext_matrix_columns"];
	vector<Cipher_elg>* r = 0;
	Cipher_elg temp;
	ZZ ran_2,ord;
	Mod_p ran_1;
	long count;
	long i,j;
	ofstream ost;

	count = 1;
	ord=H.get_ord();

/*	string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<"q "<<ord<<" p"<<H.get_mod()<<endl;
	ost<<"Ciphertext c "<<endl;*/

	//ost.open("cipher.txt");
	for (i=0; i<m; i++){
		r  = new vector<Cipher_elg>(n);
		for (j = 0; j <n; j++){
			if (count <= N){
				ran_2 = RandomBnd(ord);
				ran_1 = H.random_el(0);
				temp = El.encrypt(ran_1, ran_2);
				r->at(j)=temp;
				count ++;
					//ost<<temp<<" ";
			}
			else
			{
				ran_2 = RandomBnd(ord);
				temp = El.encrypt(1,ran_2);
				r->at(j)=temp;
				count++;
			}
		}
			//ost<<endl;
		C->at(i)=r;
	}
}



//Creates a matrix of random numbers
vector<vector<ZZ>* >* Functions::randomEl(map<string, long> num){
	long m = num["ciphertext_matrix_rows"];
	long n = num["ciphertext_matrix_columns"];
	vector<vector<ZZ>* >* R = new vector<vector<ZZ>* >(m);
	vector<ZZ>* r = 0;
	ZZ ran,ord;
	long i,j;
	ord= H.get_ord();
	for (i=0; i<m; i++){
		r = new vector<ZZ>(n);

		for (j = 0; j <n; j++){
			r->at(j) = RandomBnd(ord);
		}
		R->at(i)=r;
	}
	return R;
}

void Functions::randomEl(vector<vector<ZZ>*>* R, map<string, long> num){
	long m = num["ciphertext_matrix_rows"];
	long n = num["ciphertext_matrix_columns"];
	vector<ZZ>* r = 0;
	ZZ ran,ord;
	long i,j;
	ord= H.get_ord();

	string name = "random.txt";
	ofstream ost;
	ost.open(name.c_str());
	for (i=0; i<m; i++){
		r = new vector<ZZ>(n);

		for (j = 0; j <n; j++){
			r->at(j) = RandomBnd(ord);
					ost<<r->at(j)<<" ";
		}

		//		ost<<endl;
		R->at(i)=r;
	}
	ost.close();
}

//reencrypts the ciphertexts e using the permutation pi and the random elements R
vector<vector<Cipher_elg>* >*  Functions::reencryptCipher(
					vector<vector<Cipher_elg>* >* e, 
					vector<vector<vector<long>* >* >* pi, 
					vector<vector<ZZ>*>* R, 
					map<string, long> num) {
	long m = num["ciphertext_matrix_rows"];
	long n = num["ciphertext_matrix_columns"];
	vector<vector<Cipher_elg>* >* C= new vector<vector<Cipher_elg>* >(m);
	vector<Cipher_elg>* r =0;
	Cipher_elg temp;
	ZZ ran;
	long i,j;
	long row, col;
	for (i=0; i<m; i++){
		r =new vector<Cipher_elg>(n);
		for (j = 0; j <n; j++){
			temp = El.encrypt(1,R->at(i)->at(j));
			row = pi->at(i)->at(j)->at(0);
			col = pi->at(i)->at(j)->at(1);
			Cipher_elg::mult(r->at(j), temp, e->at(row)->at(col));
		}
		C->at(i)=r;
	}
	return C;
}


void Functions::reencryptCipher(vector<vector<Cipher_elg>* >* C, 
				vector<vector<Cipher_elg>* >* e, 
				vector<vector<vector<long>* >* >* pi,
				vector<vector<ZZ>*>* R, 
				map<string, long> num) {
	long m = num["ciphertext_matrix_rows"];
	long n = num["ciphertext_matrix_columns"];
	vector<Cipher_elg>* r =0;
	Cipher_elg temp;
	ZZ ran;
	long i,j;
	long row, col;
	string name = "reencrypted_ciper.txt";
	ofstream ost;
	ost.open(name.c_str());
	for (i=0; i<m; i++){
		r =new vector<Cipher_elg>(n);
		for (j = 0; j <n; j++){
			temp = El.encrypt(1,R->at(i)->at(j));
			row = pi->at(i)->at(j)->at(0);
			col = pi->at(i)->at(j)->at(1);
			Cipher_elg::mult(r->at(j), temp, e->at(row)->at(col));
					ost<<r->at(j)<<" ";
		}
		//	ost<<endl;
		C->at(i)=r;
	}
	ost.close();
}

//Returns the Hadamard product of x and y
void Functions::Hadamard(vector<ZZ>* ret, vector<ZZ>* x, vector<ZZ>* y){

	long n, m,i;
	ZZ ord=H.get_ord();
	n=x->size();
	m =y->size();

	if(m !=n){
		 cout<< "Not possible"<< endl;
	}
	else{
		 for (i = 0; i<n; i++){
			 MulMod(ret->at(i),x->at(i), y->at(i), ord);
		 }
	}
}

//returns the bilinear map of x and y, defined as x(y¡t)^T
ZZ Functions::bilinearMap(vector<ZZ>* x, vector<ZZ>* y, vector<ZZ>* t){
	long i, l;
	ZZ result,ord, tem;

	vector<ZZ>* temp = new vector<ZZ>(x->size());

	ord = H.get_ord();
	Hadamard(temp, y,t);
	l= x->size();
	result =0;
	for (i= 0; i<l; i++){
		MulMod(tem,x->at(i), temp->at(i), ord);
		AddMod(result, result, tem,ord);
	}
	delete temp;
	return result;
}

void Functions::find_stat_group(){
	ZZ q1_t, q2_t, q, q1, q2, q3, q4, p,a;
	ZZ genq, genq1, genq2, genq3, genq4, gen, gen1,gen2, gen3, gen4,F,an;
	bool b, bo, bol;
	long l, lq, lq1, lq2, lq3, lq4, lp, m , logl, j,i;
	string name;

	m=64;
	bol=false;
	while(bol ==false){
		lq=100;
		l = lq-NumBits(2*m);
		//generates q as 2*2*m*q1*q2+1 and tests if q can be prime
		b=false;
		bol=true;
		while(b==false){
			b=new_q(q,q1_t,q2_t,m,l);
		}
		b = false;
		while(b==false){
			b=check_q(a,q,q1_t,q2_t,m,l);
		}
		genq = a;

		lq1=200;
		l = lq1-NumBits(2*m);
		//generates q as 2*2*m*q1*q2+1 and tests if q can be prime
		b=false;
		bol=true;
		while(b==false){
			b=new_q(q1,q1_t,q2_t,m,l);
		}
		b = false;
		while(b==false){
			b=check_q(a,q1,q1_t,q2_t,m,l);
		}
		genq1 = a;

		lq2=224;
		l = lq2-NumBits(2*m);
		//generates q as 2*2*m*q1*q2+1 and tests if q can be prime
		b=false;
		bol=true;
		while(b==false){
			b=new_q(q2,q1_t,q2_t,m,l);
		}
		b = false;
		while(b==false){
			b=check_q(a,q2,q1_t,q2_t,m,l);
		}
		genq2 = a;

		lq3=256;
		l = lq3-NumBits(2*m);
		//generates q as 2*2*m*q1*q2+1 and tests if q can be prime
		b=false;
		bol=true;
		while(b==false){
			b=new_q(q3,q1_t,q2_t,m,l);
		}
		b = false;
		while(b==false){
			b=check_q(a,q3,q1_t,q2_t,m,l);
		}
		genq3 = a;

		lq4=400;
		l = lq4-NumBits(2*m);
		//generates q as 2*2*m*q1*q2+1 and tests if q can be prime
		b=false;
		bol=true;
		while(b==false){
			b=new_q(q4,q1_t,q2_t,m,l);
		}
		b = false;
		while(b==false){
			b=check_q(a,q4,q1_t,q2_t,m,l);
		}
		genq = a;


		lp=2048;
		l= lp-lq-lq1-lq2-lq3-lq4;
		logl = 20*log(l);
		bo=false;

		F = q*q1*q2*q3*q4;

		//Generate p as 2*q*q1+1 and test if p is possible prime
		while(bo==false){
			bo=new_p(p,q1_t,F,l);
		}
		j=0;
		b=false;
		//If after log tries no p=2*q*q1+1 is prime a new q is picked
		while(j<logl && b==false){
			b=true;
			for(i = 0; i<100; i++){
				a = RandomBnd(p);
				an= PowerMod(a,p-1,p);
				if (a !=1 && an==1&& checkGCD(a, q,p) && checkGCD(a, to_ZZ(2),p)){
					if(checkGCD(a, q1,p) && checkGCD(a, q2,p) && checkGCD(a, q3,p) && checkGCD(a, q4,p)){

						break;
					}
				}
			}
			if(i==100){
				bo=false;
				while(bo==false){
					bo=new_p(p,q1_t,F,l);
				}
				b= false;
				j++;
			}
			else{//Test if a is primitive, following Mau94 p.143
					b= checkPow(a,q1_t,p);
			}
			if(j==logl){
				bol=false;
			}
		}
	}


	//Generator of G_q in Z_p
	gen = PowerMod(a,2*q1_t*q1*q2*q3*q4 ,p);
	gen1 = PowerMod(a,2*q1_t*q*q2*q3*q4 ,p);
	gen2 = PowerMod(a,2*q1_t*q*q1*q3*q4 ,p);
	gen3 = PowerMod(a,2*q1_t*q*q1*q2*q4 ,p);
	gen4 = PowerMod(a,2*q1_t*q*q1*q2*q3 ,p);

	ofstream ost;

	name = "100_2048";
	ost.open (name.c_str());
	ost<<p<<endl;
	ost<<q<<endl;
	ost<<gen<<endl;
	ost<<genq<<endl;
	ost.close();

	name = "200_2048";
	ost.open (name.c_str());
	ost<<p<<endl;
	ost<<q1<<endl;
	ost<<gen1<<endl;
	ost<<genq1<<endl;
	ost.close();

	name = "224_2048";
	ost.open (name.c_str());
	ost<<p<<endl;
	ost<<q2<<endl;
	ost<<gen2<<endl;
	ost<<genq2<<endl;
	ost.close();

	name = "256_2048";
	ost.open (name.c_str());
	ost<<p<<endl;
	ost<<q3<<endl;
	ost<<gen3<<endl;
	ost<<genq3<<endl;
	ost.close();

	name = "400_2048";
	ost.open (name.c_str());
	ost<<p<<endl;
	ost<<q4<<endl;
	ost<<gen4<<endl;
	ost<<genq4<<endl;
	ost.close();



}

//finds prime numbers q,p such that p = 2*a*q+1 using test provided by Mau94, lp,lq are the number of bits of q,p
void Functions::find_group(vector<ZZ>* pq, long lq, long lp, long m){
	long l,i,j,logl;
	ZZ mod30;
	ZZ q, q1,q2,p,a, an,gcd, gcd_1,gcd_2, temp, temp_1, gen, genq;
	bool b,bo, bol;
	int count=0;
	string name;
	//q-1 needs to be divisible by 2*m, such that we can find a 2m-root of unity
	cout<<lp<<" "<<lq<<endl;
	if((lp-lq)>2){
		bol=false;
		while(bol ==false){
			l = lq-NumBits(2*m);
			//generates q as 2*2*m*q1*q2+1 and tests if q can be prime
			b=false;
			bol=true;
			while(b==false){
				b=new_q(q,q1,q2,m,l);
			}
			b = false;
			while(b==false){
				b=check_q(a,q,q1,q2,m,l);
			}
			genq = a;

			l= lp-lq;
			bo=false;
			//Generate p as 2*q*q1+1 and test if p is possible prime
			while(bo==false){
				bo=new_p(p,q1,q,l);
				//bo=new_p(p,q1, q2,q,l);
			}
			logl = 20*log(l);
			j=0;
			b=false;
			//If after log tries no p=2*q*q1+1 is prime a new q is picked
			while(j<logl && b==false){
				b=true;

				if(q1*q2>q){
					b=check_p(a, p, q1,q,l,j);
					//b=check_p(a, p, q1, q2,q,l,j);
				}
				else{
					b=check_p(a, p, q,q1,l,j);
				}
			}
			if(j==logl){
				bol=false;
			}
		}

		//Generator of G_q in Z_p
		gen = PowerMod(a,2*q1 ,p);
	}
	else{//Sophie Germain prime p=2*q+1
		bol=false;
		count = 0;
		while(bol ==false){
			l = lq-NumBits(2*m);
			b=false;
			count ++;
			while(b==false){
				//Generate q as 2*2*m*q1*q2+1
				b=new_q(q,q1,q2,m,l);

				//q has to be 11,23,39 mod 30 to be part of a Sophie Germain prime
				mod30 = q % 30;
				if (mod30 ==to_ZZ(11)){
					b=true;
				}
				if (mod30 ==to_ZZ(23)){
					b=true;
				}
				if (mod30 ==to_ZZ(29)){
					b=true;
				}

			}
			b = false;
			while(b==false){
				b=check_q(a,q,q1,q2,m,l);
			}
			genq = a;

			p= 2*q+1;
			bol=probPrime(p);
			if(bol ==true){
				//Checks for random a, if a is a generator
				for(i = 0; i<100; i++){
					a = RandomBnd(p);
					an= PowerMod(a,p-1,p);
					if (a !=1 && an==1){
						temp = PowerMod(a,q,p);
						if(temp !=1){
							break;
						}
					}
				}
				if(i==100){
					bol = false;
				}
			}
		}

		//Generator of G_q in Z_p
		gen = PowerMod(a,2 ,p);
	}
	pq->at(0)=p;
	pq->at(1)=q;
	pq->at(2)=gen;
	//Generator of Z_q
	pq->at(3)=genq;
	cout<<NumBits(pq->at(1))<<" "<<NumBits(pq->at(0))<<endl;
	ofstream ost;

	name = tostring(lq)+"_"+tostring(lp);
	ost.open (name.c_str());
	ost<<p<<endl;
	ost<<q<<endl;
	ost<<gen<<endl;
	ost<<genq<<endl;
	ost.close();

}


//finds prime numbers q,p, p1 such that p = 2*a*q+1 and p1=2*b*q+1 using test provided by Mau94, lp,lq are the number of bits of q,p
void Functions::find_groups(vector<ZZ>* pq, long lq, long lp, long lp1, long m){
	ZZ q, q1, p1,a, gen;
	bool b, bo, bol;
	long logl, l,j;
	string name;


	bol = false;
	while(bol==false){
		bol=true;
		find_group(pq, lq, lp, m);

		q = pq->at(1);
		l= lp1-lq;
		//Generate p1 as 2*q*q1+1 and test if p1 is possible prime
		bo=false;
		while(bo==false){
			bo=new_p(p1,q1,q,l);
		}
		logl = log(l);
		j=0;
		b=false;
		//If after log tries no p=2*q*q1+1 is prime a new q and p is picked
		while(j<logl && b==false){
			b=true;

			if(q1>q){
				b=check_p(a, p1, q1,q,l,j);
			}
			else{
				b=check_p(a, p1, q,q1,l,j);
			}
		}
		if(j==logl){
			bol=false;
		}
	}

	//Generator of G_q in Z_p1
	gen = PowerMod(a,2*q1 ,p1);

	pq->at(4)=p1;
	pq->at(5)=gen;

	ofstream ost;
	cout<<NumBits(pq->at(1))<<" "<<NumBits(pq->at(0))<<" "<<NumBits(pq->at(4))<<endl;
	name = tostring(lq)+"_"+tostring(lp)+"_ "+tostring(lp1);
	ost.open (name.c_str());
	ost<<pq->at(0)<<endl;
	ost<<pq->at(1)<<endl;
	ost<<pq->at(2)<<endl;
	ost<<pq->at(3)<<endl;
	ost<<p1<<endl;
	ost<<gen;
	ost.close();


}

//Checks if a integer q is probably prime, calls the MillerRabin Test only with 1 witness
bool Functions::probPrime(ZZ q){
	bool b;

	b = false;
	if(q % 3 ==0){}
	else if(q % 5 ==0){}
	else if(q % 7 ==0) {}
	else if(q % 11 ==0) {}
	else if(q % 13 ==0) {}
	else if(q % 17 ==0) {}
	else if(q % 19 ==0){}
	else if(ProbPrime(q,1)==0){}
	else b=true;

	return b;
}

bool Functions::checkGCD(ZZ a, ZZ q1, ZZ q){
	bool b;
	ZZ temp, gcd;

	b=false;

	temp = PowerMod(a, (q-1)/q1,q);
	gcd = GCD(temp-1,q);
	if(gcd ==1){
		b=true;
	}
	return b;
}

bool Functions::checkPow(ZZ a, ZZ q1, ZZ q){
	bool b;
	ZZ temp;

	b=true;
	temp = PowerMod(a, (q-1)/q1,q);
	if(temp ==1){
		b=false;
	}
	return b;
}

long Functions::checkL1(ZZ &a, ZZ q, ZZ q1){
	long i;
	ZZ an;

	for(i = 0; i<100; i++){
		a = RandomBnd(q);
		an= PowerMod(a,q-1,q);
		if (a !=1 && an==1&& checkGCD(a, q1,q) && checkGCD(a, to_ZZ(2),q)){
			break;
		}
	}

	return i;

}

long Functions::checkL1(ZZ &a, ZZ q, ZZ q1, ZZ q2){
	long i;
	ZZ an;

	for(i = 0; i<100; i++){
		a = RandomBnd(q);
		an= PowerMod(a,q-1,q);
		if (a !=1 && an==1 && checkGCD(a, q1,q) && checkGCD(a, q2,q) && checkGCD(a, to_ZZ(2),q)){
			break;
		}
	}

	return i;

}


bool Functions::new_q(ZZ&q, ZZ &q1, ZZ & q2, long m, long l){
	bool b;

	//Generate q as 2*2*m*q1*q2+1
	q1 = GenPrime_ZZ(l/2+1);
	q2 = GenPrime_ZZ(l/2-1);
	q = 2*2*m*q1*q2+1;

	b=probPrime(q);
	return b;
}

bool Functions::check_q(ZZ &a, ZZ &q, ZZ &q1, ZZ&q2, long m , long l){
	bool b, bo;
	long i;

	b=true;
	//Test condition of Lemma 1 of Mau94 with F=2*m*q1 and R = q2, F>sqrt(q) with different random integers a
	i=checkL1(a,q,q1);
	//If no a satisfies condition of Lemma 1, new values for q1, q2 are picked
	if(i==100){
		bo =false;
		while(bo==false){
			bo=new_q(q,q1,q2,m,l);
		}
		b = false;
	}
	else{
		//checks if a is primitive (p.143 Mau94)
		b=checkPow(a,q2,q);
	}
	return b;
}

bool Functions::new_p(ZZ& p, ZZ & q1, ZZ q,long l){
	bool b;
	q1 = GenPrime_ZZ(l);
	p= 2*q*q1+1;
 //cout<<" in new p: ";
	b=probPrime(p);
	return b;
}

bool Functions::new_p(ZZ& p, ZZ & q1, ZZ& q2, ZZ q,long l){
	bool b;
	long len;
	len =0;
	while(len ==0 or len ==1 or len == l or len == l-1){
		len = RandomBnd(l);
	}
	//cout<<len<<" ";
	q1 = GenPrime_ZZ(len);
	q1 = GenPrime_ZZ(l-len);
	p= 2*q*q1*q2+1;
 //cout<<" in new p: ";
	b=probPrime(p);
	return b;
}


bool Functions::check_p(ZZ &a, ZZ &p, ZZ &q1, ZZ q, long l, long &j){
	bool b, bo;
	long i;

	b=true;
//cout<<" in check";
	i=checkL1(a,p,q1);
	//If no a satisfies the condition, pick new prime q1
	if(i==100){
		bo=false;
		while(bo==false){
			bo=new_p(p,q1,q,l);
		}
		b= false;
		j++;
	}
	else{//Test if a is primitive, following Mau94 p.143
		b= checkPow(a,q,p);
	}
	return b;

}


bool Functions::check_p(ZZ &a, ZZ &p, ZZ &q1, ZZ & q2, ZZ q, long l, long &j){
	bool b, bo;
	long i;

	b=true;

	i=checkL1(a,p,q1, q2);
	//If no a satisfies the condition, pick new prime q1 und q2
	if(i==100){
		bo=false;
		while(bo==false){
			bo=new_p(p,q1, q2, q,l);
		}
		b= false;
		j++;
	}
	else{//Test if a is primitive, following Mau94 p.143
		b= checkPow(a,q,p);
	}
	return b;

}





//help functions to delete matrices
void Functions::delete_vector(vector<vector<ZZ>* >* v){
	long i;
	long l = v->size();

	for(i=0; i<l; i++){
		delete v->at(i);
		v->at(i)=0;
	}
	delete v;
}


void Functions::delete_vector(vector<vector<long>* >* v){
	long i;
	long l = v->size();

	for(i=0; i<l; i++){
		delete v->at(i);
		v->at(i)=0;
	}
	delete v;
}
void Functions::delete_vector(vector<vector<Cipher_elg>* >* v){
	long i;
	long l = v->size();

	for(i=0; i<l; i++){
		delete v->at(i);
		v->at(i)=0;
	}
	delete v;
}


void Functions::delete_vector(vector<vector<vector<long>* >*>* v){
	long i;
	long l = v->size();

	for(i=0; i<l; i++){
		delete_vector(v->at(i));
	}
	delete v;
}

void Functions::delete_vector(vector<vector<vector<ZZ>* >*>* v){
	long i;
	long l = v->size();

	for(i=0; i<l; i++){
		delete_vector(v->at(i));
	}
	delete v;
}


// help functions, which pick random values and commit to a vector/matrix
//picks random value r and commits to the vector a,
void Functions::commit(vector<ZZ>* a, ZZ& r, Mod_p& com){
	ZZ ord = H.get_ord();

	r = RandomBnd(ord);
	com = Ped.commit(a,r);
	/*string name = "example.txt";
	ofstream ost;
	ost.open(name.c_str(),ios::app);
	ost<<r<<" "<<com<<endl;*/
}

//picks random values r and commits to the rows of the matrix a, a,r,com are variables of Prover
void Functions::commit(vector<vector<ZZ>*>* a_in, vector<ZZ>* r, vector<Mod_p>* com){
	long i,l;
	ZZ ord = H.get_ord();

	l=a_in->size();

//	string name = "example.txt";
/*	ofstream ost;
	ost.open(name.c_str(),ios::app);*/
	for(i=0; i<l; i++){
		r->at(i) = RandomBnd(ord);
		//		ost<<r->at(i)<<" ";
	}
	//	ost<<endl;
	for(i=0; i<l; i++){
		com->at(i) = Ped.commit(a_in->at(i),r->at(i));
		//	ost<<com->at(i)<<" ";
	}
	//	ost<<endl;
}

//picks random value r and commits to the vector a,
void Functions::commit_op(vector<ZZ>* a, ZZ& r, Mod_p& com){
	ZZ ord = H.get_ord();

	r = RandomBnd(ord);
	com = Ped.commit_opt(a,r);
}

//picks random values r and commits to the rows of the matrix a, a,r,com are variables of Prover
void Functions::commit_op(vector<vector<ZZ>*>* a_in, vector<ZZ>* r, vector<Mod_p>* com){
	long i,l;
	ZZ ord = H.get_ord();

	l=a_in->size();

	for(i=0; i<l; i++){
		r->at(i) = RandomBnd(ord);
	}
	for(i=0; i<l; i++){
		com->at(i) = Ped.commit_opt(a_in->at(i),r->at(i));
	}
}
