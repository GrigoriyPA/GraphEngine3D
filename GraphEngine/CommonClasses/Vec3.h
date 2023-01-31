#pragma once

#include <SFML/Graphics.hpp>
#include "Vec2.h"


namespace gre {
	class Vec3 {
	public:
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;

		Vec3() noexcept {
		}

		explicit Vec3(double value) noexcept {
			x = value;
			y = value;
			z = value;
		}

		Vec3(double x, double y, double z) noexcept {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		Vec3(const Vec2& xy, double z) noexcept {
			x = xy.x;
			y = xy.y;
			this->z = z;
		}

		Vec3(double x, const Vec2& yz) noexcept {
			this->x = x;
			y = yz.x;
			z = yz.y;
		}

		template <typename T>  // Casts required: double(T)
		Vec3(const std::initializer_list<T>& init) {
#ifdef _DEBUG
			if (init.size() != 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Vec3, invalid initializer list size.\n\n");
			}
#endif // _DEBUG

			size_t comp_id = 0;
			for (const T& element : init) {
				(*this)[comp_id++] = static_cast<double>(element);
			}
		}

		explicit Vec3(const sf::Color& color) noexcept {
			x = color.r;
			y = color.g;
			z = color.b;
		}

		double& operator[](size_t index) {
			if (index == 0) {
				return x;
			}
			if (index == 1) {
				return y;
			}

#ifdef _DEBUG
			if (index == 2) {
				return z;
			}
			throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
#endif // _DEBUG

			return z;
		}

		const double& operator[](size_t index) const {
			if (index == 0) {
				return x;
			}
			if (index == 1) {
				return y;
			}

#ifdef _DEBUG
			if (index == 2) {
				return z;
			}
			throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
#endif // _DEBUG

			return z;
		}

		bool operator==(const Vec3& other) const noexcept {
			return equality(x, other.x) && equality(y, other.y) && equality(z, other.z);
		}

		bool operator!=(const Vec3& other) const noexcept {
			return !equality(x, other.x) || !equality(y, other.y) || !equality(z, other.z);
		}

		Vec3& operator+=(const Vec3& other)& noexcept {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vec3& operator-=(const Vec3& other)& noexcept {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		Vec3& operator*=(double other)& noexcept {
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}

		Vec3& operator/=(double other)& {
#ifdef _DEBUG
			if (equality(other, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
			}
#endif // _DEBUG

			x /= other;
			y /= other;
			z /= other;
			return *this;
		}

		Vec3& operator^=(double other)& {
#ifdef _DEBUG
			if (x < 0.0 || y < 0.0 || z < 0.0) {
				throw GreDomainError(__FILE__, __LINE__, "operator^=, raising a negative number to a power.\n\n");
			}
#endif // _DEBUG

			x = std::pow(x, other);
			y = std::pow(y, other);
			z = std::pow(z, other);
			return *this;
		}

		Vec3 operator-() const noexcept {
			return Vec3(-x, -y, -z);
		}

		Vec3 operator+(const Vec3& other) const noexcept {
			return Vec3(x + other.x, y + other.y, z + other.z);
		}

		Vec3 operator-(const Vec3& other) const noexcept {
			return Vec3(x - other.x, y - other.y, z - other.z);
		}

		Vec3 operator*(double other) const noexcept {
			return Vec3(x * other, y * other, z * other);
		}

		double operator*(const Vec3& other) const noexcept {
			return x * other.x + y * other.y + z * other.z;
		}

		Vec3 operator/(double other) const {
#ifdef _DEBUG
			if (equality(other, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
			}
#endif // _DEBUG

			return Vec3(x / other, y / other, z / other);
		}

		Vec3 operator^(const Vec3& other) const noexcept {
			return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
		}

		Vec3 operator^(double other) const {
#ifdef _DEBUG
			if (x < 0.0 || y < 0.0 || z < 0.0) {
				throw GreDomainError(__FILE__, __LINE__, "operator^, raising a negative number to a power.\n\n");
			}
#endif // _DEBUG

			return Vec3(std::pow(x, other), std::pow(y, other), std::pow(z, other));
		}

		double length() const {
			return std::sqrt(x * x + y * y + z * z);
		}

		Vec3 normalize() const {
			double vect_length = length();

#ifdef _DEBUG
			if (equality(vect_length, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "normalize, null vector normalization.\n\n");
			}
#endif // _DEBUG

			return *this / vect_length;
		}

		Vec3 horizont() const noexcept {
			double vect_length = z * z + x * x;

			if (equality(vect_length, 0.0)) {
				return Vec3(1.0, 0.0, 0.0);
			}
			return Vec3(z / vect_length, 0.0, -x / vect_length);
		}
	};

	std::ostream& operator<<(std::ostream& fout, const Vec3& vector) noexcept {
		fout << '(' << vector.x << ", " << vector.y << ", " << vector.z << ')';
		return fout;
	}

	Vec3 operator*(double value, const Vec3& vector) noexcept {
		return Vec3(vector.x * value, vector.y * value, vector.z * value);
	}
}

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
