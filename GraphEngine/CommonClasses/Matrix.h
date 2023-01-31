#pragma once

#include <algorithm>
#include "MatrixLine.h"
#include "Vec3.h"


namespace gre {
	class Matrix {
		std::vector<MatrixLine> matrix_;

	public:
		template <typename T>  // Casts required: double(T)
		Matrix(const std::initializer_list<T>& init) {
			matrix_.reserve(init.size());
			for (const T& value : init) {
				matrix_.push_back(MatrixLine(1, value));
			}
		}

		template <typename T>  // Casts required: double(T)
		explicit Matrix(const std::vector<T>& init) {
			matrix_.reserve(init.size());
			for (const T& value : init) {
				matrix_.push_back(MatrixLine(1, value));
			}
		}

		template <typename T>  // Casts required: double(T)
		Matrix(const std::initializer_list<std::initializer_list<T>>& init) {
			size_t columns = 0;
			if (init.size() > 0) {
				columns = init.begin()->size();
			}

			matrix_.reserve(init.size());
			for (const auto& line : init) {
				if (columns != line.size()) {
					throw GreInvalidArgument(__FILE__, __LINE__, "Matrix, not all rows are the same size.\n\n");
				}

				matrix_.push_back(MatrixLine(line));
			}
		}

		template <typename T>  // Casts required: double(T)
		explicit Matrix(const std::vector<std::vector<T>>& init) {
			size_t columns = 0;
			if (!init.empty()) {
				columns = init[0].size();
			}

			matrix_.reserve(init.size());
			for (const auto& line : init) {
				if (columns != line.size()) {
					throw GreInvalidArgument(__FILE__, __LINE__, "Matrix, not all rows are the same size.\n\n");
				}

				matrix_.push_back(MatrixLine(line));
			}
		}

		Matrix(const Vec3& vector_x, const Vec3& vector_y, const Vec3& vector_z) {
			*this = Matrix({
				{ vector_x.x, vector_y.x, vector_z.x, 0.0 },
				{ vector_x.y, vector_y.y, vector_z.y, 0.0 },
				{ vector_x.z, vector_y.z, vector_z.z, 0.0 },
				{        0.0,        0.0,        0.0, 1.0 },
				});
		}

		Matrix(size_t count_lines, size_t count_columns, double value, const Matrix& init) {
			matrix_.resize(count_lines, MatrixLine(count_columns, value));
			for (size_t i = 0; i < std::min(count_lines, init.count_strings()); ++i) {
				for (size_t j = 0; j < std::min(count_columns, init.count_columns()); ++j) {
					matrix_[i][j] = init[i][j];
				}
			}
		}

		Matrix(size_t count_lines, size_t count_columns, double value = 0.0) {
			matrix_.resize(count_lines, MatrixLine(count_columns, value));
		}

		Matrix(size_t count_lines, const MatrixLine& value) {
			matrix_.resize(count_lines, value);
		}

		// Identity matrix with init part replaced
		Matrix(size_t count_lines, size_t count_columns, const Matrix& init) {
			matrix_.resize(count_lines, MatrixLine(count_columns, 0));
			for (size_t i = 0; i < std::min(count_lines, count_columns); ++i) {
				matrix_[i][i] = 1;
			}
			for (size_t i = 0; i < std::min(count_lines, init.count_strings()); ++i) {
				for (size_t j = 0; j < std::min(count_columns, init.count_columns()); ++j) {
					matrix_[i][j] = init[i][j];
				}
			}
		}

		template <typename T>  // Constructors required: T(double)
		explicit operator std::vector<T>() const {
			std::vector<T> result;
			result.reserve(matrix_.size() * count_columns());
			for (size_t j = 0; j < count_columns(); ++j) {
				for (size_t i = 0; i < matrix_.size(); ++i) {
					result.push_back(T(matrix_[i][j]));
				}
			}
			return result;
		}

		MatrixLine& operator[](size_t index) {
			if (matrix_.size() <= index) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return matrix_[index];
		}

		const MatrixLine& operator[](size_t index) const {
			if (matrix_.size() <= index) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return matrix_[index];
		}

		bool operator==(const Matrix& other) const noexcept {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				return false;
			}

			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					if (!equality(matrix_[i][j], other[i][j])) {
						return false;
					}
				}
			}
			return true;
		}

		bool operator!=(const Matrix& other) const noexcept {
			return !(*this == other);
		}

		Matrix& operator+=(const Matrix& other)& {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator+=, invalid matrix sizes.\n\n");
			}

			*this = *this + other;
			return *this;
		}

		Matrix& operator-=(const Matrix& other)& {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator-=, invalid matrix sizes.\n\n");
			}

			*this = *this - other;
			return *this;
		}

		Matrix& operator*=(double other)& noexcept {
			for (size_t i = 0; i < matrix_.size(); i++) {
				matrix_[i] *= other;
			}
			return *this;
		}

		Matrix& operator*=(const Matrix& other)& {
			if (count_columns() != other.count_strings()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator*=, invalid matrix sizes.\n\n");
			}

			*this = *this * other;
			return *this;
		}

		Matrix& operator/=(double other)& {
			if (equality(other, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
			}

			*this = *this / other;
			return *this;
		}

		Matrix& operator|=(const Matrix& other)& {
			if (matrix_.size() != other.count_strings()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator|=, invalid matrix sizes.\n\n");
			}

			*this = *this | other;
			return *this;
		}

		Matrix operator-() const {
			Matrix result = *this;
			for (size_t i = 0; i < matrix_.size(); i++) {
				result[i] *= -1;
			}
			return result;
		}

		Matrix operator+(const Matrix& other) const {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator+, invalid matrix sizes.\n\n");
			}

			Matrix result = *this;
			for (size_t i = 0; i < matrix_.size(); i++) {
				result[i] += other[i];
			}
			return result;
		}

		Matrix operator-(const Matrix& other) const {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator-, invalid matrix sizes.\n\n");
			}

			return *this + (-other);
		}

		Matrix operator*(double other) const {
			Matrix result = *this;
			for (size_t i = 0; i < matrix_.size(); i++) {
				result[i] *= other;
			}
			return result;
		}

		Matrix operator*(const Matrix& other) const {
			if (count_columns() != other.count_strings()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator*, invalid matrix sizes.\n\n");
			}

			Matrix transposed = other.transpose();
			Matrix result(matrix_.size(), other.count_columns(), 0);
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < other.count_columns(); ++j) {
					result[i][j] = matrix_[i] * transposed[j];
				}
			}
			return result;
		}

		Vec3 operator*(const Vec3& other) const {
			if (matrix_.size() != 4 || matrix_[0].size() != 4) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator*, invalid matrix size.\n\n");
			}

			Vec3 result;
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					result[i] += matrix_[i][j] * other[j];
				}
				result[i] += matrix_[i][3];
			}
			return result;
		}

		Matrix operator/(double other) const {
			if (equality(other, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
			}

			return *this * (1.0 / other);
		}

		Matrix operator|(const Matrix& other) const {
			if (matrix_.size() != other.count_strings()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "operator|, invalid matrix sizes.\n\n");
			}

			Matrix result(matrix_.size(), count_columns() + other.count_columns(), *this);
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < other.count_columns(); ++j) {
					result[i][count_columns() + j] = other[i][j];
				}
			}
			return result;
		}

		template <typename T>
		T get_value(T value, std::function<void(double, T*)> func) const {
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					func(matrix_[i][j], &value);
				}
			}
			return value;
		}

		size_t count_strings() const noexcept {
			return matrix_.size();
		}

		size_t count_columns() const noexcept {
			if (matrix_.empty()) {
				return 0;
			}
			return matrix_[0].size();
		}

		Matrix transpose() const {
			Matrix result(count_columns(), matrix_.size());
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					result[j][i] = matrix_[i][j];
				}
			}
			return result;
		}

		Matrix flip_horizontally() const {
			Matrix result(matrix_.size(), count_columns());
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					result[matrix_.size() - 1 - i][j] = matrix_[i][j];
				}
			}
			return result;
		}

		Matrix flip_vertically() const {
			Matrix result(matrix_.size(), count_columns());
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					result[i][matrix_[i].size() - 1 - j] = matrix_[i][j];
				}
			}
			return result;
		}

		Matrix rotate_clockwise() const {
			return flip_horizontally().transpose();
		}

		Matrix rotate_counterclockwise() const {
			return flip_vertically().transpose();
		}

		Matrix submatrix(size_t line, size_t column, size_t height, size_t width) const {
			if (line + height > matrix_.size() || column + width > count_columns()) {
				throw GreInvalidArgument(__FILE__, __LINE__, "submatrix, invalid submatrix size.\n\n");
			}

			Matrix result(height, width);
			for (size_t i = 0; i < height; ++i) {
				for (size_t j = 0; j < width; ++j) {
					result[i][j] = matrix_[line + i][column + j];
				}
			}
			return result;
		}

		Matrix improved_step_view() const {
			Matrix result = *this;
			for (size_t j = 0, i0 = 0; j < count_columns() && i0 < matrix_.size(); ++j) {
				size_t k = matrix_.size();
				for (size_t i = i0; i < matrix_.size(); ++i) {
					if (!equality(result[i][j], 0.0)) {
						k = i;
						break;
					}
				}

				if (k == matrix_.size()) {
					continue;
				}

				result[i0].swap(result[k]);
				result[i0] *= 1.0 / result[i0][j];
				for (size_t i = 0; i < matrix_.size(); ++i) {
					if (i == i0) {
						continue;
					}

					result[i] -= result[i0] * result[i][j];
				}
				++i0;
			}
			return result;
		}

		double determinant() const {
			if (matrix_.size() != count_columns()) {
				throw GreDomainError(__FILE__, __LINE__, "determinant, not a square matrix.\n\n");
			}

			double result = 1.0;
			Matrix cur_matrix = *this;
			for (size_t j = 0, i0 = 0; j < count_columns(); ++j) {
				size_t k = matrix_.size();
				for (size_t i = i0; i < matrix_.size(); ++i) {
					if (!equality(cur_matrix[i][j], 0.0)) {
						k = i;
						break;
					}
				}

				if (k == matrix_.size()) {
					return 0;
				}

				if (k != i0) {
					result *= -1;
					cur_matrix[i0].swap(cur_matrix[k]);
				}
				result *= cur_matrix[i0][j];
				cur_matrix[i0] *= 1.0 / cur_matrix[i0][j];
				for (size_t i = i0 + 1; i < matrix_.size(); ++i) {
					cur_matrix[i] -= cur_matrix[i0] * cur_matrix[i][j];
				}
				++i0;
			}
			return result;
		}

		Matrix inverse() const {
			if (matrix_.size() != count_columns()) {
				throw GreDomainError(__FILE__, __LINE__, "inverse, not a square matrix.\n\n");
			}

			Matrix result = (*this | one_matrix(matrix_.size())).improved_step_view();
			if (result.submatrix(0, 0, matrix_.size(), matrix_.size()) != one_matrix(matrix_.size())) {
				throw GreDomainError(__FILE__, __LINE__, "inverse, the matrix is not invertible.\n\n");
			}
			return result.submatrix(0, matrix_.size(), matrix_.size(), matrix_.size());
		}

		Matrix solve_equation(const Matrix& value) const {
			if (value.count_columns() != 1 || matrix_.size() != value.count_strings()) {
				throw GreDomainError(__FILE__, __LINE__, "solve_equation, invalid matrix size.\n\n");
			}

			Matrix isv_matrix = (*this | Matrix(value)).improved_step_view();
			Matrix result(count_columns(), 1, 0);
			for (size_t i = 0, j = 0; i < matrix_.size(); ++i) {
				for (; j <= count_columns() && !equality(isv_matrix[i][j], 1.0); ++j) {}

				if (j == count_columns()) {
					return Matrix(0, 0);
				}
				if (j > count_columns()) {
					break;
				}

				result[j][0] = isv_matrix[i].back();
			}
			return result;
		}

		static Matrix zip_map(const Matrix& matrix1, const Matrix& matrix2, std::function<double(double, double)> zip_func) {
			Matrix result(std::min(matrix1.count_strings(), matrix2.count_strings()), std::min(matrix1.count_columns(), matrix2.count_columns()));
			for (size_t i = 0; i < result.count_strings(); ++i) {
				for (size_t j = 0; j < result.count_columns(); ++j) {
					result[i][j] = zip_func(matrix1[i][j], matrix2[i][j]);
				}
			}
			return result;
		}

		static Matrix one_matrix(size_t size) {
			Matrix result(size, size, 0);
			for (size_t i = 0; i < size; ++i) {
				result[i][i] = 1;
			}
			return result;
		}

		static Matrix scale_matrix(const Vec3& scale) {
			return Matrix({
				{ scale.x,     0.0,     0.0, 0.0 },
				{     0.0, scale.y,     0.0, 0.0 },
				{     0.0,     0.0, scale.z, 0.0 },
				{     0.0,     0.0,     0.0, 1.0 },
				});
		}

		static Matrix scale_matrix(double scale) {
			return Matrix({
				{ scale,   0.0,   0.0, 0.0 },
				{   0.0, scale,   0.0, 0.0 },
				{   0.0,   0.0, scale, 0.0 },
				{   0.0,   0.0,   0.0, 1.0 },
				});
		}

		static Matrix translation_matrix(const Vec3& translation) {
			return Matrix({
				{ 1.0, 0.0, 0.0, translation.x },
				{ 0.0, 1.0, 0.0, translation.y },
				{ 0.0, 0.0, 1.0, translation.z },
				{ 0.0, 0.0, 0.0,           1.0 },
				});
		}

		static Matrix rotation_matrix(const Vec3& axis, double angle) {
			if (equality(axis.length(), 0.0)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "rotation_matrix, the axis vector has zero length.\n\n");
			}

			Vec3 norm_axis = axis.normalize();
			double x = norm_axis.x;
			double y = norm_axis.y;
			double z = norm_axis.z;
			double c = cos(angle);
			double s = sin(angle);

			return Matrix({
				{     c + x * x * (1 - c), x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0.0 },
				{ y * x * (1 - c) + z * s,     c + y * y * (1 - c), y * z * (1 - c) - x * s, 0.0 },
				{ z * x * (1 - c) - y * s, z * y * (1 - c) + x * s,     c + z * z * (1 - c), 0.0 },
				{                     0.0,                     0.0,                     0.0, 1.0 },
				});
		}

		static Matrix normal_transform(const Matrix& transform) {
			if (transform.count_strings() != 4 || transform.count_columns() != 4) {
				throw GreInvalidArgument(__FILE__, __LINE__, "normal_transform, invalid matrix size.\n\n");
			}

			try {
				return transform.inverse().transpose();
			}
			catch (GreDomainError) {
				throw GreDomainError(__FILE__, __LINE__, "normal_transform, the matrix is not invertible.\n\n");
			}
		}
	};

	std::istream& operator>>(std::istream& fin, Matrix& matrix) noexcept {
		for (size_t i = 0; i < matrix.count_strings(); ++i) {
			for (size_t j = 0; j < matrix.count_columns(); ++j) {
				fin >> matrix[i][j];
			}
		}
		return fin;
	}

	std::ostream& operator<<(std::ostream& fout, const Matrix& matrix) {
		std::vector<std::string> output(matrix.count_strings());
		for (std::string& line : output) {
			line += char(179);
		}
		for (size_t j = 0; j < matrix.count_columns(); ++j) {
			size_t string_size = 0;
			for (size_t i = 0; i < matrix.count_strings(); ++i) {
				output[i] += std::to_string(matrix[i][j]);
				string_size = std::max(string_size, output[i].size());
			}

			if (j < matrix.count_columns() - 1) {
				++string_size;
			}
			for (size_t i = 0; i < matrix.count_strings(); ++i) {
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

	Matrix operator*(double value, const Matrix& matrix) {
		return matrix * value;
	}
}
