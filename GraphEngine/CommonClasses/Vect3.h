#pragma once

#include <initializer_list>
#include <SFML/Graphics/Color.hpp>
#include "Functions.h"


namespace eng {
	class Vect3 {
		inline static double eps_ = 1e-5;

	public:
		double x = 0;
		double y = 0;
		double z = 0;

		Vect3(double x, double y, double z) noexcept {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		template <typename T>  // Casts required: double(T)
		Vect3(const std::initializer_list<T>& init) {
			size_t comp_id = 0;
			for (const T& element : init) {
				(*this)[comp_id] = static_cast<double>(element);

				if (++comp_id == 3) {
					break;
				}
			}
		}

		template <typename T>  // Casts required: double(T)
		explicit Vect3(const std::vector<T>& init) {
			for (size_t comp_id = 0; comp_id < std::min(static_cast<size_t>(3), init.size()); ++comp_id) {
				(*this)[comp_id] = static_cast<double>(init[comp_id]);
			}
		}

		explicit Vect3(const sf::Color& color) noexcept {
			x = color.r;
			y = color.g;
			z = color.b;
		}

		Vect3() noexcept {
		}

		template <typename T>  // Constructors required: T(T)
		explicit operator std::vector<T>() const {
			return { T(x), T(y), T(z) };
		}

		explicit operator std::string() const noexcept {
			return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
		}

		double& operator[](size_t index) {
			if (index == 0) {
				return x;
			}
			if (index == 1) {
				return y;
			}
			if (index == 2) {
				return z;
			}
			throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
		}

		const double& operator[](size_t index) const {
			if (index == 0) {
				return x;
			}
			if (index == 1) {
				return y;
			}
			if (index == 2) {
				return z;
			}
			throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
		}

		bool operator==(const Vect3& other) const noexcept {
			return equality(x, other.x, eps_) && equality(y, other.y, eps_) && equality(z, other.z, eps_);
		}

		bool operator!=(const Vect3& other) const noexcept {
			return !(*this == other);
		}

		Vect3& operator+=(const Vect3& other)& noexcept {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vect3& operator-=(const Vect3& other)& noexcept {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		Vect3& operator*=(double other)& noexcept {
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}

		Vect3& operator/=(double other)& {
			if (equality(other, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
			}

			x /= other;
			y /= other;
			z /= other;
			return *this;
		}

		Vect3& operator^=(double other)& {
			if (x < 0 || y < 0 || z < 0) {
				throw EngDomainError(__FILE__, __LINE__, "operator^=, raising a negative number to a power.\n\n");
			}

			x = pow(x, other);
			y = pow(y, other);
			z = pow(z, other);
			return *this;
		}

		Vect3& operator^=(uint32_t other)& noexcept {
			x = binary_exponentiation(x, other);
			y = binary_exponentiation(y, other);
			z = binary_exponentiation(z, other);
			return *this;
		}

		Vect3 operator-() const noexcept {
			return Vect3(-x, -y, -z);
		}

		Vect3 operator+(const Vect3& other) const noexcept {
			return Vect3(x + other.x, y + other.y, z + other.z);
		}

		Vect3 operator-(const Vect3& other) const noexcept {
			return Vect3(x - other.x, y - other.y, z - other.z);
		}

		Vect3 operator*(double other) const noexcept {
			return Vect3(x * other, y * other, z * other);
		}

		double operator*(const Vect3& other) const noexcept {
			return x * other.x + y * other.y + z * other.z;
		}

		Vect3 operator/(double other) const {
			if (equality(other, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
			}

			return Vect3(x / other, y / other, z / other);
		}

		Vect3 operator^(const Vect3& other) const noexcept {
			return Vect3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
		}

		Vect3 operator^(double other) const {
			if (x < 0 || y < 0 || z < 0) {
				throw EngDomainError(__FILE__, __LINE__, "operator^, raising a negative number to a power.\n\n");
			}

			return Vect3(pow(x, other), pow(y, other), pow(z, other));
		}

		Vect3 operator^(uint32_t other) const noexcept {
			return Vect3(binary_exponentiation(x, other), binary_exponentiation(y, other), binary_exponentiation(z, other));
		}

		template <typename T>
		T get_value(T value, std::function<void(double, T*)> func) const {
			func(x, &value);
			func(y, &value);
			func(z, &value);
			return value;
		}

		double length_sqr() const noexcept {
			return *this * *this;
		}

		double length() const noexcept {
			return sqrt(*this * *this);
		}

		Vect3 normalize() const {
			double vect_length = length();
			if (equality(vect_length, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "normalize, null vector normalization.\n\n");
			}

			return *this / vect_length;
		}

		Vect3 horizont() const noexcept {
			Vect3 horizont_vect(1, 0, 0);
			if (!equality(Vect3(z, 0, -x).length(), 0.0, eps_)) {
				horizont_vect = Vect3(z, 0, -x).normalize();
			}

			return horizont_vect;
		}

		Vect3 reflect_vect(const Vect3& n) const {
			if (equality(n.length(), 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "reflect_vect, the normal vector has zero length.\n\n");
			}

			Vect3 norm = n.normalize();
			return norm * (norm * *this) * 2 - *this;
		}

		Vect3 symmetry(const Vect3& center) const noexcept {
			return (center - *this) * 2 + *this;
		}

		bool in_two_side_angle(const Vect3& v1, const Vect3& v2) const noexcept {
			try {
				return equality(cos_angle(v1 ^ *this, v2 ^ *this), -1.0, eps_);
			}
			catch (EngDomainError) {
				return false;
			}
		}

		bool in_angle(const Vect3& v1, const Vect3& v2) const noexcept {
			Vect3 prod1 = v1 ^ *this, prod2 = v2 ^ *this, prod3 = v1 ^ v2;
			if (equality(prod1.length() * prod3.length(), 0.0, eps_)) {
				return false;
			}

			try {
				if (equality(prod1.length() * prod2.length(), 0.0, eps_)) {
					return equality(cos_angle(*this, v1), 1.0, eps_) || equality(cos_angle(*this, v2), 1.0, eps_);
				}
				return equality(cos_angle(prod1, prod2), -1.0, eps_) && equality(cos_angle(prod1, prod3), 1.0, eps_);
			}
			catch (EngDomainError) {
				return false;
			}
		}

		bool in_triangle(const Vect3& v1, const Vect3& v2, const Vect3& v3) const noexcept {
			return (*this - v1).in_angle(v2 - v1, v3 - v1) && (*this - v2).in_angle(v1 - v2, v3 - v2);
		}

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}

		static double cos_angle(const Vect3& v1, const Vect3& v2) {
			double length_prod = v1.length() * v2.length();
			if (equality(length_prod, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "cos_angle, one of the vectors has zero length.\n\n");
			}

			return (v1 * v2) / length_prod;
		}

		static double sin_angle(const Vect3& v1, const Vect3& v2) {
			double length_prod = v1.length() * v2.length();
			if (equality(length_prod, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "sin_angle, one of the vectors has zero length.\n\n");
			}

			return (v1 ^ v2).length() / length_prod;
		}

		static Vect3 zip_map(const Vect3& v1, const Vect3& v2, std::function<double(double, double)> zip_func) {
			return Vect3(zip_func(v1.x, v2.x), zip_func(v1.y, v2.y), zip_func(v1.z, v2.z));
		}

		template <typename T>  // Casts required: double(T)
		static std::vector<Vect3> move_in(size_t size, T* data) {
			std::vector<Vect3> result(size);
			for (size_t i = 0; i < size; ++i) {
				result[i] = Vect3(static_cast<double>(data[3 * i]), static_cast<double>(data[3 * i + 1]), static_cast<double>(data[3 * i + 2]));
			}

			delete[] data;
			return result;
		}
	};

	std::istream& operator>>(std::istream& fin, Vect3& vector) noexcept {
		fin >> vector.x >> vector.y >> vector.z;
		return fin;
	}

	std::ostream& operator<<(std::ostream& fout, const Vect3& vector) noexcept {
		fout << '(' << vector.x << ", " << vector.y << ", " << vector.z << ')';
		return fout;
	}

	Vect3 operator*(double value, const Vect3& vector) noexcept {
		return Vect3(vector.x * value, vector.y * value, vector.z * value);
	}
}

template <>
struct std::hash<eng::Vect3> {
	size_t operator()(const eng::Vect3& vector) const noexcept {
		size_t result = 0;
		eng::hash_combine(result, vector.x);
		eng::hash_combine(result, vector.y);
		eng::hash_combine(result, vector.z);
		return result;
	}
};
