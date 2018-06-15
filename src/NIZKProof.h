#ifndef __NIZK_PROOF_H__
#define __NIZK_PROOF_H__

#include "FakeZZ.h"
#include <string>
#include <sstream>

using namespace std;
NTL_CLIENT

class NIZKProof {
public:
	NIZKProof() {}
	NIZKProof(NIZKProof& other) : proof_(other.proof()) {}
	NIZKProof(string ser);
	
	void set_proof(string proof);
	void add_new_step(string& input_to_ver, ZZ& challenge, ZZ& rand);
	void add_final_step(string& input_to_ver);
	void read_next(string& input_to_ver, ZZ& challenge, ZZ& rand);
	void read_final_step(string& input_to_ver);
	string proof() { return proof_.str(); }
private:
	void write_str(string& s);
	string read_str();
	void write_ZZ(ZZ& n);
	ZZ read_ZZ();
	stringstream proof_;
};

#endif
