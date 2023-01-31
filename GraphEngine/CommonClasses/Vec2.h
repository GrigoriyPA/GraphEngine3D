#pragma once

#include <initializer_list>
#include <iostream>
#include <SFML/System.hpp>
#include "Functions.h"


namespace gre {
    class Vec2 {
    public:
        double x = 0.0;
        double y = 0.0;

        Vec2() noexcept {
        }

        explicit Vec2(double value) noexcept {
            x = value;
            y = value;
        }

        Vec2(double x, double y) noexcept {
            this->x = x;
            this->y = y;
        }

        template <typename T>  // Casts required: double(T)
        Vec2(const std::initializer_list<T>& init) {
#ifdef _DEBUG
            if (init.size() != 2) {
                throw GreInvalidArgument(__FILE__, __LINE__, "Vec2, invalid initializer list size.\n\n");
            }
#endif // _DEBUG

            size_t comp_id = 0;
            for (const T& element : init) {
                (*this)[comp_id++] = static_cast<double>(element);
            }
        }

        explicit Vec2(const sf::Vector2f& init) noexcept {
            x = init.x;
            y = init.y;
        }

        explicit operator sf::Vector2f() const {
            return sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
        }

        double& operator[](size_t index) {
            if (index == 0) {
                return x;
            }

#ifdef _DEBUG
            if (index == 1) {
                return y;
            }
            throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
#endif // _DEBUG

            return y;
        }

        const double& operator[](size_t index) const {
            if (index == 0) {
                return x;
            }

#ifdef _DEBUG
            if (index == 1) {
                return y;
            }
            throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
#endif // _DEBUG

            return y;
        }

        bool operator==(const Vec2& other) const noexcept {
            return equality(x, other.x) && equality(y, other.y);
        }

        bool operator!=(const Vec2& other) const noexcept {
            return !equality(x, other.x) || !equality(y, other.y);
        }

        Vec2& operator+=(const Vec2& other)& noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2& operator-=(const Vec2& other)& noexcept {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2& operator*=(double other)& noexcept {
            x *= other;
            y *= other;
            return *this;
        }

        Vec2& operator/=(double other)& {
#ifdef _DEBUG
            if (equality(other, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
            }
#endif // _DEBUG

            x /= other;
            y /= other;
            return *this;
        }

        Vec2& operator^=(double other)& {
#ifdef _DEBUG
            if (x < 0 || y < 0) {
                throw GreDomainError(__FILE__, __LINE__, "operator^=, raising a negative number to a power.\n\n");
            }
#endif // _DEBUG

            x = std::pow(x, other);
            y = std::pow(y, other);
            return *this;
        }

        Vec2 operator-() const noexcept {
            return Vec2(-x, -y);
        }

        Vec2 operator+(const Vec2& other) const noexcept {
            return Vec2(x + other.x, y + other.y);
        }

        Vec2 operator-(const Vec2& other) const noexcept {
            return Vec2(x - other.x, y - other.y);
        }

        Vec2 operator*(double other) const noexcept {
            return Vec2(x * other, y * other);
        }

        double operator*(const Vec2& other) const noexcept {
            return x * other.x + y * other.y;
        }

        double operator^(const Vec2& other) const noexcept {
            return x * other.y - y * other.x;
        }

        Vec2 operator^(double other) const {
#ifdef _DEBUG
            if (x < 0 || y < 0) {
                throw GreDomainError(__FILE__, __LINE__, "operator^, raising a negative number to a power.\n\n");
            }
#endif // _DEBUG

            return Vec2(pow(x, other), pow(y, other));
        }

        Vec2 operator/(double other) const {
#ifdef _DEBUG
            if (equality(other, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
            }
#endif // _DEBUG

            return Vec2(x / other, y / other);
        }

        double length() const {
            return std::sqrt(*this * *this);
        }

        Vec2 normalize() const {
            double vect_length = length();

#ifdef _DEBUG
            if (equality(vect_length, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "normalize, null vector normalization.\n\n");
            }
#endif // _DEBUG

            return *this / vect_length;
        }

        Vec2 reflect_vect(const Vec2& n) const {
#ifdef _DEBUG
            if (equality(n.length(), 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "reflect_vect, the normal vector has zero length.\n\n");
            }
#endif // _DEBUG

            const Vec2& norm = n.normalize();
            return norm * (norm * *this) * 2 - *this;
        }

        static double cos_angle(const Vec2& v1, const Vec2& v2) {
            double length_prod = v1.length() * v2.length();

#ifdef _DEBUG
            if (equality(length_prod, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "cos_angle, one of the vectors has zero length.\n\n");
            }
#endif // _DEBUG

            return (v1 * v2) / length_prod;
        }

        static double sin_angle(const Vec2& v1, const Vec2& v2) {
            double length_prod = v1.length() * v2.length();

#ifdef _DEBUG
            if (equality(length_prod, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "sin_angle, one of the vectors has zero length.\n\n");
            }
#endif // _DEBUG

            return (v1 ^ v2) / length_prod;
        }
    };

    std::ostream& operator<<(std::ostream& fout, const Vec2& vector) noexcept {
        fout << '(' << vector.x << ", " << vector.y << ')';
        return fout;
    }

    Vec2 operator*(double value, const Vec2& vector) noexcept {
        return Vec2(vector.x * value, vector.y * value);
    }
}

template <>
struct std::hash<gre::Vec2> {
    size_t operator()(const gre::Vec2& vector) const noexcept {
        size_t result = 0;
        gre::hash_combine(result, vector.x);
        gre::hash_combine(result, vector.y);
        return result;
    }
};
