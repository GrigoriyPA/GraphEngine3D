#pragma once

#include "Vec3.hpp"


// Representation of a quaternion
namespace gre {
	class Quaternion {
	public:
		double x = 0.0;
		double y = 0.0;
		double u = 0.0;
		double v = 0.0;

		// Constructors
		Quaternion() noexcept;
		
		explicit Quaternion(double value) noexcept;

		Quaternion(double real, const Vec3& imaginary) noexcept;

		Quaternion(double x, double y, double u, double v) noexcept;

		template <typename T>  // Casts required: double(T)
		Quaternion(const std::initializer_list<T>& init) {
			GRE_ENSURE(init.size() == 4, GreInvalidArgument, "invalid initializer list size");

			size_t comp_id = 0;
			for (const T& element : init) {
				(*this)[comp_id++] = static_cast<double>(element);
			}
		}

		// Operators
		double& operator[](size_t index);

		const double& operator[](size_t index) const;

		bool operator==(const Quaternion& other) const noexcept;

		bool operator!=(const Quaternion& other) const noexcept;

		Quaternion& operator+=(const Quaternion& other)& noexcept;

		Quaternion& operator-=(const Quaternion& other)& noexcept;

		Quaternion& operator*=(const Quaternion& other)& noexcept;

		Quaternion operator~() const noexcept;

		Quaternion operator-() const noexcept;

		Quaternion operator+(const Quaternion& other) const noexcept;

		Quaternion operator-(const Quaternion& other) const noexcept;

		Quaternion operator*(const Quaternion& other) const noexcept;

		// Math functions
		Vec3 get_imaginary() const noexcept;

		Vec3 rotate(const Vec3& vector) const noexcept;

		// Precalculated matrices
		Quaternion rotation_quaternion(const Vec3& axis, double angle) noexcept;
	};

	// External operators
	std::ostream& operator<<(std::ostream& fout, const Quaternion& quaternion);
}  // namespace gre
