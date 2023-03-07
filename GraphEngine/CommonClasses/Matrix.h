#pragma once

#include "Vec3.h"


namespace gre {
    class Matrix4x4 {
        double matrix_[4][4];

        double algebraic_addition(uint32_t x1, uint32_t x2, uint32_t x3, uint32_t y1, uint32_t y2, uint32_t y3) const noexcept {
            double result = 0.0;
            result += matrix_[x1][y1] * matrix_[x2][y2] * matrix_[x3][y3];
            result += matrix_[x1][y2] * matrix_[x2][y3] * matrix_[x3][y1];
            result += matrix_[x1][y3] * matrix_[x2][y1] * matrix_[x3][y2];

            result -= matrix_[x1][y1] * matrix_[x2][y3] * matrix_[x3][y2];
            result -= matrix_[x1][y2] * matrix_[x2][y1] * matrix_[x3][y3];
            result -= matrix_[x1][y3] * matrix_[x2][y2] * matrix_[x3][y1];
            return result;
        }

    public:
        Matrix4x4() noexcept {
            for (uint32_t i = 0; i < 4; ++i) {
                for (uint32_t j = 0; j < 4; ++j) {
                    matrix_[i][j] = 0.0;
                }
            }
        }

        explicit Matrix4x4(double value) noexcept {
            for (uint32_t i = 0; i < 4; ++i) {
                for (uint32_t j = 0; j < 4; ++j) {
                    matrix_[i][j] = value;
                }
            }
        }

        template <typename T>  // Casts required: double(T)
        Matrix4x4(const std::initializer_list<std::initializer_list<T>>& init) {
#ifdef _DEBUG
            if (init.size() != 4) {
                throw GreInvalidArgument(__FILE__, __LINE__, "Matrix4x4, invalid init size.\n\n");
            }
#endif // _DEBUG

            uint32_t i = 0;
            for (const auto& line : init) {
#ifdef _DEBUG
                if (line.size() != 4) {
                    throw GreInvalidArgument(__FILE__, __LINE__, "Matrix4x4, invalid init size.\n\n");
                }
#endif // _DEBUG

                uint32_t j = 0;
                for (const auto& element : line) {
                    matrix_[i][j++] = double(element);
                }
                ++i;
            }
        }

        Matrix4x4(const Vec3& vector_x, const Vec3& vector_y, const Vec3& vector_z) noexcept {
            *this = Matrix4x4({
                { vector_x.x, vector_y.x, vector_z.x, 0.0 },
                { vector_x.y, vector_y.y, vector_z.y, 0.0 },
                { vector_x.z, vector_y.z, vector_z.z, 0.0 },
                {        0.0,        0.0,        0.0, 1.0 },
            });
        }

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

        double* operator[](size_t index) {
#ifdef _DEBUG
            if (4 <= index) {
                throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
            }
#endif // _DEBUG

            return matrix_[index];
        }

        const double* operator[](size_t index) const {
#ifdef _DEBUG
            if (4 <= index) {
                throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
            }
#endif // _DEBUG

            return matrix_[index];
        }

        bool operator==(const Matrix4x4& other) const noexcept {
            for (uint32_t i = 0; i < 4; ++i) {
                for (uint32_t j = 0; j < 4; ++j) {
                    if (!equality(matrix_[i][j], other[i][j])) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool operator!=(const Matrix4x4& other) const noexcept {
            return !(*this == other);
        }

        Matrix4x4& operator+=(const Matrix4x4& other)& noexcept {
            for (uint32_t i = 0; i < 4; i++) {
                for (uint32_t j = 0; j < 4; ++j) {
                    matrix_[i][j] += other[i][j];
                }
            }
            return *this;
        }

        Matrix4x4& operator-=(const Matrix4x4& other)& noexcept {
            for (uint32_t i = 0; i < 4; i++) {
                for (uint32_t j = 0; j < 4; ++j) {
                    matrix_[i][j] -= other[i][j];
                }
            }
            return *this;
        }

        Matrix4x4& operator*=(double other)& noexcept {
            for (uint32_t i = 0; i < 4; i++) {
                for (uint32_t j = 0; j < 4; ++j) {
                    matrix_[i][j] *= other;
                }
            }
            return *this;
        }

        Matrix4x4& operator*=(const Matrix4x4& other)& noexcept {
            *this = *this * other;
            return *this;
        }

        Matrix4x4& operator/=(double other)& {
#ifdef _DEBUG
            if (equality(other, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
            }
#endif // _DEBUG

            for (uint32_t i = 0; i < 4; i++) {
                for (uint32_t j = 0; j < 4; ++j) {
                    matrix_[i][j] /= other;
                }
            }
            return *this;
        }

        Matrix4x4 operator-() const noexcept {
            Matrix4x4 result = *this;
            return result *= -1.0;
        }

        Matrix4x4 operator+(const Matrix4x4& other) const noexcept {
            Matrix4x4 result = *this;
            return result += other;
        }

        Matrix4x4 operator-(const Matrix4x4& other) const noexcept {
            Matrix4x4 result = *this;
            return result -= other;
        }

        Matrix4x4 operator*(double other) const noexcept {
            Matrix4x4 result = *this;
            return result *= other;
        }

        Matrix4x4 operator*(const Matrix4x4& other) const noexcept {
            Matrix4x4 result(0.0);
            for (uint32_t i = 0; i < 4; ++i) {
                for (uint32_t j = 0; j < 4; ++j) {
                    for (uint32_t k = 0; k < 4; ++k) {
                        result[i][j] += matrix_[i][k] * other[k][j];
                    }
                }
            }
            return result;
        }

        Vec3 operator*(const Vec3& other) const noexcept {
            Vec3 result(0.0);
            result.x = matrix_[0][0] * other[0] + matrix_[0][1] * other[1] + matrix_[0][2] * other[2] + matrix_[0][3];
            result.y = matrix_[1][0] * other[0] + matrix_[1][1] * other[1] + matrix_[1][2] * other[2] + matrix_[1][3];
            result.z = matrix_[2][0] * other[0] + matrix_[2][1] * other[1] + matrix_[2][2] * other[2] + matrix_[2][3];
            return result;
        }

        Matrix4x4 operator/(double other) const {
#ifdef _DEBUG
            if (equality(other, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
            }
#endif // _DEBUG

            Matrix4x4 result = *this;
            return result *= 1.0 / other;
        }

        Matrix4x4 transpose() const noexcept {
            Matrix4x4 result;
            for (uint32_t i = 0; i < 4; ++i) {
                for (uint32_t j = 0; j < 4; ++j) {
                    result[j][i] = matrix_[i][j];
                }
            }
            return result;
        }

        Matrix4x4 inverse() const {
            Matrix4x4 result;

            result[0][0] = algebraic_addition(1, 2, 3, 1, 2, 3);
            result[1][0] = -algebraic_addition(1, 2, 3, 0, 2, 3);
            result[2][0] = algebraic_addition(1, 2, 3, 0, 1, 3);
            result[3][0] = -algebraic_addition(1, 2, 3, 0, 1, 2);

            double det = result[0][0] * matrix_[0][0] + result[1][0] * matrix_[0][1] + result[2][0] * matrix_[0][2] + result[3][0] * matrix_[0][3];

#ifdef _DEBUG
            if (equality(det, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "inverse, the matrix is not invertible.\n\n");
            }
#endif // _DEBUG

            result[0][1] = -algebraic_addition(0, 2, 3, 1, 2, 3);
            result[1][1] = algebraic_addition(0, 2, 3, 0, 2, 3);
            result[2][1] = -algebraic_addition(0, 2, 3, 0, 1, 3);
            result[3][1] = algebraic_addition(0, 2, 3, 0, 1, 2);

            result[0][2] = algebraic_addition(0, 1, 3, 1, 2, 3);
            result[1][2] = -algebraic_addition(0, 1, 3, 0, 2, 3);
            result[2][2] = algebraic_addition(0, 1, 3, 0, 1, 3);
            result[3][2] = -algebraic_addition(0, 1, 3, 0, 1, 2);

            result[0][3] = -algebraic_addition(0, 1, 2, 1, 2, 3);
            result[1][3] = algebraic_addition(0, 1, 2, 0, 2, 3);
            result[2][3] = -algebraic_addition(0, 1, 2, 0, 1, 3);
            result[3][3] = algebraic_addition(0, 1, 2, 0, 1, 2);

            return result / det;
        }

        static Matrix4x4 one_matrix() noexcept {
            return Matrix4x4({
                { 1.0, 0.0, 0.0, 0.0 },
                { 0.0, 1.0, 0.0, 0.0 },
                { 0.0, 0.0, 1.0, 0.0 },
                { 0.0, 0.0, 0.0, 1.0 },
            });
        }

        static Matrix4x4 scale_matrix(double scale_x, double scale_y, double scale_z) noexcept {
            return Matrix4x4({
                { scale_x,     0.0,     0.0, 0.0 },
                {     0.0, scale_y,     0.0, 0.0 },
                {     0.0,     0.0, scale_z, 0.0 },
                {     0.0,     0.0,     0.0, 1.0 },
            });
        }

        static Matrix4x4 scale_matrix(const Vec3& scale) noexcept {
            return Matrix4x4({
                { scale.x,     0.0,     0.0, 0.0 },
                {     0.0, scale.y,     0.0, 0.0 },
                {     0.0,     0.0, scale.z, 0.0 },
                {     0.0,     0.0,     0.0, 1.0 },
            });
        }

        static Matrix4x4 scale_matrix(double scale) noexcept {
            return Matrix4x4({
                { scale,   0.0,   0.0, 0.0 },
                {   0.0, scale,   0.0, 0.0 },
                {   0.0,   0.0, scale, 0.0 },
                {   0.0,   0.0,   0.0, 1.0 },
            });
        }

        static Matrix4x4 translation_matrix(const Vec3& translation) noexcept {
            return Matrix4x4({
                { 1.0, 0.0, 0.0, translation.x },
                { 0.0, 1.0, 0.0, translation.y },
                { 0.0, 0.0, 1.0, translation.z },
                { 0.0, 0.0, 0.0,           1.0 },
            });
        }

        static Matrix4x4 rotation_matrix(const Vec3& axis, double angle) {
#ifdef _DEBUG
            if (equality(axis.length(), 0.0)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "rotation_matrix, the axis vector has zero length.\n\n");
            }
#endif // _DEBUG

            const Vec3& norm_axis = axis.normalize();
            double x = norm_axis.x;
            double y = norm_axis.y;
            double z = norm_axis.z;
            double c = cos(angle);
            double s = sin(angle);

            return Matrix4x4({
                {     c + x * x * (1.0 - c), x * y * (1.0 - c) - z * s, x * z * (1.0 - c) + y * s, 0.0 },
                { y * x * (1.0 - c) + z * s,     c + y * y * (1.0 - c), y * z * (1.0 - c) - x * s, 0.0 },
                { z * x * (1.0 - c) - y * s, z * y * (1.0 - c) + x * s,     c + z * z * (1.0 - c), 0.0 },
                {                       0.0,                       0.0,                       0.0, 1.0 },
            });
        }

        static Matrix4x4 normal_transform(const Matrix4x4& transform) {
#ifdef _DEBUG
            try {
                return transform.inverse().transpose();
            }
            catch (GreDomainError) {
                throw GreDomainError(__FILE__, __LINE__, "normal_transform, the matrix is not invertible.\n\n");
            }
#else // _DEBUG
            return transform.inverse().transpose();
#endif // NO _DEBUG
        }
    };

    std::ostream& operator<<(std::ostream& fout, const Matrix4x4& matrix) {
        std::vector<std::string> output(4);
        for (std::string& line : output) {
            line += char(179);
        }
        for (size_t j = 0; j < 4; ++j) {
            size_t string_size = 0;
            for (size_t i = 0; i < 4; ++i) {
                output[i] += std::to_string(matrix[i][j]);
                string_size = std::max(string_size, output[i].size());
            }
            
            string_size += j < 3;
            for (size_t i = 0; i < 4; ++i) {
                for (; output[i].size() < string_size; output[i] += ' ') {}
            }
        }

        std::string space_line;
        if (!output.empty()) {
            for (size_t i = 0; i < output[0].size() - 1; ++i) {
                space_line += ' ';
            }
        }

        fout << char(218) << space_line << char(191) << "\n";
        for (const std::string& line : output) {
            fout << line << char(179) << "\n";
        }
        fout << char(192) << space_line << char(217) << "\n";
        return fout;
    }

    Matrix4x4 operator*(double value, const Matrix4x4& matrix) noexcept {
        return matrix * value;
    }
}
