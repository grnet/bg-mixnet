#include "Bgmix.h"

#include "CipherTable.h"
#include "Globals.h"
#include "Functions.h"
#include <string.h>
#include "RemoteShuffler.h"
#include "FakeZZ.h"
#include "SchnorrProof.h"

#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <map>
#include <stdio.h>
#include <mutex>
using namespace std;

mutex gInitMutex;

extern G_q H;
extern G_q G;

static bool kIsInit = false;

static vector<long> num(8);

void init_private_key(ElGammal* elgammal, int key_id) {
	ZZ secret;
        string private_key_file = string("config/keys/priv") + to_string(key_id);
#if USE_REAL_POINTS
        ZZFromBytes(secret, skeys[key_id], 32);
#else
	string line;
	ifstream sist;
	sist.open(private_key_file);
	getline(sist, line);

	getline(sist, line);
	istringstream secretstr(line);
	secretstr >> secret;
#endif
	elgammal->set_group(G);
	elgammal->set_sk(secret);
}

void* create_pub_key(int key_id) {
	Mod_p pk;
#if USE_REAL_POINTS
        // private key is
        // {0x50, 0x44, 0x4f, 0x53, 0x0, ...}
        CurvePoint pk_ = raw_curve_pt(pkeys[key_id]);
        // TODO this does not handle garbage collection
        pk = Mod_p(pk_, G.get_mod());
#else
	string fname = string("config/keys/pub") + to_string(key_id);
	ifstream ist;
	ist.open(fname);
	if (ist.fail()) {
		cout << "cannot open key file " << fname <<endl;
		return false; // TODO should probably raise an exception
	} 
	string line;
	getline(ist, line);
	istringstream pkstr(line);
	pkstr >> pk;
	ist.close();
#endif

	ElGammal* ret = new ElGammal();
	ret->set_group(G);
	ret->set_pk(pk);
	return ret;
}

void* create_decryption_key(int key_id) {
	ElGammal* elgammal = new ElGammal();
	init_private_key(elgammal, key_id);
	return (void*)elgammal;
}

void delete_key(void* elgammal) {
	ElGammal* tmp = (ElGammal*) elgammal;
	delete tmp;
}


void init() {
	lock_guard<mutex> guard(gInitMutex);

	if (kIsInit) return;
	Functions::read_config(kConfigFile, num, genq);

#if USE_REAL_POINTS
        CurvePoint gen = curve_basepoint();
        ZZ ord = ZZ(NTL::conv<NTL::ZZ>("7237005577332262213973186563042994240857116359379907606001950938285454250989"));
        // ZZ mod = ZZ(NTL::conv<NTL::ZZ>("2093940378184301311653365957372856779274958817946641127345598909177821235333110899157852449358735758089191470831461169154289110965924549400975552759536367817772197222736877807377880197200409316970791234520514702977005806082978079032920444679504632247059010175405894645810064101337094360118559702814823284408560044493630320638017495213077621340331881796467607713650957219938583"));
        ZZ mod = ZZ(NTL::conv<NTL::ZZ>("42"));
#else
        NTL::ZZ gen_sc = NTL::conv<NTL::ZZ>("1929181099559129674691211513194785872536670409492790905276619913671396722443243145931673445424440902236760877484211441680348197072495215150053603001343967365713940597148603897520835948403066356627154482171157913975934174689003578096019980791028264452409955094293631742810957258379488668086855090084223965396993821991583550151470397960480522495500106360092070361350077271147228");
        CurvePoint gen = zz_to_curve_pt(gen_sc);
        ZZ ord = ZZ(NTL::conv<NTL::ZZ>("1257206741114416297422800737364843764556936223541"));
        ZZ mod = ZZ(NTL::conv<NTL::ZZ>("2093940378184301311653365957372856779274958817946641127345598909177821235333110899157852449358735758089191470831461169154289110965924549400975552759536367817772197222736877807377880197200409316970791234520514702977005806082978079032920444679504632247059010175405894645810064101337094360118559702814823284408560044493630320638017495213077621340331881796467607713650957219938583"));
#endif

        G = G_q(gen, ord, mod);
        H = G_q(gen, ord, mod);
	
	m = num[1];
	kIsInit = true;
}

string key_index_to_fname(int key_index) {
	return string("config/keys/pub") + to_string(key_index);
}


vector<vector<ZZ> >* buildSecretsVector(const unsigned char** secrects, int secretLen, int array_len) {
	vector<vector<ZZ> >* ret = new vector<vector<ZZ> >(m);
	vector<ZZ> r;

	int count = 0;
	int num_cols = Functions::get_num_cols(m, array_len);
	
	for (int i=0; i<m; i++){
		r = vector<ZZ>(num_cols);
		for (int j = 0; j <num_cols; j++){
			if (count < array_len){
				r.at(j)=ZZFromBytes(secrects[count], secretLen) % H.get_ord();
			}
			else
			{
				r.at(j)=RandomBnd(H.get_ord());
			}
			count++;
		}
		ret->at(i)=r;
	}
	return ret;
}

struct ciphertexts_and_proofs {
  CipherTable *ciphertexts;
  char *proofs;
  int proofs_size;
};

void* encrypt_with_proof(void** in_secrets, int secretLen, int arrayLen, int keyIndex) {
	init();
	const unsigned char** secrects = (const unsigned char**) in_secrets;
	ElGammal* elgammal = (ElGammal*)create_pub_key(keyIndex);
	int num_cols = Functions::get_num_cols(m, arrayLen);
	CipherTable* ret = new CipherTable();
	vector<vector<ZZ> >* my_secrets = buildSecretsVector(secrects, secretLen, arrayLen);

        struct ciphertexts_and_proofs *s = (struct ciphertexts_and_proofs *) malloc(sizeof(struct ciphertexts_and_proofs));
        s->proofs_size = SchnorrProof::bytesize * m * num_cols;
        s->proofs = new char[s->proofs_size];
	Functions::createCipherWithProof(my_secrets, m, num_cols, arrayLen, ret->getCMatrix(), ret->getElementsMatrix(), s->proofs, elgammal);

	ret->set_dimentions(m, num_cols);
	delete my_secrets;
	delete_key(elgammal);
        s->ciphertexts = ret;
	return s;
}

void* encrypt_cipher_part(void* cipher_and_proof) {
  struct ciphertexts_and_proofs *s = (struct ciphertexts_and_proofs *) cipher_and_proof;
  return s->ciphertexts;
}

void* encrypt_proof_part(void* cipher_and_proof, int* proof_size) {
  struct ciphertexts_and_proofs *s = (struct ciphertexts_and_proofs *) cipher_and_proof;
  *proof_size = s->proofs_size;
  return s->proofs;
}

void delete_ciphers_with_proof(void* x) {
  struct ciphertexts_and_proofs *s = (struct ciphertexts_and_proofs *) x;
  delete_ciphers(s->ciphertexts);
  delete[] s->proofs;
  free(s);
}

int verify_encrypt(void* ciphertexts, int ciphertexts_size, void* pfs, int proofs_size) {
  init();
  int num_elems = proofs_size / SchnorrProof::bytesize;
  int num_cols = num_elems / m;
  CipherTable *ct = (CipherTable*) parse_ciphers(ciphertexts, ciphertexts_size, 0); // TODO check this doesn't segfault
  char *proofs = (char *) pfs;

  volatile int verified = 1;
  #pragma omp parallel for collapse(2) num_threads(num_threads) if(parallel)
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < num_cols; j++) {
      char *proof = &proofs[(i*num_cols + j) * SchnorrProof::bytesize];
      SchnorrProof pf = SchnorrProof(proof);
      Cipher_elg c = ct->get_elg_cipher(i, j);
      CurvePoint x = c.get_u();

      if (pf.verify(x) == 0) {
        verified = 0;
      }
    }
  }
  delete ct;

  return verified;
}

void* elg_encrypt(void** in_secrets, int secretLen, int arrayLen, int keyIndex) {
	init();
	const unsigned char** secrects = (const unsigned char**) in_secrets; 
	ElGammal* elgammal = (ElGammal*)create_pub_key(keyIndex);
	int num_cols = Functions::get_num_cols(m, arrayLen);
	CipherTable* ret = new CipherTable();
	vector<vector<ZZ> >* my_secrets = buildSecretsVector(secrects, secretLen, arrayLen);
	Functions::createCipher(my_secrets, m, num_cols, arrayLen, ret->getCMatrix(), ret->getElementsMatrix(), elgammal);
	ret->set_dimentions(m, num_cols);
	delete my_secrets;
	delete_key(elgammal);
	return ret;
}

void* get_ciphertexts(void* in_table, void* in_len, void* in_elmenent_size) {
	int* elmenent_size = (int*) in_elmenent_size;
	int* len = (int*) in_len;
	CipherTable* cipher_table = (CipherTable*)in_table;
	string encoded(cipher_table->encode_all_ciphers());
	*len = encoded.size();
	*elmenent_size = (*len) / (cipher_table->rows() * cipher_table->cols());
	char* out = new char[*len];
	memcpy(out, encoded.c_str(), *len);
	return (void*) out;
}

void* get_element(void* in_table, int index, void* in_len) {
	int* len = (int*) in_len;
	CipherTable* cipher_table = (CipherTable*)in_table;
	int i = index / cipher_table->cols();
	int j = index % cipher_table->cols();

        // use canonical serialization
#if USE_REAL_POINTS
        CurvePoint pt = cipher_table->getElementsMatrix()->at(i)->at(j).get_val();
        *len = 32; // TODO is this correct?
        char* out = new char[*len];
        pt.serialize_canonical(out);
#else
	string encoded(cipher_table->getElement(i, j));
	*len = encoded.size();
	char* out = new char[*len];
	memcpy(out, encoded.c_str(), *len);
#endif
	return (void*)out;
}

void delete_ciphers(void* in_table) {
	CipherTable* cipher_table = (CipherTable*)in_table;
	delete cipher_table;
}

int rows(void* cipher_table) {
	return ((CipherTable*) cipher_table)->rows();
}

int cols(void* cipher_table) {
	return ((CipherTable*) cipher_table)->cols();
}

void* get_cipher(void* cipher_table, int i, int j, void* in_len) {
	int* len = (int*) in_len;
	CipherTable* ct = (CipherTable*) cipher_table;
	string cipher (ct->getCipher(i, j));
	*len = cipher.size();
	char* out = new char[*len];
	memcpy(out, cipher.c_str(), *len);
	return (void*) out;
}


void* parse_ciphers(void* in_ciphers, int len, void* elgammal) {
	unsigned char* ciphers = (unsigned char*) in_ciphers;
	long m = num[1];

	string c((char*)ciphers, len);
	return new CipherTable(c, m, (ElGammal*)elgammal);
}

void* decrypt_cipher(void* in_table, int i, int j, void* in_len, void* elgammal_in) {
	init();
	
	ElGammal* elgammal = (ElGammal*)elgammal_in;
	int* len = (int*) in_len;
	CipherTable* ciphers = (CipherTable*)in_table;
	Mod_p plain = elgammal->decrypt(ciphers->get_elg_cipher(i, j));

        // use canonical serialization
#if USE_REAL_POINTS
        CurvePoint pt = plain.get_val();
        *len = 32;
        char* out = new char[*len];
        pt.serialize_canonical(out);
#else
	stringstream elm_str;
	elm_str << plain;
	string encoded = elm_str.str();
	*len = encoded.size();
	char* out = new char[*len];
	memcpy(out, encoded.c_str(), *len);
#endif
	return (void*) out;
}

void delete_str(void* s) {
	delete [] (char*)s;
}


char**makeCharArray(int size) {
	return new char* [size];
}

void setArrayString(char **a, char *s, int index, int src_index, int size) {
	a[index] = new char [size];
	memcpy(a[index], s + src_index, size);
}

void freeCharArray(char **a, int size) {
	int i;
	for (i = 0; i < size; i++) {
		delete [] a[i];
	}
	delete [] a;
}

#ifdef LOG_CRYPTO_OUTPUT
void redirect_streams_to_log(ofstream &log, streambuf **saved_cout, streambuf **saved_cerr) {
	*saved_cout = cout.rdbuf();	// save cout buffer
	*saved_cerr = cerr.rdbuf();	// save cout buffer
	cout.rdbuf(log.rdbuf());		// redirect cout buffer to log file
	cerr.rdbuf(log.rdbuf());		// redirect cout buffer to log file
	cout << "Log messages in file " << LOG_CRYPTO_OUTPUT << endl;
}

void redirect_log_to_streams(streambuf *saved_cout, streambuf *saved_cerr, ofstream &log) {
	cout.rdbuf(saved_cout);
	cerr.rdbuf(saved_cerr);
	log.close();
}
#endif

void usage(long m) {
	cout << "Invalid number of rows (m): " << m <<endl;
	cout << "Requirement: 4^x = m for integer x > 2" <<endl;
	exit(1);
}

void check_usage(long m) {
	if (m < 64)
		usage(m);
	// Check that m satisfies 4^x = m
	int i = 3;
	long pow_m = 64;
        while (pow_m < m)
		pow_m = pow(4, i++);
	if (pow_m > m)
		usage(m);
#if USE_REAL_POINTS
        cout << "Cryptosystem: ElGamal on elliptic curve (Curve25519) points" << endl;
#else
        cout << "Cryptosystem: ElGamal on big integers" << endl;
#endif
}

bool generate_ciphers(const char* ciphers_file, const long dim_m, const long dim_n) {
#ifdef LOG_CRYPTO_OUTPUT
        // log file specified in config
	ofstream log(LOG_CRYPTO_OUTPUT, ofstream::out | ofstream::app);
	streambuf *saved_cout, *saved_cerr;
	redirect_streams_to_log(log, &saved_cout, &saved_cerr);
#endif
	init();
	// Override config file's cipher matrix dimensions
	num[1] = dim_m;
	num[2] = dim_n;

	long m = num[1];
	long n = num[2];
	cout << "m: " << m << ", n: " << n << endl;

	check_usage(m);

	const int SECRET_SIZE = 5;
	unsigned char** secrets = new unsigned char* [m * n];

	srand ((unsigned int) time (NULL));

	for (int i = 0; i < m * n; i++) {
		secrets[i] = new unsigned char[SECRET_SIZE];
		for (int j = 0; j < SECRET_SIZE; j++) {
			secrets[i][j] = (char)rand();
		}
	}

	ElGammal* elgammal = (ElGammal*)create_pub_key(1);
	G_q group = elgammal->get_group();

	time_t begin = time(NULL);
	CipherTable* ciphers = (CipherTable*) elg_encrypt((void**) secrets, SECRET_SIZE, m * n, 1);
	time_t enc_time = time(NULL);
	cout << "encryption time: " << enc_time - begin << endl; 

	ofstream ofciphers;
	ofciphers.open(ciphers_file);
	if (ofciphers.fail()) {
		cout << "cannot open ciphers file " << ciphers_file <<endl;
		return false; // TODO should probably raise an exception
	}
	ofciphers << "{";
	ofciphers << "\"generator\": " << group.get_gen();
	ofciphers << ", ";
	ofciphers << "\"modulus\": " << group.get_mod();
	ofciphers << ", ";
	ofciphers << "\"order\": " << group.get_ord();
	ofciphers << ", ";
	ofciphers << "\"public\": " << elgammal->get_pk();
	ofciphers << ", ";
	ofciphers << "\"original_ciphers\": [";
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++) {
			ofciphers << ciphers->getCipher(i, j);
			if (!(i == m-1 && j == n-1))
				ofciphers << ", ";
			//cout << "cipher " << i << " " << j << " : " << ciphers->getCipher(i, j) << endl;
		}
        ofciphers << "],";
        ofciphers << "\"mixed_ciphers\": [";
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++) {
			ofciphers << ciphers->getCipher(i, j);
			if (!(i == m-1 && j == n-1))
				ofciphers << ", ";
			//cout << "cipher " << i << " " << j << " : " << ciphers->getCipher(i, j) << endl;
		}
        ofciphers << "]";
        ofciphers << "}";
	ofciphers.close();

	for (int i = 0; i < m * n; i++) {
		delete [] secrets[i];
	}
	delete [] secrets;
	delete ciphers;

#ifdef LOG_CRYPTO_OUTPUT
	redirect_log_to_streams(saved_cout, saved_cerr, log);
#endif
	return true;
}

bool mix(const char* ciphers_file, const long dim_m, const long dim_n) {
#ifdef LOG_CRYPTO_OUTPUT
        // log file specified in config
	ofstream log(LOG_CRYPTO_OUTPUT, ofstream::out | ofstream::app);
	streambuf *saved_cout, *saved_cerr;
        redirect_streams_to_log(log, &saved_cout, &saved_cerr);
#endif
	init();
	// Override config file's cipher matrix dimensions
	num[1] = dim_m;
	num[2] = dim_n;

	long m = num[1];
	long n = num[2];

	check_usage(m);

	CipherTable* ciphers = new CipherTable();
	vector<vector<Cipher_elg>* >* cm = ciphers->getCMatrix();

	ElGammal *elgammal = Functions::set_crypto_ciphers_from_json(ciphers_file,
									*cm, m, n);
	cout << "shuffling " << n * m << " messages" <<endl;

	time_t parse_start = time(NULL);
	cout << "parsing input" <<endl;
	string shuffle_input(ciphers->encode_all_ciphers());
	cout << "done parsing. " << time(NULL) - parse_start << endl;

	char* shuffled_ciphers;
	int shuffled_ciphers_len;
	char* proof;
	int proof_len;
	int* permutation;
	int permutation_len;
	char* public_randoms;
	int public_randoms_len;
	
	cout << "shuffle begins!" <<endl;
	char* input = (char*)shuffle_input.c_str();
	time_t shuffle_time = time(NULL);
	void *cached_shuffle = shuffle_internal(elgammal, input, shuffle_input.size(), m*n, &shuffled_ciphers, &shuffled_ciphers_len, &permutation, &permutation_len);
	cout << "shuffle is done! In " << time(NULL) - shuffle_time << endl;
        time_t prove_time = time(NULL);
	prove(cached_shuffle, &proof, &proof_len, &public_randoms, &public_randoms_len);
	cout << "proof is done! In " << time(NULL) - prove_time << endl;

	time_t verify_time = time(NULL);
	int ret = verify(elgammal, proof, proof_len, input, shuffle_input.size(), shuffled_ciphers, shuffled_ciphers_len, public_randoms, public_randoms_len);
	cout << "verification is done! In " << time(NULL) - verify_time << endl;
	cout << "Shuffle + prove + verify = " << time(NULL) - shuffle_time << endl;
	delete ciphers;


	delete [] shuffled_ciphers;
	delete [] proof;
	delete [] permutation;

	if (ret) {
		cout << "everything passed!" <<endl;
	} else {
		cout << "shuffle failed!" <<endl;
	}

#ifdef LOG_CRYPTO_OUTPUT
	redirect_log_to_streams(saved_cout, saved_cerr, log);
#endif

	return ret;
}

void hello() {
	printf("hello world!\n");
}

// returns a pointer caching shuffle data
void *shuffle_internal(void* reenc_key, char* ciphers_in, int ciphers_array_len, int number_of_elements, char** shuffled_ciphers, int* shuffled_ciphers_len, int** permutation, int* permutation_len) {
	init();
	
	int number_of_cols = Functions::get_num_cols(m, number_of_elements);
	string inp(ciphers_in, ciphers_array_len);
	
	CipherTable input(inp, m, (ElGammal*)reenc_key);
	vector<vector<Cipher_elg>* >* c = input.getCMatrix(); // contains the original input ciphertexts

        // c does not return safely! it's on the stack...so we must make a copy
        vector<vector<Cipher_elg>* >* c_copy = new vector<vector<Cipher_elg>* >(c->size());
        for (unsigned int i = 0; i < c->size(); i++) {
          vector<Cipher_elg> *temp = new vector<Cipher_elg>(*(c->at(i)));
          c_copy->at(i) = temp;
        }

	RemoteShuffler *P = new RemoteShuffler(num, c_copy, (ElGammal*)reenc_key, m, number_of_cols, true);
	
	CipherTable output(P->getC(), m);
	int element_size;
	*shuffled_ciphers = (char*) get_ciphertexts(&output, shuffled_ciphers_len, &element_size);
	
	vector<long> reversed;
	P->reverse_permutation(reversed);
	*permutation_len = reversed.size();
	int* out_perm = new int[*permutation_len];
	for (int i = 0; i < *permutation_len; i++) {
		out_perm[i] = reversed.at(i);
	}
	*permutation = out_perm;

        return P;
}

// requires shuffle_internal to be called first and cached data must be passed in
// on exit, deletes cached shuffle data
// TODO cache freeing function may be cleaner interface
void prove(void *cache_data, char** proof_out, int* proof_len, char** public_randoms, int* public_randoms_len) {
	init();

        RemoteShuffler *P = (RemoteShuffler*) cache_data;

	string proof = P->create_nizk();

	*proof_len = proof.size();
	*proof_out = new char [*proof_len + 1];
	(*proof_out)[*proof_len] = '\0';
	memcpy(*proof_out, proof.c_str(), *proof_len);
	
	string pubv = P->get_public_vector();
	
	char* rands = new char[pubv.size() + 1];
	rands[pubv.size()] = '\0';
	memcpy(rands, pubv.c_str(), pubv.size());
	*public_randoms_len = pubv.size();
	*public_randoms = rands;

        delete P;
}

int verify(void *elgammal, char* proof, int proof_len, char* ciphers_in, int len_in, char* post_shuffle_cipehrs, int post_shuffle_cipehrs_len, char* public_randoms, int public_randoms_len) {
	init();
	string inp(ciphers_in, len_in);
	string out(post_shuffle_cipehrs, post_shuffle_cipehrs_len);
	CipherTable c(inp, m, (ElGammal*)elgammal);
	CipherTable C(out, m, (ElGammal*)elgammal);
	
	if ((c.rows() != C.rows()) || (c.cols() != C.cols())) {
		return false;
	}
	
	string in_rands(public_randoms, public_randoms_len);
	istringstream respstream(in_rands);
	
	VerifierClient V(num, C.rows(), C.cols(), c.getCMatrix(), C.getCMatrix(), (ElGammal*)elgammal, false, true);
	V.set_public_vector(respstream, c.cols(), num[3], num[7], num[4]);
	string proof_s(proof, proof_len);
        delete[] public_randoms;
	if (V.process_nizk(proof_s)) {
		return 1;
	}
	return 0;
}

void delete_int_arr(int* x) {
	delete [] x;
}

int get_int_elem(int* arr, int i) {
	return arr[i];
}
