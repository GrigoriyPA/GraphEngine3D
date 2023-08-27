#include "Vec3.h"


// Vec3
namespace gre {
	// Constructors
	Vec3::Vec3() noexcept {
	}

	Vec3::Vec3(double value) noexcept
		: x(value)
		, y(value)
		, z(value)
	{}

	Vec3::Vec3(double x, double y, double z) noexcept
		: x(x)
		, y(y)
		, z(z)
	{}

	Vec3::Vec3(const Vec2& xy, double z) noexcept
		: x(xy.x)
		, y(xy.y)
		, z(z)
	{}

	Vec3::Vec3(double x, const Vec2& yz) noexcept
		: x(x)
		, y(yz.x)
		, z(yz.y)
	{}

	Vec3::Vec3(const sf::Color& color) noexcept
		: x(color.r)
		, y(color.g)
		, z(color.b)
	{}

	Vec3::Vec3(const sf::Vector3f& init) noexcept
		: x(init.x)
		, y(init.y)
		, z(init.z)
	{}

	Vec3::Vec3(const aiVector3D& init) noexcept
		: x(init.x)
		, y(init.y)
		, z(init.z)
	{}

	// Operators
	double& Vec3::operator[](size_t index) {
		GRE_ENSURE(index <= 2, GreOutOfRange, "index out of range");

		switch (index) {
			case 0: return x;
			case 1: return y;
			case 2: return z;
		}
	}

	const double& Vec3::operator[](size_t index) const {
		GRE_ENSURE(index <= 2, GreOutOfRange, "index out of range");

		switch (index) {
			case 0: return x;
			case 1: return y;
			case 2: return z;
		}
	}

	bool Vec3::operator==(const Vec3& other) const noexcept {
		return equality(x, other.x) && equality(y, other.y) && equality(z, other.z);
	}

	bool Vec3::operator!=(const Vec3& other) const noexcept {
		return !equality(x, other.x) || !equality(y, other.y) || !equality(z, other.z);
	}

	Vec3& Vec3::operator+=(const Vec3& other)& noexcept {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3& Vec3::operator-=(const Vec3& other)& noexcept {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vec3& Vec3::operator*=(double other)& noexcept {
		x *= other;
		y *= other;
		z *= other;
		return *this;
	}

	Vec3& Vec3::operator/=(double other)& noexcept {
		GRE_CHECK(!equality(other, 0.0), "division by zero");

		x /= other;
		y /= other;
		z /= other;
		return *this;
	}

	Vec3& Vec3::operator^=(double other)& noexcept {
		GRE_CHECK(x >= 0.0 && y >= 0.0 && x >= 0.0, "raising a negative number to a power");

		x = std::pow(x, other);
		y = std::pow(y, other);
		z = std::pow(z, other);
		return *this;
	}

	Vec3 Vec3::operator-() const noexcept {
		return Vec3(-x, -y, -z);
	}

	Vec3 Vec3::operator+(const Vec3& other) const noexcept {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	Vec3 Vec3::operator-(const Vec3& other) const noexcept {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	Vec3 Vec3::operator*(double other) const noexcept {
		return Vec3(x * other, y * other, z * other);
	}

	double Vec3::operator*(const Vec3& other) const noexcept {
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 Vec3::operator/(double other) const noexcept {
		GRE_CHECK(!equality(other, 0.0), "division by zero");

		return Vec3(x / other, y / other, z / other);
	}

	Vec3 Vec3::operator^(const Vec3& other) const noexcept {
		return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	Vec3 Vec3::operator^(double other) const noexcept {
		GRE_CHECK(x >= 0.0 && y >= 0.0 && z >= 0.0, "raising a negative number to a power");

		return Vec3(std::pow(x, other), std::pow(y, other), std::pow(z, other));
	}

	// Math functions
	double Vec3::length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	Vec3 Vec3::normalize() const noexcept {
		double vect_length = length();

		GRE_CHECK(!equality(vect_length, 0.0), "null vector normalization");

		return *this / vect_length;
	}

	// Other functions
	Vec3 Vec3::horizon() const noexcept {
		double vect_length = z * z + x * x;

		if (equality(vect_length, 0.0)) {
			return Vec3(1.0, 0.0, 0.0);
		}

		vect_length = sqrt(vect_length);
		return Vec3(z / vect_length, 0.0, -x / vect_length);
	}

	// External operators
	std::ostream& operator<<(std::ostream& fout, const Vec3& vector) {
		fout << '(' << vector.x << ", " << vector.y << ", " << vector.z << ')';
		return fout;
	}

	Vec3 operator*(double value, const Vec3& vector) noexcept {
		return Vec3(vector.x * value, vector.y * value, vector.z * value);
	}
}  // namespace gre
