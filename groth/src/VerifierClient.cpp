#include "VerifierClient.h"

#include "NIZKProof.h"

#include <chrono>
using namespace std::chrono;

extern long mu;
extern long mu_h;
VerifierClient::VerifierClient(const vector<long>& config, int m, int n, vector<vector<Cipher_elg>* >* ciphers, vector<vector<Cipher_elg>* >* permuted_ciphers, ElGammal* elgammal, bool owner, bool do_process):
  config_(config), c(ciphers), C(permuted_ciphers), time_v(0), owner_(owner), flow_flag_(false) {
	m_r = config_[1]/mu;	
	flow_flag_ = (m_r == 4);
	V = new Verifier_toom(config_, m, n, m_r, do_process, elgammal);
}

VerifierClient::~VerifierClient() {
	if (owner_) {
		Functions::delete_vector(c);
		Functions::delete_vector(C);
	}
	if (V != nullptr) delete V;
}

void VerifierClient::set_public_vector(istringstream& f, long n, int o1, int o2, int o3) {
	V->set_public_vector(f, n, o1, o2, o3);
}


bool VerifierClient::process_nizk(string nizk) {
	NIZKProof proof(nizk);
	string input_to_ver;
	ZZ challenge, rand;
	
	proof.read_next(input_to_ver, challenge, rand);
	round2(input_to_ver, challenge, rand);
	
	proof.read_next(input_to_ver, challenge, rand);
	round4(input_to_ver, challenge, rand);
	
	if (flow_flag_) {
		proof.read_next(input_to_ver, challenge, rand);
		round6(input_to_ver, challenge, rand);
	} else {
		proof.read_next(input_to_ver, challenge, rand);
		round6red(input_to_ver, challenge, rand);
		proof.read_next(input_to_ver, challenge, rand);
		round6red_1(input_to_ver, challenge, rand);
	}
	proof.read_next(input_to_ver, challenge, rand);
	round8(input_to_ver, challenge, rand);
	
	proof.read_final_step(input_to_ver);
	if (flow_flag_) {
		return round10(input_to_ver);
	} else {
		return round10red(input_to_ver);
	}
}


string VerifierClient::round2(const string& input_file, ZZ* challenge, ZZ* random) {
	auto tstart = high_resolution_clock::now();
	string output_file = V->round_2(input_file, challenge, random);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 2 " << ttime << " second(s)." << endl;
	time_v+=ttime;
	return output_file;
}

string VerifierClient::round2(const string& input_file, ZZ& challenge, ZZ& random) {
	auto tstart = high_resolution_clock::now();
	string output_file = V->round_2(input_file, challenge, random);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 2 " << ttime << " second(s)." << endl;
	time_v+=ttime;
	return output_file;
}

string VerifierClient::round4(const string& input_file, ZZ* challenge, ZZ* random) {
	auto tstart = high_resolution_clock::now();
	string output_file = V->round_4(input_file, challenge, random);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	time_v+=ttime;
	return output_file;
}

string VerifierClient::round4(const string& input_file, ZZ& challenge, ZZ& rand) {
	auto tstart = high_resolution_clock::now();
	string output_file = V->round_4(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	time_v+=ttime;
	return output_file;
}

string VerifierClient::round6(const string& input_file, ZZ* challenge, ZZ* rand) {
	auto tstart = high_resolution_clock::now();
	string output_file;
	output_file = V->round_6(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 6 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}


string VerifierClient::round6(const string& input_file, ZZ& challenge, ZZ& rand) {
	auto tstart = high_resolution_clock::now();
	string output_file;
	output_file = V->round_6(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 6 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

string VerifierClient::round6red(const string& input_file, ZZ* challenge, ZZ* rand) {
	auto tstart = high_resolution_clock::now();
	string output_file;
//        cout << "c: " << c << endl;
//        cout << "c->at(0): " << c->at(0) << endl;
//        cout << "c->size(): " << c->size() << endl;
	output_file = V->round_6_red(input_file, c, challenge, rand);
	m_r=m_r/mu;
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 6 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

string VerifierClient::round6red(const string& input_file, ZZ& challenge, ZZ& rand) {
	auto tstart = high_resolution_clock::now();
	string output_file;
	output_file = V->round_6_red(input_file, c, challenge, rand);
	m_r=m_r/mu;
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 6 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

string VerifierClient::round6red_1(const string& input_file, ZZ* challenge, ZZ* rand) {
	auto tstart = high_resolution_clock::now();
	string output_file;
	output_file = V->round_6_red1(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 6 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

string VerifierClient::round6red_1(const string& input_file, ZZ& challenge, ZZ& rand) {
	auto tstart = high_resolution_clock::now();
	string output_file;
	output_file = V->round_6_red1(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 6 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

string VerifierClient::round8(const string& input_file, ZZ* challenge, ZZ* rand) {
	auto tstart = high_resolution_clock::now();
	string output_file = V->round_8(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 8 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

string VerifierClient::round8(const string& input_file, ZZ& challenge, ZZ& rand) {
	auto tstart = high_resolution_clock::now();
	string output_file = V->round_8(input_file, challenge, rand);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 8 " << ttime << " second(s)." << endl;
	time_v += ttime;
	return output_file;
}

bool VerifierClient::round10(const string& input_file) {
	auto tstart = high_resolution_clock::now();
	bool output;
	output = V->round_10(input_file, c, C);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 10 " << ttime << " second(s)." << endl;
	time_v += ttime;

	//ofstream ost("shuffle_with_toom_cook_V.txt",ios::app);
	//ost<< time_v<<endl;
	//ost.close();
	return output;
}

bool VerifierClient::round10red(const string& input_file) {
	auto tstart = high_resolution_clock::now();
	bool output;
	output = V->round_10_red(input_file, c, C);
	auto tstop = high_resolution_clock::now();
	double ttime= duration<double>(tstop-tstart).count();
	// cout << "Time for round 10 " << ttime << " second(s)." << endl;
	time_v += ttime;

	//ofstream ost("shuffle_with_toom_cook_V.txt",ios::app);
	//ost<< time_v<<endl;
	//ost.close();
	return output;
}

void VerifierClient::print_state() const {
	cout << "+++++++" << endl;
	cout << m_r << endl;
	cout << mu << endl;
	cout << mu_h << endl;
	cout << "-------" << endl;
}


