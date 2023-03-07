#pragma once

#include "Shader.h"


namespace gre {
	class Kernel {
		GLuint offset_ = 10;
		double kernel_[3][3];

	public:
		Kernel() noexcept {
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					kernel_[i][j] = 0.0;
				}
			}

			kernel_[1][1] = 1.0;
		}

		explicit Kernel(GLuint offset) noexcept : Kernel() {
			offset_ = offset;
		}

		template <typename T>  // Casts required: double(T)
		Kernel(GLuint offset, const std::initializer_list<std::initializer_list<T>>& init) {
#ifdef _DEBUG
			if (init.size() != 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Matrix4x4, invalid init size.\n\n");
			}
#endif // _DEBUG

			offset_ = offset;

			uint32_t i = 0;
			for (const auto& line : init) {
#ifdef _DEBUG
				if (line.size() != 3) {
					throw GreInvalidArgument(__FILE__, __LINE__, "Matrix4x4, invalid init size.\n\n");
				}
#endif // _DEBUG

				uint32_t j = 0;
				for (const auto& element : line) {
					kernel_[i][j++] = double(element);
				}
				++i;
			}
		}

		double* operator[](size_t index) {
#ifdef _DEBUG
			if (3 <= index) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}
#endif // _DEBUG

			return kernel_[index];
		}

		const double* operator[](size_t index) const {
#ifdef _DEBUG
			if (3 <= index) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}
#endif // _DEBUG

			return kernel_[index];
		}

		bool operator==(const Kernel& other) const noexcept {
			for (uint32_t i = 0; i < 3; ++i) {
				for (uint32_t j = 0; j < 3; ++j) {
					if (!equality(kernel_[i][j], other[i][j])) {
						return false;
					}
				}
			}
			return true;
		}

		bool operator!=(const Kernel& other) const noexcept {
			return !(*this == other);
		}

		// POST shader expected
		void set_uniforms(const Shader& shader) const {
			shader.set_uniform_i("offset", offset_);

			std::vector<GLfloat> data;
			data.reserve(9);
			for (size_t j = 0; j < 3; ++j) {
				for (size_t i = 0; i < 3; ++i) {
					data.push_back(static_cast<GLfloat>(kernel_[i][j]));
				}
			}

			shader.set_uniform_1fv("kernel", 9, &(data[0]));
		}

		void set_offset(GLuint offset) noexcept {
			offset_ = offset;
		}

		GLuint get_offset() const noexcept {
			return offset_;
		}

		friend std::istream& operator>>(std::istream& fin, Kernel& kernel) noexcept;
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
}
