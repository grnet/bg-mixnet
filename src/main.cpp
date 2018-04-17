#include "Bgmix.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdlib.h>  // atol()

static int kNumTests = 1;

void test_mix(long dim_m, long dim_n) {
	char ciphers_file[] = "ciphers.json";
	generate_ciphers(ciphers_file, dim_m, dim_n);
	mix(ciphers_file, dim_m, dim_n);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cout << "Wrong number of arguments. Expected:" << std::endl;
		std::cout << "./bgmix <number of cipher matrix rows> <number of cipher matrix columns>" << std::endl;
		exit(1);
	}

	time_t begin = time(NULL);
	std::thread* th_arr[kNumTests];
	for (int i = 0; i < kNumTests; i++) {
		th_arr[i] = new std::thread(test_mix, atol(argv[1]), atol(argv[2]));
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
