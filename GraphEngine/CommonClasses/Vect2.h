#pragma once

#include <iostream>
#include <SFML/System/Vector2.hpp>
#include "Functions.h"


namespace eng {
    class Vect2 {
        double eps_ = 1e-5;

    public:
        double x = 0, y = 0;

        Vect2(double x, double y) {
            this->x = x;
            this->y = y;
        }

        explicit Vect2(const std::vector<double>& init) {
            for (size_t comp_id = 0; comp_id < std::min(static_cast<size_t>(2), init.size()); ++comp_id) {
                (*this)[comp_id] = init[comp_id];
            }
        }

        explicit Vect2(const sf::Vector2f& init) {
            x = init.x;
            y = init.y;
        }

        Vect2() {
        }

        explicit operator std::vector<double>() const {
            return { x, y };
        }

        explicit operator sf::Vector2f() const {
            return sf::Vector2f(x, y);
        }

        explicit operator std::string() const {
            return std::to_string(x) + " " + std::to_string(y);
        }

        // In case of an error returns a y
        double& operator[](size_t index) {
            if (index == 0) {
                return x;
            }
            if (index == 1) {
                return y;
            }

            std::cout << "ERROR::VECT2::OPERATOR[](SIZE_T)\n" << "Invalid index.\n\n";
            return y;
        }

        bool operator ==(const Vect2& other) const {
            return equality(x, other.x, eps_) && equality(y, other.y, eps_);
        }

        bool operator !=(const Vect2& other) const {
            return !(*this == other);
        }

        void operator +=(const Vect2& other) {
            x += other.x;
            y += other.y;
        }

        void operator -=(const Vect2& other) {
            x -= other.x;
            y -= other.y;
        }

        void operator *=(double other) {
            x *= other;
            y *= other;
        }

        // In case of an error skips operation
        void operator /=(double other) & {
            if (equality(other, 0.0, eps_)) {
                std::cout << "ERROR::VECT2::OPERATOR/=(DOUBLE)\n" << "Division by zero.\n\n";
                return;
            }

            x /= other;
            y /= other;
        }

        // In case of an error skips operation
        void operator ^=(double other) & {
            if (x < 0 || y < 0) {
                std::cout << "ERROR::VECT2::OPERATOR^=(DOUBLE)\n" << "Raising a negative number to a power.\n\n";
                return;
            }

            x = pow(x, other);
            y = pow(y, other);
        }

        Vect2 operator -() const {
            return Vect2(-x, -y);
        }

        Vect2 operator +(const Vect2& other) const {
            return Vect2(x + other.x, y + other.y);
        }

        Vect2 operator -(const Vect2& other) const {
            return Vect2(x - other.x, y - other.y);
        }

        Vect2 operator *(double other) const {
            return Vect2(x * other, y * other);
        }

        double operator *(const Vect2& other) const {
            return x * other.x + y * other.y;
        }

        double operator ^(const Vect2& other) const {
            return x * other.y - y * other.x;
        }

        // In case of an error skips operation
        Vect2 operator ^(double other) const {
            if (x < 0 || y < 0) {
                std::cout << "ERROR::VECT2::OPERATOR^(DOUBLE)\n" << "Raising a negative number to a power.\n\n";
                return *this;
            }

            return Vect2(pow(x, other), pow(y, other));
        }

        // In case of an error skips operation
        Vect2 operator /(double other) const {
            if (equality(other, 0.0, eps_)) {
                std::cout << "ERROR::VECT2::OPERATOR/(DOUBLE)\n" << "Division by zero.\n\n";
                return *this;
            }

            return Vect2(x / other, y / other);
        }

        // In case of an error returns a y
        double at(size_t index) const {
            if (index == 0) {
                return x;
            }
            if (index == 1) {
                return y;
            }

            std::cout << "ERROR::VECT2::AT\n" << "Invalid index.\n\n";
            return y;
        }

        double length_sqr() const {
            return *this * *this;
        }

        double length() const {
            return sqrt(*this * *this);
        }

        // In case of an error normalize to (1, 0)
        void normalize() & {
            double vect_length = length();

            if (equality(vect_length, 0.0, eps_)) {
                std::cout << "ERROR::VECT2::NORMALIZE\n" << "Null vector normalization.\n\n";
                *this = Vect2(1, 0);
            } else {
                *this /= vect_length;
            }
        }

        // In case of an error normalize to (1, 0)
        Vect2 normalized() const {
            double vect_length = length();

            if (equality(vect_length, 0.0, eps_)) {
                std::cout << "ERROR::VECT2::NORMALIZED\n" << "Null vector normalization.\n\n";
                return Vect2(1, 0);
            }

            return *this / vect_length;
        }

        // In case of an error returns cos(0)
        double cos_angle(const Vect2& v) const {
            double length_prod = length() * v.length();

            if (equality(length_prod, 0.0, eps_)) {
                std::cout << "ERROR::VECT2::COS_ANGLE\n" << "One of the vectors has zero length.\n\n";
                return 1;
            }

            return (*this * v) / length_prod;
        }

        // In case of an error returns sin(0)
        double sin_angle(const Vect2& v) const {
            double length_prod = length() * v.length();

            if (equality(length_prod, 0.0, eps_)) {
                std::cout << "ERROR::VECT2::SIN_ANGLE\n" << "One of the vectors has zero length.\n\n";
                return 0;
            }

            return (*this ^ v) / length_prod;
        }

        // In case of an error skips operation
        Vect2 reflect_vect(const Vect2& n) {
            if (equality(n.length(), 0.0, eps_)) {
                std::cout << "ERROR::VECT2::REFLECT_VECT\n" << "The normal vector has zero length.\n\n";
                return *this;
            }

            Vect2 norm = n.normalized();
            return norm * (norm * *this) * 2 - *this;
        }

        Vect2 get_max(const Vect2& v) const {
            return Vect2(std::max(x, v.x), std::max(y, v.y));
        }

        Vect2 get_min(const Vect2& v) const {
            return Vect2(std::min(x, v.x), std::min(y, v.y));
        }

        bool in_angle(const Vect2& v1, const Vect2& v2) const {
            int32_t sign1 = sign(v1 ^ *this);
            int32_t sign2 = sign(v2 ^ *this);
            int32_t sign3 = sign(v1 ^ v2);

            return (sign1 == 0 || sign1 == sign3) && !(sign2 == 0 || sign2 == -sign3);
        }

        bool in_triangle(const Vect2& v1, const Vect2& v2, const Vect2& v3) const {
            return (*this - v1).in_angle(v2 - v1, v3 - v1) && (*this - v2).in_angle(v1 - v2, v3 - v2);
        }

        float* data_f() const {
            return new float[2]{ static_cast<float>(x), static_cast<float>(y) };
        }

        double* data_d() const {
            return new double[2]{ x, y };
        }

        static Vect2 get_max(const Vect2& v1, const Vect2& v2) {
            return Vect2(std::max(v1.x, v2.x), std::max(v1.y, v2.y));
        }

        static Vect2 get_min(const Vect2& v1, const Vect2& v2) {
            return Vect2(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
        }
    };

    std::istream& operator>>(std::istream& fin, Vect2& vector) {
        fin >> vector.x >> vector.y;
        return fin;
    }

    std::ostream& operator<<(std::ostream& fout, const Vect2& vector) {
        fout << '(' << vector.x << ", " << vector.y << ')';
        return fout;
    }

    Vect2 operator *(double left, const Vect2& right) {
        return Vect2(right.x * left, right.y * left);
    }
}
