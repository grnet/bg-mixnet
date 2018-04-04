#include "Bgmix.h"
#include <iostream>
#include <thread>
#include <chrono>

static int kNumTests = 1;

void test_mix() {
	char ciphers_file[] = "ciphers.txt";
	generate_ciphers(ciphers_file);
	mix(ciphers_file);
}

int main() {
	time_t begin = time(NULL);
	std::thread* th_arr[kNumTests];
	for (int i = 0; i < kNumTests; i++) {
		th_arr[i] = new std::thread(test_mix);
	}

	std::cout << "waiting for everyone..." <<std::endl;
	for (int i = 0; i < kNumTests; i++) {
		th_arr[i]->join();
	}

	std::cout << "stress test is done in " << time(NULL) - begin << " seconds" << std::endl;
	for (int i = 0; i < kNumTests; i++) {
		delete th_arr[i];
	}

	return 0;
}
