#pragma once

#include <SFML/Graphics.hpp>
#include "Vec2.hpp"


// Representation of a three-dimensional vector
namespace gre {
	class Vec3 {
	public:
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;

		// Constructors
		Vec3() noexcept;

		explicit Vec3(double value) noexcept;

		Vec3(double x, double y, double z) noexcept;

		Vec3(const Vec2& xy, double z) noexcept;

		Vec3(double x, const Vec2& yz) noexcept;

		template <typename T>  // Casts required: double(T)
		Vec3(const std::initializer_list<T>& init) {
			GRE_ENSURE(init.size() == 3, GreInvalidArgument, "invalid initializer list size");

			size_t comp_id = 0;
			for (const T& element : init) {
				(*this)[comp_id++] = static_cast<double>(element);
			}
		}

		explicit Vec3(const sf::Color& color) noexcept;

		explicit Vec3(const sf::Vector3f& init) noexcept;

		explicit Vec3(const aiVector3D& init) noexcept;

		// Operators
		double& operator[](size_t index);

		const double& operator[](size_t index) const;

		bool operator==(const Vec3& other) const noexcept;

		bool operator!=(const Vec3& other) const noexcept;

		Vec3& operator+=(const Vec3& other)& noexcept;

		Vec3& operator-=(const Vec3& other)& noexcept;

		Vec3& operator*=(double other)& noexcept;

		Vec3& operator/=(double other)& noexcept;

		Vec3& operator^=(double other)& noexcept;

		Vec3 operator-() const noexcept;

		Vec3 operator+(const Vec3& other) const noexcept;

		Vec3 operator-(const Vec3& other) const noexcept;

		Vec3 operator*(double other) const noexcept;

		double operator*(const Vec3& other) const noexcept;

		Vec3 operator/(double other) const noexcept;

		Vec3 operator^(const Vec3& other) const noexcept;

		Vec3 operator^(double other) const noexcept;

		// Math functions
		double length() const;

		Vec3 normalize() const noexcept;

		// Other functions
		Vec3 horizon() const noexcept;
	};

	// External operators
	std::ostream& operator<<(std::ostream& fout, const Vec3& vector);

	Vec3 operator*(double value, const Vec3& vector) noexcept;
}  // namespace gre

template <>
struct std::hash<gre::Vec3> {
	size_t operator()(const gre::Vec3& vector) const noexcept {
		size_t result = 0;
		gre::hash_combine(result, vector.x);
		gre::hash_combine(result, vector.y);
		gre::hash_combine(result, vector.z);
		return result;
	}
};
