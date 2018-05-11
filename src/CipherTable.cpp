#include "CipherTable.h"

#include "Functions.h"

CipherTable::CipherTable() : m_(0), n_(0), owner_(true) {}

CipherTable::CipherTable(string& ciphers, long m, ElGammal* elgammal) : m_(m), owner_(true) {
	Functions::parse_ciphers(ciphers, m, C_, elgammal);

	if (m == 0) {
		n_ = 0;
	} else {
		n_ = C_.at(0)->size();
	}	
}

CipherTable::CipherTable(vector<vector<Cipher_elg>* >* ciphers, long m): m_(m), owner_(false) {
	C_ = *ciphers;
	if (m == 0) {
		n_ = 0;
	} else {
		n_ = C_.at(0)->size();
	}
}

CipherTable::~CipherTable() {
	if (owner_) {
		for (unsigned int i = 0; i < C_.size(); i++) {
			delete C_.at(i);
		}
	
		for (unsigned int i = 0; i < elements_.size(); i++) {
			delete elements_.at(i);
		}
	}
}


vector<vector<Cipher_elg>* >* CipherTable::getCMatrix() {
	return &C_;
}

vector<vector<Mod_p>* >* CipherTable::getElementsMatrix() {
	return &elements_;
}

string CipherTable::getCipher(int i, int j) {
	stringstream cipher_str;
	cipher_str << C_.at(i)->at(j);
	return cipher_str.str();
}

Cipher_elg& CipherTable::get_elg_cipher(int i, int j) {
	return C_.at(i)->at(j);
}

string CipherTable::getElement(int i, int j) {
	stringstream cipher_str;
	cipher_str << elements_.at(i)->at(j);
	return cipher_str.str();
}

string CipherTable::encode_all_ciphers() {
	return Functions::ciphers_to_str(&C_);
}

int CipherTable::rows() {
	return m_;
}

int CipherTable::cols() {
	return n_;
}

void CipherTable::set_dimensions(int m, int n) {
	m_ = m;
	n_ = n;
}
