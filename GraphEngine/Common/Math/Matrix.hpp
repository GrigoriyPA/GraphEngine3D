#pragma once

#include "Quaternion.hpp"


// Representation of a 4x4 matrix
namespace gre {
    class Matrix4x4 {
        double matrix_[4][4];

        inline double algebraic_addition(uint32_t x1, uint32_t x2, uint32_t x3, uint32_t y1, uint32_t y2, uint32_t y3) const noexcept;

    public:
        // Constructors
        Matrix4x4() noexcept;

        explicit Matrix4x4(double value) noexcept;

        template <typename T>  // Casts required: double(T)
        Matrix4x4(const std::initializer_list<std::initializer_list<T>>& init) {
            GRE_ENSURE(init.size() == 4, GreInvalidArgument, "invalid init size");

            uint32_t i = 0;
            for (const auto& line : init) {
                GRE_ENSURE(line.size() == 4, GreInvalidArgument, "invalid init size");

                uint32_t j = 0;
                for (const auto& element : line) {
                    matrix_[i][j++] = double(element);
                }
                ++i;
            }
        }

        Matrix4x4(const Vec3& vector_x, const Vec3& vector_y, const Vec3& vector_z) noexcept;

        explicit Matrix4x4(const aiMatrix4x4& init) noexcept;

        // Operators
        template <typename T>  // Constructors required: T(double)
        explicit operator std::vector<T>() const {
            std::vector<T> result;
            result.reserve(16);
            for (uint32_t j = 0; j < 4; ++j) {
                for (uint32_t i = 0; i < 4; ++i) {
                    result.push_back(T(matrix_[i][j]));
                }
            }
            return result;
        }

        double* operator[](size_t index);

        const double* operator[](size_t index) const;

        bool operator==(const Matrix4x4& other) const noexcept;

        bool operator!=(const Matrix4x4& other) const noexcept;

        Matrix4x4& operator+=(const Matrix4x4& other)& noexcept;

        Matrix4x4& operator-=(const Matrix4x4& other)& noexcept;

        Matrix4x4& operator*=(double other)& noexcept;

        Matrix4x4& operator*=(const Matrix4x4& other)& noexcept;

        Matrix4x4& operator/=(double other)& noexcept;

        Matrix4x4 operator-() const noexcept;

        Matrix4x4 operator+(const Matrix4x4& other) const noexcept;

        Matrix4x4 operator-(const Matrix4x4& other) const noexcept;

        Matrix4x4 operator*(double other) const noexcept;

        Matrix4x4 operator*(const Matrix4x4& other) const noexcept;

        Vec3 operator*(const Vec3& other) const noexcept;

        Matrix4x4 operator/(double other) const noexcept;

        // Math functions
        Matrix4x4 transpose() const noexcept;

        Matrix4x4 inverse() const noexcept;

        static Matrix4x4 normal_transform(const Matrix4x4& transform) noexcept;

        // Precalculated matrices
        static Matrix4x4 one_matrix() noexcept;

        static Matrix4x4 scale_matrix(double scale_x, double scale_y, double scale_z) noexcept;

        static Matrix4x4 scale_matrix(const Vec3& scale) noexcept;

        static Matrix4x4 scale_matrix(double scale) noexcept;

        static Matrix4x4 translation_matrix(const Vec3& translation) noexcept;

        static Matrix4x4 rotation_matrix(const Vec3& axis, double angle) noexcept;

        static Matrix4x4 rotation_matrix(const Quaternion& rotation_quaternion) noexcept;
    };

    // External operators
    std::ostream& operator<<(std::ostream& fout, const Matrix4x4& matrix);

    Matrix4x4 operator*(double value, const Matrix4x4& matrix) noexcept;
}  // namespace gre
