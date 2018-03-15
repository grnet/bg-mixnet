#include "RemoteShuffler.h"

#include "NIZKProof.h"

extern long mu;
extern long mu_h;

RemoteShuffler::RemoteShuffler(const vector<long>& config, vector<vector<Cipher_elg>* >* ciphers, ElGammal* reenc_key, int m_in, int n_in, bool owner) : config_(config), time_rw_p(0), time_rw_v(0), time_cm(0),
  c(ciphers), C(nullptr), pi(nullptr), R(nullptr), m(m_in), n(n_in), time_p(0), P(nullptr), owner_(owner), flow_flag_(false) {

	mu = 4;
	mu_h = 2*mu-1;

	m_r_ = m/mu;

	flow_flag_ = (m_r_ == 4);
	key = new ElGammal(*reenc_key);
	permute_and_reencrypt(reenc_key);
}

RemoteShuffler::~RemoteShuffler() {
	if (owner_) {
		if (c != nullptr) Functions::delete_vector(c);
	}
	if (C != nullptr) Functions::delete_vector(C);
	if (pi != nullptr) Functions::delete_vector(pi);
	if (R != nullptr) Functions::delete_vector(R);
	if (P != nullptr) delete P;
	if (verifier_ != nullptr) delete verifier_;
        delete key;
}

string RemoteShuffler::get_public_vector() {
	return P->get_public_vector();
}

void RemoteShuffler::reverse_permutation(vector<long>& reversed) {
	reversed = Functions::permutation2d_to_vector(pi, m, n);
}

vector<vector<Cipher_elg>* >* RemoteShuffler::permute_and_reencrypt(ElGammal* reenc_key) {
	//auto tstart = high_resolution_clock::now();
	pi = new vector<vector<vector<long>* >* >(m);
	Permutation::perm_matrix(pi,n,m);
	R = new vector<vector<ZZ>*>(m);
	Functions::randomEl(R,m,n);
	C = new vector<vector<Cipher_elg>* >(m);
	Functions::reencryptCipher(C,c,pi,R,m,n, reenc_key);
	//auto tstop = high_resolution_clock::now();
	//duration<double> ttime= tstop-tstart;
	// cout << "To permute and rerandomize the ciphertexts took " << ttime.count() << " second(s)." << endl;
	return C;
}

string RemoteShuffler::create_nizk() {
	NIZKProof proof;

	string v;
	ZZ c,r;
	string input_for_next_prover;
	
	P = new Prover_toom(m_r_, C,R,pi,config_, m, n, key);
	verifier_ = new VerifierClient(config_, m, n, this->c, C, key, false, false);

	v = round1(&input_for_next_prover, &c, &r);
	proof.add_new_step(v, c, r);
	v = round3(input_for_next_prover, &input_for_next_prover, &c, &r);
	proof.add_new_step(v, c, r);
	if (verifier_->flow_flag_){
		v = round5(input_for_next_prover, &input_for_next_prover, &c, &r);
		proof.add_new_step(v, c, r);
		v = round7(input_for_next_prover, &input_for_next_prover, &c, &r);
		proof.add_new_step(v, c, r);
	} else {
		while (m_r_ > 4) {
			v = round5red(input_for_next_prover, &input_for_next_prover, &c,
                                      &r);
			proof.add_new_step(v, c, r);
			v = round5red_1(input_for_next_prover, &input_for_next_prover,
                                        &c, &r);
			proof.add_new_step(v, c, r);
		}
		v = round7red(input_for_next_prover, &input_for_next_prover, &c, &r);
		proof.add_new_step(v, c, r);
	}
	v = round9(input_for_next_prover);
	proof.add_final_step(v);
	return proof.proof();
}

string RemoteShuffler::round1(string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	time_p=0;
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_1();
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 1 " << ttime << " second(s)." << endl;
	
	time_p+=ttime;
	*input_for_next_prover = verifier_->round2(input_to_V, challenge, randomness);
	return input_to_V;
}

string RemoteShuffler::round3(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_3(input_file);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 3 " << ttime << " second(s)." << endl;
	time_p+=ttime;
	*input_for_next_prover = verifier_->round4(input_to_V, challenge, randomness);
	
	return input_to_V;
}

string RemoteShuffler::round5(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_5(input_file);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 5 " << ttime << " second(s)." << endl;
	
	*input_for_next_prover = verifier_->round6(input_to_V, challenge, randomness);
	
	time_p+=ttime;
	return input_to_V;
}

string RemoteShuffler::round5red(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_5_red(input_file);
	m_r_ /= mu;
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 5 " << ttime << " second(s)." << endl;
	*input_for_next_prover = verifier_->round6red(input_to_V, challenge, randomness);
	
	time_p+=ttime;
	return input_to_V;
}

string RemoteShuffler::round5red_1(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_5_red1(input_file);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 5 " << ttime << " second(s)." << endl;
	*input_for_next_prover = verifier_->round6red_1(input_to_V, challenge, randomness);
	time_p+=ttime;
	return input_to_V;
}

string RemoteShuffler::round7(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_7(input_file);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 7 " << ttime << " second(s)." << endl;
	*input_for_next_prover = verifier_->round8(input_to_V, challenge, randomness);
	time_p+=ttime;
	return input_to_V;
}

string RemoteShuffler::round7red(const string& input_file, string* input_for_next_prover, ZZ* challenge, ZZ* randomness) {
	auto tstart = high_resolution_clock::now();
	string input_to_V = P->round_7_red(input_file);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 7 " << ttime << " second(s)." << endl;
	*input_for_next_prover = verifier_->round8(input_to_V, challenge, randomness);
	time_p+=ttime;
	return input_to_V;
}


string RemoteShuffler::round9(const string& input_file) {
	string output_file;
	auto tstart = high_resolution_clock::now();
	output_file = P->round_9(input_file);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 9 " << ttime << " second(s)." << endl;
	time_p+=ttime;

	//ofstream ost("shuffle_with_toom_cook_P.txt", ios::app);
	//ost<< time_p<<endl;
	//ost.close();

	return output_file;
}

void RemoteShuffler::print_state() const {
	cout << "+++++++" << endl;
	cout << m_r_ << endl;
	cout << mu << endl;
	cout << mu_h << endl;
	cout << "-------" << endl;
}


