#pragma once

#include <algorithm>
#include <random>


namespace eng {
	class Random {
		std::mt19937_64 generator;

	public:
		explicit Random(uint64_t seed) noexcept {
			generator.seed(seed);
		}

		Random& set_seed(uint64_t seed) noexcept {
			generator.seed(seed);
			return *this;
		}

		double rand() noexcept {
			return static_cast<double>(generator()) / static_cast<double>(generator.max());
		}

		int64_t rand_int(int64_t left, int64_t right) {
			if (right < left) {
				throw EngInvalidArgument(__FILE__, __LINE__, "rand_int, invalid range.\n\n");
			}

			return static_cast<int64_t>(generator() % static_cast<uint64_t>(right - left + 1)) + left;
		}

		double rand_float(double left, double right) {
			if (right < left) {
				throw EngInvalidArgument(__FILE__, __LINE__, "rand_float, invalid range.\n\n");
			}

			return (right - left) * rand() + left;
		}

		template <typename T>  // Casts required: int64_t(T::value)
		T rand_int_struct(T left, T right) {
			return T::zip_map(left, right, [&](auto left, auto right) { return rand_int(static_cast<int64_t>(left), static_cast<int64_t>(right)); });
		}

		template <typename T>  // Casts required: double(T::value)
		T rand_float_struct(T left, T right) {
			return T::zip_map(left, right, [&](auto left, auto right) { return rand_float(static_cast<double>(left), static_cast<double>(right)); });
		}
	};
}
