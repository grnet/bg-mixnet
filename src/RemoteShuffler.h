#ifndef __REMOTE_SHUFFLER_H__
#define __REMOTE_SHUFFLER_H__

#define BOOST_SPIRIT_THREADSAFE

#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>

#include "G_q.h"
#include "Functions.h"
#include "ElGammal.h"
#include "Cipher_elg.h"
#include "Permutation.h"
#include "Prover_toom.h"
#include "VerifierClient.h"

#include "FakeZZ.h"
#include <chrono>
NTL_CLIENT

using namespace std::chrono;

class RemoteShuffler {
  public:
	RemoteShuffler(const vector<long>& config, vector<vector<Cipher_elg>* >* ciphers_in, ElGammal* reenc_key, int m, int n, bool owner = true);
	~RemoteShuffler();
	
	string create_nizk();
	
	vector<vector<Cipher_elg>* >* permute_and_reencrypt(ElGammal* reenc_key);
	vector<vector<Cipher_elg>* >* getc() { return c; }
	vector<vector<Cipher_elg>* >* getC() { return C; }
	vector<vector<vector<long>* >* > * getPermutation() { return pi; }
	void print_state() const;
	void reverse_permutation(vector<long>& reversed);
	void output_permutation(ostream& f);
	double get_time() const { return time_p; }
	string get_public_vector();

  private:
	string round1(string* input_for_next, ZZ* challenge, ZZ* randomness);
	string round3(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness);
	string round5(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness);
	string round5red(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness);
	string round5red_1(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness);
	string round7(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness);
	string round7red(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness);
	string round9(const string& input_file);

	vector<long> config_; //Containing the number of ciphertexts and the structure of the matrix of the ciphertexts
	double time_rw_p;
	double time_rw_v;
	double time_cm;

	int i;
	vector<vector<Cipher_elg>* >* c; // contains the original input ciphertexts
	vector<vector<Cipher_elg>* >* C;//Contains reencryptetd ciphers
	vector<vector<vector<long>* >* > * pi; //Permutation
	vector<vector<ZZ>* > * R; //Random elements for reencryption
        ElGammal *key;
	long m, n;
	long m_r_;
	double time_p;

	// prover vars
	Prover_toom* P;
	ZZ chal_10,ans_12;
	string name;
	
	VerifierClient* verifier_;
	bool owner_;

  public:
	bool flow_flag_;
};

#endif
