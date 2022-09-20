#pragma once

#include <cassert>
#include <iostream>
#include <random>
#include "Vect3.h"


class Random {
	std::mt19937_64 generator;

public:
	Random(int seed) {
		generator.seed(seed);
	}

	void set_seed(int seed) {
		generator.seed(seed);
	}

	double rand() {
		return ((double)generator()) / ((double)generator.max());
	}

	long long rand_int(long long l, long long r) {
		if (r < l) {
			std::cout << "ERROR::RANDOM::RAND_INT\n" << "Invalid range.\n";
			assert(0);
		}

		return ((long long)(generator() % (r - l + 1))) + l;
	}

	double rand_float(double l, double r) {
		if (r < l) {
			std::cout << "ERROR::RANDOM::RAND_FLOAT\n" << "Invalid range.\n";
			assert(0);
		}

		return (r - l) * rand() + l;
	}

	Vect3 rand_vect3(Vect3 l, Vect3 r) {
		if (l.set_max(r) != r) {
			std::cout << "ERROR::RANDOM::RAND_VECT3\n" << "Invalid range.\n";
			assert(0);
		}

		return Vect3(rand_float(l.x, r.x), rand_float(l.y, r.y), rand_float(l.z, r.z));
	}
};
