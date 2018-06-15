#include "NIZKProof.h"

NIZKProof::NIZKProof(string ser) {
	proof_ << ser;
}

void NIZKProof::set_proof(string proof) {
	proof_ << proof;
}

void NIZKProof::add_new_step(string& input_to_ver, ZZ& challenge, ZZ& rand) {
	write_str(input_to_ver);
	write_ZZ(challenge);
	write_ZZ(rand);
}

void NIZKProof::add_final_step(string& input_to_ver) {
	write_str(input_to_ver);
}

void NIZKProof::read_final_step(string& input_to_ver) {
	input_to_ver = read_str();
}


void NIZKProof::read_next(string& input_to_ver, ZZ& challenge, ZZ& rand) {
	input_to_ver = read_str();
	challenge = read_ZZ();
	rand = read_ZZ();
}

void NIZKProof::write_str(string& input_to_ver) {
	int len = input_to_ver.size();
	proof_ << to_string(len) << "\n";
	proof_ << input_to_ver;	
}

string NIZKProof::read_str() {
	string slen;
	getline(proof_, slen);
	int len = stoi(slen);
	char* input = new char[len];
	proof_.read(input, len);
	string input_to_ver = string(input, len) + "\0";
	delete[] input;
	return input_to_ver;
}

void NIZKProof::write_ZZ(ZZ& n) {
	proof_ << n << "\n";
}

ZZ NIZKProof::read_ZZ() {
	string x;
	getline(proof_, x);
	stringstream xsream(x);
	ZZ ret;
	xsream >> ret;
	return ret;
}

