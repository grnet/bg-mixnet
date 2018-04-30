/*
 * Functions.h
 *
 *  Created on: 26.10.2010
 *      Author: stephaniebayer
 *
 *      Functions used for the shuffle argument, which are used from main, Verifier and Prover
 *
 */

#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <vector>
#include <map>
#include "G_q.h"
#include "Mod_p.h"
#include "Cipher_elg.h"
#include "ElGammal.h"
#include "Pedersen.h"
#include "FakeZZ.h"
NTL_CLIENT

class Functions {
public:

	Functions();
	virtual ~Functions();
	//read the config file and sets the parameters and the groups
	static void read_config(const string& config_file, vector<long> & num, ZZ & genq);
	static void sha256(string input, unsigned char* out_buf);

	static ElGammal* set_crypto_ciphers_from_json(const char *ciphers_file,
						 vector<vector<Cipher_elg>* >& C,
						 const long m, const long n);

	static void print_crypto(const map<string, string>& crypto);

	static void print_cipher_matrix(const vector<vector<Cipher_elg>* >& C,
					const long m, const long n);

	static void parse_ciphers(string& f, long m, vector<vector<Cipher_elg>* >& C, ElGammal* elgammal);
	static string ciphers_to_str(vector<vector<Cipher_elg>* >* ciphers);
	static string parse_response(std::basic_streambuf<char>* in);

	//generates N=num[0] different ciphertexts
	static void createCipher(vector<vector<ZZ> >* secrets, int m, int n, int N, vector<vector<Cipher_elg>* >* C, vector<vector<Mod_p>* >* elements, ElGammal* enc_key);
        static void createCipherWithProof(vector<vector<ZZ> >* secrets, int m, int n, int N, vector<vector<Cipher_elg>* >* C, vector<vector<Mod_p>* >* elements, char* proofs, ElGammal* enc_key);
	//generates a matrix of random elements
	static void randomEl( vector<vector<ZZ>*>* R, int m, int n);
	//reencrypts the ciphertexts e using the permutation pi and random elements R
	static void reencryptCipher( vector<vector<Cipher_elg>* >* E, vector<vector<Cipher_elg>* >* e, vector<vector<vector<long>* >* >* pi, vector<vector<ZZ>*>* R, int m, int n, ElGammal* reenc_pub_key);

	static vector<long> permutation2d_to_vector(vector<vector<vector<long>* >* >* pi, long m, long n);
	static int get_num_cols(int m, int num_elements);
	//returns the Hadamard product of x and y
	static void Hadamard( vector<ZZ>* ret, vector<ZZ>* x, vector<ZZ>* y);
	//Calculates the bilinear map Z^n x Z^ -> Z: x(y¡t)^T
	static ZZ bilinearMap(vector<ZZ>* x, vector<ZZ>* y, vector<ZZ>* t);

	static long tolong(string s);
	static string tostring(long s);
	static void write_to_file(const string& filename, double output);

	//help functions to delete matrices
	static void delete_vector(vector<vector<ZZ>* >* v);
	static void delete_vector(vector<vector<long>* >* v);
	static void delete_vector(vector<vector<Cipher_elg>* >* v);
	static void delete_vector(vector<vector<vector<long>* >*>* v);
	static void delete_vector(vector<vector<vector<ZZ>* >*>* v);

	// help functions, which pick random values and commit to a vector/matrix
	static void commit_op( vector<ZZ>* a, ZZ& r, Mod_p& com, Pedersen& ped);
	static void commit_op(vector<vector<ZZ>* >* a, vector<ZZ>* r, vector<Mod_p>* com, Pedersen& ped);
};

#endif /* FUNCTIONS_H_ */
