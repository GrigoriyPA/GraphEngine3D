#pragma once

#include <math.h>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include "Vect2.h"


const double PI = acos(-1), FI = (sqrtl(5) - 1.0) / 2.0;


class Vect3 {
	double eps = 0.00001;

public:
	double x, y, z;

	Vect3(double x = 0, double y = 0, double z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vect3(std::vector < double > init) {
		x = 0;
		y = 0;
		z = 0;
		for (int i = 0; i < std::min(3, (int)init.size()); i++)
			(*this)[i] = init[i];
	}

	Vect3(sf::Color color) {
		x = color.r;
		y = color.g;
		z = color.b;
	}

	double& operator[](int index) {
		if (index == 0)
			return x;
		if (index == 1)
			return y;
		if (index == 2)
			return z;

		std::cout << "ERROR::VECT3::OPERATOR[]\n" << "Invalid index.\n";
		assert(0);
	}

	bool operator ==(Vect3 other) {
		return abs(x - other.x) < eps && abs(y - other.y) < eps && abs(z - other.z) < eps;
	}

	bool operator !=(Vect3 other) {
		return !(*this == other);
	}

	void operator +=(Vect3 other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void operator -=(Vect3 other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}

	void operator *=(double other) {
		x *= other;
		y *= other;
		z *= other;
	}

	void operator /=(double other) {
		if (abs(other) < eps) {
			std::cout << "ERROR::VECT3::OPERATOR/=_DOUBLE\n" << "Division by zero.\n";
			assert(0);
		}

		x /= other;
		y /= other;
		z /= other;
	}

	Vect3 operator -() {
		return Vect3(-x, -y, -z);
	}

	Vect3 operator +(Vect3 other) {
		return Vect3(x + other.x, y + other.y, z + other.z);
	}

	Vect3 operator -(Vect3 other) {
		return Vect3(x - other.x, y - other.y, z - other.z);
	}

	Vect3 operator *(double other) {
		return Vect3(x * other, y * other, z * other);
	}

	double operator *(Vect3 other) {
		return x * other.x + y * other.y + z * other.z;
	}

	Vect3 operator ^(Vect3 other) {
		return Vect3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	Vect3 operator ^(double other) {
		if (x < 0 || y < 0 || z < 0) {
			std::cout << "ERROR::VECT3::OPERATOR^_DOUBLE\n" << "Raising a negative number to a power.\n";
			assert(0);
		}

		return Vect3(pow(x, other), pow(y, other), pow(z, other));
	}

	Vect3 operator /(double other) {
		if (abs(other) < eps) {
			std::cout << "ERROR::VECT3::OPERATOR/_DOUBLE\n" << "Division by zero.\n";
			assert(0);
		}

		return Vect3(x / other, y / other, z / other);
	}

	double length_sqr() {
		return *this * *this;
	}

	double length() {
		return sqrt(*this * *this);
	}

	Vect3 normalize() {
		double length = (*this).length();

		if (length < eps)
			return Vect3(1, 0, 0);

		return *this / length;
	}

	Vect3 horizont() {
		Vect3 horizont_vect(1, 0, 0);
		if (Vect3(-z, 0, x).length() >= eps)
			horizont_vect = Vect3(-z, 0, x).normalize();

		return horizont_vect;
	}

	double cos_angle(Vect3 v) {
		double prod = (*this).length() * v.length();

		if (prod < eps) {
			std::cout << "ERROR::VECT3::COS_ANGLE\n" << "One of the vectors has zero length.\n";
			assert(0);
		}

		return (*this * v) / prod;
	}

	double sin_angle(Vect3 v) {
		double prod = (*this).length() * v.length();

		if (prod < eps) {
			std::cout << "ERROR::VECT3::SIN_ANGLE\n" << "One of the vectors has zero length.\n";
			assert(0);
		}

		return (*this ^ v).length() / prod;
	}

	Vect3 reflect_vect(Vect3 n) {
		if (n.length() < eps) {
			std::cout << "ERROR::VECT3::REFLECT_VECT\n" << "The normal vector has zero length.\n";
			assert(0);
		}

		n = n.normalize();

		return n * (n * *this) * 2 - *this;
	}

	Vect3 symmetry(Vect3 center) {
		return (center - *this) * 2 + *this;
	}

	Vect3 set_max(Vect3 v) {
		x = std::max(x, v.x);
		y = std::max(y, v.y);
		z = std::max(z, v.z);

		return *this;
	}

	Vect3 set_min(Vect3 v) {
		x = std::min(x, v.x);
		y = std::min(y, v.y);
		z = std::min(z, v.z);

		return *this;
	}

	bool in_angle(Vect3 v1, Vect3 v2) {
		Vect3 prod1 = v1 ^ *this, prod2 = v2 ^ *this, prod3 = v1 ^ v2;

		if (prod1.length() * prod2.length() < eps)
			return true;

		if (prod1.length() * prod3.length() < eps)
			return false;

		return abs(prod1.cos_angle(prod2) + 1) < eps && abs(prod1.cos_angle(prod3) - 1) < eps;
	}

	bool in_triangle(Vect3 v1, Vect3 v2, Vect3 v3) {
		return (*this - v1).in_angle(v2 - v1, v3 - v1) && (*this - v2).in_angle(v1 - v2, v3 - v2);
	}

	float* value_ptr() {
		return new float[3]{ (float)x, (float)y, (float)z };
	}

	std::vector < double > value_vector() {
		return { x, y, z };
	}

	std::string value_string() {
		return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
	}

	void print() {
		std::cout << '(' << x << ", " << y << ", " << z << ")\n";
	}
};

Vect3 operator *(double left, Vect3 right) {
	return Vect3(right.x * left, right.y * left, right.z * left);
}
