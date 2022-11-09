#pragma once

#include "Shader.h"


namespace eng {
	class Kernel {
		GLuint offset_ = 0;
		Matrix kernel_ = Matrix(3, 3, 0.0);

	public:
		Kernel() noexcept {
			kernel_[1][1] = 1.0;
		}

		template <typename T>  // Casts required: double(T)
		Kernel(GLuint offset, const std::initializer_list<std::initializer_list<T>>& init) {
			if (init.size() != 3 || init.begin()->size() != 3) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Kernel, invalid kernel size.\n\n");
			}

			offset_ = offset;
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

		friend std::istream& operator>>(std::istream& fin, Kernel& kernel) noexcept;

		friend std::ostream& operator<<(std::ostream& fout, const Kernel& kernel) noexcept;

		void set_uniforms(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::POST) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
			}

			shader.set_uniform_i("offset", offset_);
			shader.set_uniform_fv<1>("kernel", 9, &std::vector<GLfloat>(kernel_)[0]);
		}

		Kernel& set_offset(GLuint offset) noexcept {
			offset_ = offset;
			return *this;
		}

		GLuint get_offset() const noexcept {
			return offset_;
		}
	};

	std::istream& operator>>(std::istream& fin, Kernel& kernel) noexcept {
		fin >> kernel.offset_;
		for (size_t i = 0; i < 3; ++i) {
			for (size_t j = 0; j < 3; ++j) {
				fin >> kernel[i][j];
			}
		}
		return fin;
	}

	std::ostream& operator<<(std::ostream& fout, const Kernel& kernel) noexcept {
		fout << kernel.offset_ << "\n" << kernel.kernel_;
		return fout;
	}
}
