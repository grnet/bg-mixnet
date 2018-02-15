#ifndef __GLOBALS_H__
#define __GLOBALS_H__


#include <thread>

#include "ElGammal.h"
#include "FakeZZ.h"

NTL_CLIENT


G_q G=G_q();// group used for the Pedersen commitment
G_q H=G_q();// group used for the the encryption
//ElGammal El = ElGammal(); //The class for encryption and decryption
//Pedersen Ped = Pedersen(); //Object which calculates the commitments
string kConfigFile("config/config");
ZZ genq; //generator of Z_q
long m;

//OpenMP parallelization configuration
bool parallel = 1;
int num_threads = std::thread::hardware_concurrency();

long m_r=0;//number of rows after reduction
long mu=0; //number of rows after reduction
long mu_h=0;//2*mu-1, number of extra elements in the reduction

#define kGroup (1248)

#endif
