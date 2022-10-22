#pragma once

#include <iostream>
#include <initializer_list>
#include <SFML/System/Vector2.hpp>
#include "Functions.h"


namespace eng {
    class Vect2 {
        inline static double eps_ = 1e-5;

    public:
        double x = 0;
        double y = 0;

        Vect2(double x, double y) noexcept {
            this->x = x;
            this->y = y;
        }

        template <typename T>  // Casts required: double(T)
        Vect2(const std::initializer_list<T>& init) {
            size_t comp_id = 0;
            for (const T& element : init) {
                (*this)[comp_id] = static_cast<double>(element);

                if (++comp_id == 2) {
                    break;
                }
            }
        }

        template <typename T>  // Casts required: double(T)
        explicit Vect2(const std::vector<T>& init) {
            for (size_t comp_id = 0; comp_id < std::min(static_cast<size_t>(2), init.size()); ++comp_id) {
                (*this)[comp_id] = static_cast<double>(init[comp_id]);
            }
        }

        explicit Vect2(const sf::Vector2f& init) noexcept {
            x = init.x;
            y = init.y;
        }

        Vect2() noexcept {
        }

        template <typename T>  // Constructors required: T(T)
        explicit operator std::vector<T>() const {
            return { T(x), T(y) };
        }

        explicit operator sf::Vector2f() const noexcept {
            return sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
        }

        explicit operator std::string() const noexcept {
            return std::to_string(x) + " " + std::to_string(y);
        }

        double& operator[](size_t index) {
            if (index == 0) {
                return x;
            }
            if (index == 1) {
                return y;
            }
            throw eng_exceptions::EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
        }

        const double& operator[](size_t index) const {
            if (index == 0) {
                return x;
            }
            if (index == 1) {
                return y;
            }
            throw eng_exceptions::EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
        }

        bool operator==(const Vect2& other) const noexcept {
            return equality(x, other.x, eps_) && equality(y, other.y, eps_);
        }

        bool operator!=(const Vect2& other) const noexcept {
            return !(*this == other);
        }

        Vect2& operator+=(const Vect2& other)& noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vect2& operator-=(const Vect2& other)& noexcept {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vect2& operator*=(double other)& noexcept {
            x *= other;
            y *= other;
            return *this;
        }

        Vect2& operator/=(double other)& {
            if (equality(other, 0.0, eps_)) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
            }

            x /= other;
            y /= other;
            return *this;
        }

        Vect2& operator^=(double other)& {
            if (x < 0 || y < 0) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "operator^=, raising a negative number to a power.\n\n");
            }

            x = pow(x, other);
            y = pow(y, other);
            return *this;
        }

        Vect2& operator^=(uint32_t other)& noexcept {
            x = binary_exponentiation(x, other);
            y = binary_exponentiation(y, other);
            return *this;
        }

        Vect2 operator-() const noexcept {
            return Vect2(-x, -y);
        }

        Vect2 operator+(const Vect2& other) const noexcept {
            return Vect2(x + other.x, y + other.y);
        }

        Vect2 operator-(const Vect2& other) const noexcept {
            return Vect2(x - other.x, y - other.y);
        }

        Vect2 operator*(double other) const noexcept {
            return Vect2(x * other, y * other);
        }

        double operator*(const Vect2& other) const noexcept {
            return x * other.x + y * other.y;
        }

        double operator^(const Vect2& other) const noexcept {
            return x * other.y - y * other.x;
        }

        Vect2 operator^(double other) const {
            if (x < 0 || y < 0) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "operator^, raising a negative number to a power.\n\n");
            }

            return Vect2(pow(x, other), pow(y, other));
        }

        Vect2 operator^(uint32_t other) const noexcept {
            return Vect2(binary_exponentiation(x, other), binary_exponentiation(y, other));
        }

        Vect2 operator/(double other) const {
            if (equality(other, 0.0, eps_)) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
            }

            return Vect2(x / other, y / other);
        }

        void set_epsilon(double eps)& {
            if (eps <= 0) {
                throw eng_exceptions::EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
            }

            eps_ = eps;
        }

        double length_sqr() const noexcept {
            return *this * *this;
        }

        double length() const noexcept {
            return sqrt(*this * *this);
        }

        Vect2 normalize() const {
            double vect_length = length();
            if (equality(vect_length, 0.0, eps_)) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "normalize, null vector normalization.\n\n");
            }

            return *this / vect_length;
        }

        Vect2 reflect_vect(const Vect2& n) const {
            if (equality(n.length(), 0.0, eps_)) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "reflect_vect, the normal vector has zero length.\n\n");
            }

            Vect2 norm = n.normalize();
            return norm * (norm * *this) * 2 - *this;
        }

        bool in_angle(const Vect2& v1, const Vect2& v2) const noexcept {
            int32_t sign1 = sgn(v1 ^ *this);
            int32_t sign2 = sgn(v2 ^ *this);
            int32_t sign3 = sgn(v1 ^ v2);
            return (sign1 == 0 || sign1 == sign3) && !(sign2 == 0 || sign2 == -sign3);
        }

        bool in_triangle(const Vect2& v1, const Vect2& v2, const Vect2& v3) const noexcept {
            return (*this - v1).in_angle(v2 - v1, v3 - v1) && (*this - v2).in_angle(v1 - v2, v3 - v2);
        }

        static double cos_angle(const Vect2& v1, const Vect2& v2) {
            double length_prod = v1.length() * v2.length();
            if (equality(length_prod, 0.0, eps_)) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "cos_angle, one of the vectors has zero length.\n\n");
            }

            return (v1 * v2) / length_prod;
        }

        static double sin_angle(const Vect2& v1, const Vect2& v2) {
            double length_prod = v1.length() * v2.length();
            if (equality(length_prod, 0.0, eps_)) {
                throw eng_exceptions::EngDomainError(__FILE__, __LINE__, "sin_angle, one of the vectors has zero length.\n\n");
            }

            return (v1 ^ v2) / length_prod;
        }

        static Vect2 zip_map(const Vect2& v1, const Vect2& v2, std::function<double(double, double)> zip_func) {
            return Vect2(zip_func(v1.x, v2.x), zip_func(v1.y, v2.y));
        }
    };

    std::istream& operator>>(std::istream& fin, Vect2& vector) noexcept {
        fin >> vector.x >> vector.y;
        return fin;
    }

    std::ostream& operator<<(std::ostream& fout, const Vect2& vector) noexcept {
        fout << '(' << vector.x << ", " << vector.y << ')';
        return fout;
    }

    Vect2 operator*(double value, const Vect2& vector) noexcept {
        return Vect2(vector.x * value, vector.y * value);
    }
}

template <>
struct std::hash<eng::Vect2> {
    size_t operator()(const eng::Vect2& vector) const noexcept {
        size_t result = 0;
        eng::hash_combine(result, vector.x);
        eng::hash_combine(result, vector.y);
        return result;
    }
};
