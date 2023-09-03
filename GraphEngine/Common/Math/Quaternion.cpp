#include "Quaternion.hpp"


// Quaternion
namespace gre {
    // Constructors
    Quaternion::Quaternion() noexcept {
    }

    Quaternion::Quaternion(double value) noexcept
        : x(value)
        , y(0.0)
        , u(0.0)
        , v(0.0)
    {}

    Quaternion::Quaternion(double real, const Vec3& imaginary) noexcept
        : x(real)
        , y(imaginary.x)
        , u(imaginary.y)
        , v(imaginary.z)
    {}

    Quaternion::Quaternion(double x, double y, double u, double v) noexcept
        : x(x)
        , y(y)
        , u(u)
        , v(v)
    {}

    // Operators
    double& Quaternion::operator[](size_t index) {
        GRE_ENSURE(index <= 3, GreOutOfRange, "index out of range");

        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return u;
            case 3: return v;
        }
    }

    const double& Quaternion::operator[](size_t index) const {
        GRE_ENSURE(index <= 3, GreOutOfRange, "index out of range");

        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return u;
            case 3: return v;
        }
    }

    bool Quaternion::operator==(const Quaternion& other) const noexcept {
        return equality(x, other.x) && equality(y, other.y) && equality(u, other.u) && equality(v, other.v);
    }

    bool Quaternion::operator!=(const Quaternion& other) const noexcept {
        return !equality(x, other.x) || !equality(y, other.y) || !equality(u, other.u) || !equality(v, other.v);
    }

    Quaternion& Quaternion::operator+=(const Quaternion& other)& noexcept {
        x += other.x;
        y += other.y;
        u += other.u;
        v += other.v;
        return *this;
    }

    Quaternion& Quaternion::operator-=(const Quaternion& other)& noexcept {
        x -= other.x;
        y -= other.y;
        u -= other.u;
        v -= other.v;
        return *this;
    }

    Quaternion& Quaternion::operator*=(const Quaternion& other)& noexcept {
        *this = *this * other;
        return *this;
    }

    Quaternion Quaternion::operator~() const noexcept {
        return Quaternion(x, -y, -u, -v);
    }

    Quaternion Quaternion::operator-() const noexcept {
        return Quaternion(-x, -y, -u, -v);
    }

    Quaternion Quaternion::operator+(const Quaternion& other) const noexcept {
        Quaternion result = *this;
        return result += other;
    }

    Quaternion Quaternion::operator-(const Quaternion& other) const noexcept {
        Quaternion result = *this;
        return result -= other;
    }

    Quaternion Quaternion::operator*(const Quaternion& other) const noexcept {
        return Quaternion(
            x * other.x - y * other.y - u * other.u - v * other.v,
            x * other.y + y * other.x + u * other.v - v * other.u,
            x * other.u + y * other.v + u * other.x - v * other.y,
            x * other.v + y * other.u - u * other.y + v * other.x
        );
    }

    // Math functions
    Vec3 Quaternion::get_imaginary() const noexcept {
        return Vec3(y, u, v);
    }

    Vec3 Quaternion::rotate(const Vec3& vector) const noexcept {
        return (*this * Quaternion(0.0, vector) * (~*this)).get_imaginary();
    }

    // Precalculated matrices
    Quaternion Quaternion::rotation_quaternion(const Vec3& axis, double angle) noexcept {
        GRE_CHECK(!equality(axis.length(), 0.0), "the axis vector has zero length");

        return Quaternion(cos(angle / 2.0), sin(angle / 2.0) * axis.normalize());
    }

    // External operators
    std::ostream& operator<<(std::ostream& fout, const Quaternion& quaternion) {
        fout << '(' << quaternion.x << ", " << quaternion.y << ", " << quaternion.u << ", " << quaternion.v << ')';
        return fout;
    }
}  // namespace gre
