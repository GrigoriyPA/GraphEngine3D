#pragma once

#include <array>
#include "Shader.h"


namespace gre {
	class Kernel {
		GLuint offset_ = 10;
		std::array<std::array<double, 3>, 3> kernel_;

	public:
		Kernel() noexcept {
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					kernel_[i][j] = 0.0;
				}
			}

			kernel_[1][1] = 1.0;
		}

		template <typename T>  // Casts required: double(T)
		Kernel(GLuint offset, const std::initializer_list<std::initializer_list<T>>& init) {
			if (init.size() != 3 || init.begin()->size() != 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Kernel, invalid kernel size.\n\n");
			}

			offset_ = offset;
			kernel_ = Matrix4x4(init);
		}

		auto& operator[](size_t index) {
			if (3 <= index) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return kernel_[index];
		}

		const auto& operator[](size_t index) const {
			if (3 <= index) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
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

		void set_uniforms(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::POST) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
			}

			shader.set_uniform_i("offset", offset_);

			std::vector<GLfloat> data;
			data.reserve(9);
			for (size_t j = 0; j < 3; ++j) {
				for (size_t i = 0; i < 3; ++i) {
					data.push_back(static_cast<GLfloat>(kernel_[i][j]));
				}
			}

			shader.set_uniform_fv<1>("kernel", 9, &(data[0]));
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
}
