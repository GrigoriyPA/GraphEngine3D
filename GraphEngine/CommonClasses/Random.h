#pragma once

#include <random>
#include "Vect3.h"


namespace eng {
	class Random {
		std::mt19937_64 generator;

	public:
		explicit Random(unsigned long long seed) {
			generator.seed(seed);
		}

		void set_seed(unsigned long long seed) & {
			generator.seed(seed);
		}

		double rand() {
			return static_cast<double>(generator()) / static_cast<double>(generator.max());
		}

		// In case of an error returns left
		long long rand_int(long long left, long long right) {
			if (right < left) {
				std::cout << "ERROR::RANDOM::RAND_INT\n" << "Invalid range.\n\n";
				return left;
			}

			return static_cast<long long>(generator()) % (right - left + 1) + left;
		}

		// In case of an error returns left
		double rand_float(double left, double right) {
			if (right < left) {
				std::cout << "ERROR::RANDOM::RAND_FLOAT\n" << "Invalid range.\n\n";
				return left;
			}

			return (right - left) * rand() + left;
		}

		// In case of an error returns left
		Vect3 rand_vect3(Vect3 left, Vect3 right) {
			if (Vect3::get_max(left, right) != right) {
				std::cout << "ERROR::RANDOM::RAND_VECT3\n" << "Invalid range.\n\n";
			}

			return Vect3(rand_float(left.x, right.x), rand_float(left.y, right.y), rand_float(left.z, right.z));
		}
	};
}
