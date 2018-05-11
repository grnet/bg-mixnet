#ifndef __CIPHER_TABLE_H__
#define __CIPHER_TABLE_H__

#include <vector>
#include <string>
#include <sstream>

#include "G_q.h"
#include "Mod_p.h"
#include "Cipher_elg.h"
#include "ElGammal.h"
#include "Pedersen.h"

using namespace std;

class CipherTable {
  public:
	CipherTable();
	CipherTable(string& ciphers, long m, ElGammal* elgammal);
	CipherTable(vector<vector<Cipher_elg>* >* ciphers, long m);
	virtual ~CipherTable();
	vector<vector<Cipher_elg>* >* getCMatrix();
	vector<vector<Mod_p>* >* getElementsMatrix();
	string getCipher(int i, int j);
	string getElement(int i, int j);
	Cipher_elg& get_elg_cipher(int i, int j);
	
	int rows();
	int cols();
	void set_dimensions(int m, int n);
	
	string encode_all_ciphers();
  private:
	vector<vector<Cipher_elg>* > C_;
	vector<vector<Mod_p>* > elements_;
	int m_;
	int n_;
	const bool owner_;
};

#endif
