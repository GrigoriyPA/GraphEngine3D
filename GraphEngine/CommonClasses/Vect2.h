#pragma once

#include <math.h>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>


int sign(double x) {
	if (x > 0)
		return 1;
	if (x < 0)
		return -1;
	return 0;
}


std::vector < std::string > split(std::string s) {
	std::vector < std::string > result(1);
	for (char el : s) {
		if (result.back().size() > 0 && (el == ' ' || el == '\n'))
			result.push_back("");
		else if (el != ' ' && el != '\n')
			result.back() += el;
	}

	return result;
}


class Vect2 {
	double eps = 0.00001;

public:
	double x, y;

	Vect2(double x = 0, double y = 0) {
		this->x = x;
		this->y = y;
	}

	Vect2(std::vector < double > init) {
		x = 0;
		y = 0;
		for (int i = 0; i < std::min(2, (int)init.size()); i++)
			(*this)[i] = init[i];
	}

	Vect2(sf::Vector2f init) {
		x = init.x;
		y = init.y;
	}

	double& operator[](int index) {
		if (index == 0)
			return x;
		if (index == 1)
			return y;

		std::cout << "ERROR::VECT2::OPERATOR[]\n" << "Invalid index.\n";
		assert(0);
	}

	bool operator ==(Vect2 other) {
		return abs(x - other.x) < eps && abs(y - other.y) < eps;
	}

	bool operator !=(Vect2 other) {
		return !(*this == other);
	}

	void operator +=(Vect2 other) {
		x += other.x;
		y += other.y;
	}

	void operator -=(Vect2 other) {
		x -= other.x;
		y -= other.y;
	}

	void operator *=(double other) {
		x *= other;
		y *= other;
	}

	void operator /=(double other) {
		if (abs(other) < eps) {
			std::cout << "ERROR::VECT2::OPERATOR/=_DOUBLE\n" << "Division by zero.\n";
			assert(0);
		}

		x /= other;
		y /= other;
	}

	Vect2 operator -() {
		return Vect2(-x, -y);
	}

	Vect2 operator +(Vect2 other) {
		return Vect2(x + other.x, y + other.y);
	}

	Vect2 operator -(Vect2 other) {
		return Vect2(x - other.x, y - other.y);
	}

	Vect2 operator *(double other) {
		return Vect2(x * other, y * other);
	}

	double operator *(Vect2 other) {
		return x * other.x + y * other.y;
	}

	double operator ^(Vect2 other) {
		return x * other.y - y * other.x;
	}

	Vect2 operator ^(double other) {
		if (x < 0 || y < 0) {
			std::cout << "ERROR::VECT2::OPERATOR^_DOUBLE\n" << "Raising a negative number to a power.\n";
			assert(0);
		}

		return Vect2(pow(x, other), pow(y, other));
	}

	Vect2 operator /(double other) {
		if (abs(other) < eps) {
			std::cout << "ERROR::VECT2::OPERATOR/_DOUBLE\n" << "Division by zero.\n";
			assert(0);
		}

		return Vect2(x / other, y / other);
	}

	double length_sqr() {
		return *this * *this;
	}

	double length() {
		return sqrt(*this * *this);
	}

	Vect2 normalize() {
		double length = (*this).length();

		if (length < eps)
			return Vect2(1, 0);

		return *this / length;
	}

	double cos_angle(Vect2 v) {
		double prod = (*this).length() * v.length();

		if (prod < eps) {
			std::cout << "ERROR::VECT2::COS_ANGLE\n" << "One of the vectors has zero length.\n";
			assert(0);
		}

		return (*this * v) / prod;
	}

	double sin_angle(Vect2 v) {
		double prod = (*this).length() * v.length();

		if (prod < eps) {
			std::cout << "ERROR::VECT2::SIN_ANGLE\n" << "One of the vectors has zero length.\n";
			assert(0);
		}

		return (*this ^ v) / prod;
	}

	Vect2 reflect_vect(Vect2 n) {
		if (n.length() < eps) {
			std::cout << "ERROR::VECT2::REFLECT_VECT\n" << "The normal vector has zero length.\n";
			assert(0);
		}

		n = n.normalize();

		return n * (n * *this) * 2 - *this;
	}

	Vect2 set_max(Vect2 v) {
		x = std::max(x, v.x);
		y = std::max(y, v.y);

		return *this;
	}

	Vect2 set_min(Vect2 v) {
		x = std::min(x, v.x);
		y = std::min(y, v.y);

		return *this;
	}

	bool in_angle(Vect2 v1, Vect2 v2) {
		int sign1 = sign(v1 ^ *this), sign2 = sign(v2 ^ *this), sign3 = sign(v1 ^ v2);

		return (sign1 == 0 || sign1 == sign3) && !(sign2 == 0 || sign2 == -sign3);
	}

	bool in_triangle(Vect2 v1, Vect2 v2, Vect2 v3) {
		return (*this - v1).in_angle(v2 - v1, v3 - v1) && (*this - v2).in_angle(v1 - v2, v3 - v2);
	}

	float* value_ptr() {
		return new float[3]{ (float)x, (float)y };
	}

	std::vector < double > value_vector() {
		return { x, y };
	}

	sf::Vector2f value_vector2f() {
		return sf::Vector2f(x, y);
	}

	std::string value_string() {
		return std::to_string(x) + " " + std::to_string(y);
	}

	void print() {
		std::cout << '(' << x << ", " << y << ")\n";
	}
};

Vect2 operator *(double left, Vect2 right) {
	return Vect2(right.x * left, right.y * left);
}

