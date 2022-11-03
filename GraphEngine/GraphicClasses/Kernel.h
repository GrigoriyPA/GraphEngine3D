#pragma once

#include "Shader.h"


namespace eng {
	class Kernel {
		Matrix kernel_ = Matrix(3, 3, 0.0);

	public:
		Kernel() noexcept {
			kernel_[1][1] = 1.0;
		}

		template <typename T>  // Casts required: double(T)
		Kernel(const std::initializer_list<std::initializer_list<T>>& init) {
			if (init.size() != 3 || init.begin()->size() != 3) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Kernel, invalid kernel size.\n\n");
			}

			kernel_ = Matrix(init);
		}

		auto& operator[](size_t index) {
			if (kernel_.count_strings() <= index) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return kernel_[index];
		}

		const auto& operator[](size_t index) const {
			if (kernel_.count_strings() <= index) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return kernel_[index];
		}

		bool operator==(const Kernel& other) const noexcept {
			return kernel_ == other.kernel_;
		}

		bool operator!=(const Kernel& other) const noexcept {
			return !(*this == other);
		}

		friend std::ostream& operator<<(std::ostream& fout, const Kernel& kernel) noexcept;

		template <typename T>
		void use(const std::string& uniform_name, const Shader<T>& shader) const {
			shader.set_uniform_fv<1>(uniform_name.c_str(), 9, &std::vector<float>(kernel_)[0]);
		}
	};

	std::istream& operator>>(std::istream& fin, Kernel& kernel) noexcept {
		for (size_t i = 0; i < 3; ++i) {
			for (size_t j = 0; j < 3; ++j) {
				fin >> kernel[i][j];
			}
		}
		return fin;
	}

	std::ostream& operator<<(std::ostream& fout, const Kernel& kernel) noexcept {
		fout << kernel.kernel_;
		return fout;
	}
}
