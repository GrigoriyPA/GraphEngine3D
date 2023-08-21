#include "Vec2.h"


// Vec2
namespace gre {
    // Constructors
	Vec2::Vec2() noexcept {
	}

    Vec2::Vec2(double value) noexcept
        : x(value)
        , y(value)
    {}

    Vec2::Vec2(double x, double y) noexcept
        : x(x)
        , y(y)
    {}

    Vec2::Vec2(const sf::Vector2f& init) noexcept
        : x(init.x)
        , y(init.y)
    {}

    Vec2::Vec2(const aiVector2D& init) noexcept
        : x(init.x)
        , y(init.y)
    {}

    // Operators
    Vec2::operator sf::Vector2f() const {
        return sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
    }

    double& Vec2::operator[](size_t index) {
        GRE_ENSURE(index <= 1, GreOutOfRange, "index out of range");

        return index == 0 ? x : y;
    }

    const double& Vec2::operator[](size_t index) const {
        GRE_ENSURE(index <= 1, GreOutOfRange, "index out of range");

        return index == 0 ? x : y;
    }

    bool Vec2::operator==(const Vec2& other) const noexcept {
        return equality(x, other.x) && equality(y, other.y);
    }

    bool Vec2::operator!=(const Vec2& other) const noexcept {
        return !equality(x, other.x) || !equality(y, other.y);
    }

    Vec2& Vec2::operator+=(const Vec2& other)& noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& Vec2::operator-=(const Vec2& other)& noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& Vec2::operator*=(double other)& noexcept {
        x *= other;
        y *= other;
        return *this;
    }

    Vec2& Vec2::operator/=(double other)& {
        GRE_CHECK(!equality(other, 0.0), "division by zero");

        x /= other;
        y /= other;
        return *this;
    }

    Vec2& Vec2::operator^=(double other)& {
        GRE_CHECK(x >= 0.0 && y >= 0.0, "raising a negative number to a power");

        x = std::pow(x, other);
        y = std::pow(y, other);
        return *this;
    }

    Vec2 Vec2::operator-() const noexcept {
        return Vec2(-x, -y);
    }

    Vec2 Vec2::operator+(const Vec2& other) const noexcept {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 Vec2::operator-(const Vec2& other) const noexcept {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 Vec2::operator*(double other) const noexcept {
        return Vec2(x * other, y * other);
    }

    double Vec2::operator*(const Vec2& other) const noexcept {
        return x * other.x + y * other.y;
    }

    double Vec2::operator^(const Vec2& other) const noexcept {
        return x * other.y - y * other.x;
    }

    Vec2 Vec2::operator^(double other) const {
        GRE_CHECK(x >= 0.0 && y >= 0.0, "raising a negative number to a power");

        return Vec2(std::pow(x, other), std::pow(y, other));
    }

    Vec2 Vec2::operator/(double other) const {
        GRE_CHECK(!equality(other, 0.0), "division by zero");

        return Vec2(x / other, y / other);
    }

    // Math functions
    double Vec2::length() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 Vec2::normalize() const {
        double vect_length = length();

        GRE_CHECK(!equality(vect_length, 0.0), "null vector normalization");

        return *this / vect_length;
    }

    // External operators
    std::ostream& operator<<(std::ostream& fout, const Vec2& vector) {
        fout << '(' << vector.x << ", " << vector.y << ')';
        return fout;
    }

    Vec2 operator*(double value, const Vec2& vector) noexcept {
        return Vec2(vector.x * value, vector.y * value);
    }
}  // namespace gre
