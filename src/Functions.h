/*
 * Functions.h
 *
 *  Created on: 26.10.2010
 *      Author: stephaniebayer
 *
 *      Functions used for the shuffle argument, which are used from main, Verifier and Prover
 *
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <vector>
#include <map>
#include "G_q.h"
#include "Mod_p.h"
#include "Cipher_elg.h"
#include "Permutation.h"


#include <NTL/ZZ.h>
#include <NTL/mat_ZZ.h>
NTL_CLIENT

class Functions {
public:
	Functions();
	virtual ~Functions();
	//read the config file and sets the parameters and the groups
	static void read_config(map<string, long> & num, ZZ & genq);

	//generates N=num[0] different ciphertexts
	static vector<vector<Cipher_elg>* >*  createCipher(
						map<string, long> num);
	static  void createCipher(vector<vector<Cipher_elg>* >* e, 
							map<string, long> num);
	//generates a matrix of random elements
	static vector<vector<ZZ>* >* randomEl(map<string, long> num);
	static void randomEl( vector<vector<ZZ>*>* R, map<string, long> num);
	//reencrypts the ciphertexts e using the permutation pi and random elements R
	static vector<vector<Cipher_elg>* >*  reencryptCipher(
				vector<vector<Cipher_elg>* >* e, 
				vector<vector<vector<long>* >* >* pi, 
				vector<vector<ZZ>*>* R, map<string, long> num);
	static  void reencryptCipher(vector<vector<Cipher_elg>* >* E, 
					vector<vector<Cipher_elg>* >* e, 
					vector<vector<vector<long>* >* >* pi, 
					vector<vector<ZZ>*>* R, 
					map<string, long> num);
	//returns the Hadamard product of x and y
	static void Hadamard( vector<ZZ>* ret, vector<ZZ>* x, vector<ZZ>* y);
	//Calculates the bilinear map Z^n x Z^ -> Z: x(y¡t)^T
	static ZZ bilinearMap(vector<ZZ>* x, vector<ZZ>* y, vector<ZZ>* t);

	static long tolong(string s);
	static string tostring(long s);

	//find order and modular value such that p = 2*a*q+1, p1=2*b*q+ 1 and it exists a 2m roof of unity
	static void find_stat_group();
	static void find_groups(vector<ZZ>* pq,long lq, long lp, long lp1, long m);
	static void find_group(vector<ZZ>* pq, long lq, long lp, long m);
	static void set_group();
	static bool probPrime(ZZ p);
	static bool checkGCD(ZZ a, ZZ q1, ZZ q);
	static bool checkPow(ZZ a, ZZ q1, ZZ q);
	static long checkL1(ZZ & a, ZZ q, ZZ q1);
	static long checkL1(ZZ & a, ZZ q, ZZ q1, ZZ q2);
	static bool new_q(ZZ&q, ZZ &q1, ZZ & q2, long m, long l);
	static bool check_q(ZZ &a, ZZ &q, ZZ &q1, ZZ&q2, long m , long l);
	static bool new_p(ZZ&p, ZZ &q1, ZZ q, long l);
	static bool new_p(ZZ&p, ZZ &q1, ZZ &q2, ZZ q, long l);
	static bool check_p(ZZ &a, ZZ &p, ZZ &q1, ZZ q, long l, long &j);
	static bool check_p(ZZ &a, ZZ &p, ZZ &q1, ZZ& q2, ZZ q, long l, long &j);

	//help functions to delete matrices
	static void delete_vector(vector<vector<ZZ>* >* v);
	static void delete_vector(vector<vector<long>* >* v);
	static void delete_vector(vector<vector<Cipher_elg>* >* v);
	static void delete_vector(vector<vector<vector<long>* >*>* v);
	static void delete_vector(vector<vector<vector<ZZ>* >*>* v);

	// help functions, which pick random values and commit to a vector/matrix
	static void commit( vector<ZZ>* a, ZZ& r, Mod_p& com);
	static void commit(vector<vector<ZZ>* >* a, vector<ZZ>* r, vector<Mod_p>* com);
	static void commit_op( vector<ZZ>* a, ZZ& r, Mod_p& com);
	static void commit_op(vector<vector<ZZ>* >* a, vector<ZZ>* r, vector<Mod_p>* com);
};

#endif /* FUNCTIONS_H_ */
