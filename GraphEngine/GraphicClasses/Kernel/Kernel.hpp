#pragma once

#include "../Shader/Shader.hpp"


// Convolution kernel for post shader
namespace gre {
	class Kernel {
		GLuint offset_ = 10;
		double kernel_[3][3];

	public:
		// Constructors
		Kernel() noexcept;

		explicit Kernel(GLuint offset) noexcept;

		template <typename T>  // Casts required: double(T)
		Kernel(GLuint offset, const std::initializer_list<std::initializer_list<T>>& init) {
			GRE_ENSURE(init.size() == 3, GreInvalidArgument, "invalid init size");
			offset_ = offset;

			uint32_t i = 0;
			for (const auto& line : init) {
				GRE_ENSURE(line.size() == 3, GreInvalidArgument, "invalid init size");

				uint32_t j = 0;
				for (const auto& element : line) {
					kernel_[i][j++] = double(element);
				}
				++i;
			}
		}

		// Operators
		double* operator[](size_t index);

		const double* operator[](size_t index) const;

		bool operator==(const Kernel& other) const noexcept;

		bool operator!=(const Kernel& other) const noexcept;

		// Seters
		void set_offset(GLuint offset) noexcept;

		// Getters
		GLuint get_offset() const noexcept;

		// Uploading into shader

		// POST shader expected
		void set_uniforms(const Shader& shader) const;

		// Friend members
		friend std::istream& operator>>(std::istream& fin, Kernel& kernel);
	};

	// External operators
	std::istream& operator>>(std::istream& fin, Kernel& kernel);
}  // namespace gre
