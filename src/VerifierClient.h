#ifndef __VERIFIER_CLIENT_H__
#define __VERIFIER_CLIENT_H__

#include <stdio.h>
#include <time.h>
#include <vector>
#include <fstream>

#include "G_q.h"
#include "Functions.h"
#include "ElGammal.h"
#include "Cipher_elg.h"
#include "Permutation.h"
#include "Verifier_toom.h"
#include "FakeZZ.h"
#include "NIZKProof.h"
NTL_CLIENT

class VerifierClient {
  public:
	VerifierClient(const vector<long>& config, int m, int n, vector<vector<Cipher_elg>* >* ciphers, vector<vector<Cipher_elg>* >* permuted_ciphers, ElGammal* elgammal, bool owner, bool do_process);
	~VerifierClient();

	void set_public_vector(istringstream& f, long n, int o1, int o2, int o3);
	bool process_nizk(string nizk);
	string get_proof();


	void print_state() const;
	double get_time() const { return time_v; }

	string round2(const string& input_file, ZZ* challenge, ZZ* rand);
	string round4(const string& input_file, ZZ* challenge, ZZ* rand);
	string round6(const string& input_file, ZZ* challenge, ZZ* rand);
	string round6red(const string& input_file, ZZ* challenge, ZZ* rand);
	string round6red_1(const string& input_file, ZZ* challenge, ZZ* rand);
	string round8(const string& input_file, ZZ* challenge, ZZ* rand);
	bool round10(const string& input_file);
  private:
	
	string round2(const string& input_file, ZZ& challenge, ZZ& rand);
	string round4(const string& input_file, ZZ& challenge, ZZ& rand);
	string round6(const string& input_file, ZZ& challenge, ZZ& rand);
	string round6red(const string& input_file, ZZ& challenge, ZZ& rand);
	string round6red_1(const string& input_file, ZZ& challenge, ZZ& rand);
	string round8(const string& input_file, ZZ& challenge, ZZ& rand);
	bool round10red(const string& input_file);

	vector<long> config_;
	long m_r; //number of rows after reduction
	vector<vector<Cipher_elg>* >* c; // contains the original input ciphertexts
	vector<vector<Cipher_elg>* >* C; //Contains reencryptetd ciphers
	double time_v;

	NIZKProof proof;
	Verifier_toom* V;
	bool owner_;
	bool do_process_;

  public:
	bool flow_flag_;
};

#endif
