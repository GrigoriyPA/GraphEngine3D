#pragma once

#include <random>
#include "Vect3.h"


namespace eng {
	class Random {
		std::mt19937_64 generator;

	public:
		explicit Random(uint64_t seed) noexcept {
			generator.seed(seed);
		}

		Random& set_seed(uint64_t seed)& noexcept {
			generator.seed(seed);
			return *this;
		}

		double rand() noexcept {
			return static_cast<double>(generator()) / static_cast<double>(generator.max());
		}

		int64_t rand_int(int64_t left, int64_t right) {
			if (right < left) {
				throw eng_exceptions::EngInvalidArgument(__FILE__, __LINE__, "rand_int, invalid range.\n\n");
			}

			return static_cast<int64_t>(generator()) % (right - left + 1) + left;
		}

		double rand_float(double left, double right) {
			if (right < left) {
				throw eng_exceptions::EngInvalidArgument(__FILE__, __LINE__, "rand_float, invalid range.\n\n");
			}

			return (right - left) * rand() + left;
		}

		Vect3 rand_vect3(Vect3 left, Vect3 right) {
			if (Vect3::zip_map(left, right, [](auto left, auto right) { return std::max(left, right); }) != right) {
				throw eng_exceptions::EngInvalidArgument(__FILE__, __LINE__, "rand_vect3, invalid range.\n\n");
			}

			return Vect3(rand_float(left.x, right.x), rand_float(left.y, right.y), rand_float(left.z, right.z));
		}
	};
}
