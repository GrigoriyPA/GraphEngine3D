#pragma once

#include <iostream>
#include <SFML/Graphics/Color.hpp>
#include "Functions.h"


namespace eng {
	class Vect3 {
		double eps_ = 1e-5;

	public:
		double x = 0, y = 0, z = 0;

		Vect3(double x, double y, double z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		explicit Vect3(const std::vector<double>& init) {
			for (size_t comp_id = 0; comp_id < std::min(static_cast<size_t>(3), init.size()); comp_id++) {
				(*this)[comp_id] = init[comp_id];
			}
		}

		explicit Vect3(const sf::Color& color) {
			x = color.r;
			y = color.g;
			z = color.b;
		}

		Vect3() {
		}

		explicit operator std::vector<double>() const {
			return { x, y, z };
		}

		explicit operator std::string() const {
			return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
		}

		// In case of an error returns a z
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

			std::cout << "ERROR::VECT3::OPERATOR[](SIZE_T)\n" << "Invalid index.\n\n";
			return z;
		}

		bool operator ==(const Vect3& other) const {
			return equality(x, other.x, eps_) && equality(y, other.y, eps_) && equality(z, other.z, eps_);
		}

		bool operator !=(const Vect3& other) const {
			return !(*this == other);
		}

		void operator +=(const Vect3& other) {
			x += other.x;
			y += other.y;
			z += other.z;
		}

		void operator -=(const Vect3& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
		}

		void operator *=(double other) {
			x *= other;
			y *= other;
			z *= other;
		}

		// In case of an error skips operation
		void operator /=(double other) & {
			if (equality(other, 0.0, eps_)) {
				std::cout << "ERROR::VECT3::OPERATOR/=(DOUBLE)\n" << "Division by zero.\n\n";
				return;
			}

			x /= other;
			y /= other;
			z /= other;
		}

		// In case of an error skips operation
		void operator ^=(double other) & {
			if (x < 0 || y < 0 || z < 0) {
				std::cout << "ERROR::VECT3::OPERATOR^=(DOUBLE)\n" << "Raising a negative number to a power.\n\n";
				return;
			}

			x = pow(x, other);
			y = pow(y, other);
			z = pow(z, other);
		}

		Vect3 operator -() const {
			return Vect3(-x, -y, -z);
		}

		Vect3 operator +(const Vect3& other) const {
			return Vect3(x + other.x, y + other.y, z + other.z);
		}

		Vect3 operator -(const Vect3& other) const {
			return Vect3(x - other.x, y - other.y, z - other.z);
		}

		Vect3 operator *(double other) const {
			return Vect3(x * other, y * other, z * other);
		}

		double operator *(const Vect3& other) const {
			return x * other.x + y * other.y + z * other.z;
		}

		Vect3 operator ^(const Vect3& other) const {
			return Vect3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
		}

		// In case of an error skips operation
		Vect3 operator ^(double other) const {
			if (x < 0 || y < 0 || z < 0) {
				std::cout << "ERROR::VECT3::OPERATOR^(DOUBLE)\n" << "Raising a negative number to a power.\n\n";
				return *this;
			}

			return Vect3(pow(x, other), pow(y, other), pow(z, other));
		}

		// In case of an error skips operation
		Vect3 operator /(double other) const {
			if (equality(other, 0.0, eps_)) {
				std::cout << "ERROR::VECT3::OPERATOR/(DOUBLE)\n" << "Division by zero.\n\n";
				return *this;
			}

			return Vect3(x / other, y / other, z / other);
		}

		// In case of an error returns a z
		double at(size_t index) const {
			if (index == 0) {
				return x;
			}
			if (index == 1) {
				return y;
			}
			if (index == 2) {
				return z;
			}

			std::cout << "ERROR::VECT3::AT\n" << "Invalid index.\n\n";
			return z;
		}

		double length_sqr() const {
			return *this * *this;
		}

		double length() const {
			return sqrt(*this * *this);
		}

		// In case of an error normalize to (1, 0, 0)
		void normalize() & {
			double vect_length = length();

			if (equality(vect_length, 0.0, eps_)) {
				std::cout << "ERROR::VECT3::NORMALIZE\n" << "Null vector normalization.\n\n";
				*this = Vect3(1, 0, 0);
			} else {
				*this /= vect_length;
			}
		}

		// In case of an error normalize to (1, 0, 0)
		Vect3 normalized() const {
			double vect_length = length();

			if (equality(vect_length, 0.0, eps_)) {
				std::cout << "ERROR::VECT3::NORMALIZED\n" << "Null vector normalization.\n\n";
				return Vect3(1, 0, 0);
			}

			return *this / vect_length;
		}

		// In case of an vertical vector returns (1, 0, 0)
		Vect3 horizont() const {
			Vect3 horizont_vect(1, 0, 0);
			if (equality(Vect3(z, 0, -x).length(), 0.0, eps_)) {
				horizont_vect = Vect3(z, 0, -x).normalized();
			}

			return horizont_vect;
		}

		// In case of an error returns cos(0)
		double cos_angle(const Vect3& v) const {
			double length_prod = length() * v.length();

			if (equality(length_prod, 0.0, eps_)) {
				std::cout << "ERROR::VECT3::COS_ANGLE\n" << "One of the vectors has zero length.\n\n";
				return 1;
			}

			return (*this * v) / length_prod;
		}

		// In case of an error returns sin(0)
		double sin_angle(const Vect3& v) const {
			double length_prod = length() * v.length();

			if (equality(length_prod, 0.0, eps_)) {
				std::cout << "ERROR::VECT3::SIN_ANGLE\n" << "One of the vectors has zero length.\n\n";
				return 0;
			}

			return (*this ^ v).length() / length_prod;
		}

		// In case of an error skips operation
		Vect3 reflect_vect(const Vect3& n) const {
			if (equality(n.length(), 0.0, eps_)) {
				std::cout << "ERROR::VECT3::REFLECT_VECT\n" << "The normal vector has zero length.\n\n";
				return *this;
			}

			Vect3 norm = n.normalized();
			return norm * (norm * *this) * 2 - *this;
		}

		Vect3 symmetry(const Vect3& center) const {
			return (center - *this) * 2 + *this;
		}

		bool in_angle(const Vect3& v1, const Vect3& v2) const {
			Vect3 prod1 = v1 ^ *this, prod2 = v2 ^ *this, prod3 = v1 ^ v2;

			if (equality(prod1.length() * prod3.length(), 0.0, eps_)) {
				return false;
			}

			if (equality(prod1.length() * prod2.length(), 0.0, eps_)) {
				return equality(cos_angle(v1), 1.0, eps_) || equality(cos_angle(v2), 1.0, eps_);
			}

			return equality(prod1.cos_angle(prod2), -1.0, eps_) && equality(prod1.cos_angle(prod3), 1.0, eps_);
		}

		bool in_triangle(const Vect3& v1, const Vect3& v2, const Vect3& v3) const {
			return (*this - v1).in_angle(v2 - v1, v3 - v1) && (*this - v2).in_angle(v1 - v2, v3 - v2);
		}

		float* data_f() const {
			return new float[3]{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
		}

		double* data_d() const {
			return new double[3]{ x, y, z };
		}

		static Vect3 get_max(const Vect3& v1, const Vect3& v2) {
			return Vect3(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z));
		}

		static Vect3 get_min(const Vect3& v1, const Vect3& v2) {
			return Vect3(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z));
		}
	};

	std::istream& operator>>(std::istream& fin, Vect3& vector) {
		fin >> vector.x >> vector.y >> vector.z;
		return fin;
	}

	std::ostream& operator<<(std::ostream& fout, const Vect3& vector) {
		fout << '(' << vector.x << ", " << vector.y << ", " << vector.z << ')';
		return fout;
	}

	Vect3 operator *(double left, const Vect3& right) {
		return Vect3(right.x * left, right.y * left, right.z * left);
	}
}
