#pragma once

#include "Vect3.h"


namespace eng {
	class Matrix {
		class MatrixLine {
			std::vector<double> line_;

		public:
			template <typename T>
			MatrixLine(const std::initializer_list<T>& init) {
				line_.reserve(init.size());
				for (const T& element : init) {
					line_.push_back(static_cast<double>(element));
				}
			}

			template <typename T>
			explicit MatrixLine(const std::vector<T>& init) {
				line_.reserve(init.size());
				for (const T& element : init) {
					line_.push_back(static_cast<double>(element));
				}
			}

			explicit MatrixLine(size_t count_columns, double value = 0) {
				line_.resize(count_columns, value);
			}

			// In case of an error returns last column
			double& operator[](size_t index) {
				if (line_.size() <= index) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR[](SIZE_T)\n" << "Invalid index.\n\n";
					return line_.back();
				}

				return line_[index];
			}

			// In case of an error returns last column
			double operator[](size_t index) const {
				if (line_.size() <= index) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR[](SIZE_T)\n" << "Invalid index.\n\n";
					return line_.back();
				}

				return line_[index];
			}

			// In case of an error skips operation
			MatrixLine& operator +=(const MatrixLine& other)& {
				if (line_.size() != other.size()) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR+=(MATRIX_LINE)\n" << "Invalid line sizes.\n\n";
					return *this;
				}

				*this = *this + other;
				return *this;
			}

			// In case of an error skips operation
			MatrixLine& operator -=(const MatrixLine& other)& {
				if (line_.size() != other.size()) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR-=(MATRIX_LINE)\n" << "Invalid line sizes.\n\n";
					return *this;
				}

				*this = *this - other;
				return *this;
			}

			MatrixLine& operator *=(double other)& {
				*this = *this * other;
				return *this;
			}

			// In case of an error skips operation
			MatrixLine operator +(const MatrixLine& other) const {
				if (line_.size() != other.size()) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR+(MATRIX_LINE)\n" << "Invalid line sizes.\n\n";
					return *this;
				}

				MatrixLine result = *this;
				for (size_t i = 0; i < other.size(); ++i) {
					result[i] += other[i];
				}

				return result;
			}

			// In case of an error skips operation
			MatrixLine operator -(const MatrixLine& other) const {
				if (line_.size() != other.size()) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR-(MATRIX_LINE)\n" << "Invalid line sizes.\n\n";
					return *this;
				}

				MatrixLine result = *this;
				for (size_t i = 0; i < other.size(); ++i) {
					result[i] -= other[i];
				}

				return result;
			}

			MatrixLine operator *(double other) const {
				MatrixLine result = *this;
				for (double& element : result.line_) {
					element *= other;
				}

				return result;
			}

			// In case of an error returns 0
			double operator *(const MatrixLine& other) const {
				if (line_.size() != other.size()) {
					std::cout << "ERROR::MATRIX::MATRIX_LINE::OPERATOR*(MATRIX_LINE)\n" << "Invalid line sizes.\n\n";
					return 0;
				}

				double result = 0;
				for (size_t i = 0; i < line_.size(); ++i) {
					result += line_[i] * other[i];
				}

				return result;
			}

			double& back() {
				return line_.back();
			}

			double back() const {
				return line_.back();
			}

			size_t size() const {
				return line_.size();
			}

			void swap(MatrixLine& other) {
				std::swap(line_, other.line_);
			}
		};

		double eps_ = 1e-5;

		std::vector<MatrixLine> matrix_;

	public:
		template <typename T>
		Matrix(const std::initializer_list<T>& init) {
			matrix_.reserve(init.size());
			for (const T& value : init) {
				matrix_.push_back(MatrixLine(1, value));
			}
		}

		template <typename T>
		explicit Matrix(const std::vector<T>& init) {
			matrix_.reserve(init.size());
			for (const T& value : init) {
				matrix_.push_back(MatrixLine(1, value));
			}
		}

		// In case of an error skips all invalid columns
		template <typename T>
		Matrix(const std::initializer_list<std::initializer_list<T>>& init) {
			size_t columns = 0;
			if (init.size() > 0) {
				columns = init.begin()->size();
			}

			matrix_.reserve(init.size());
			for (const auto& line : init) {
				if (columns != line.size()) {
					std::cout << "ERROR::MATRIX::BUILDER\n" << "Not all rows are the same size.\n\n";
					break;
				}

				matrix_.push_back(MatrixLine(line));
			}
		}

		// In case of an error skips all invalid columns
		template <typename T>
		explicit Matrix(const std::vector<std::vector<T>>& init) {
			size_t columns = 0;
			if (!init.empty()) {
				columns = init[0].size();
			}

			matrix_.reserve(init.size());
			for (const auto& line : init) {
				if (columns != line.size()) {
					std::cout << "ERROR::MATRIX::BUILDER\n" << "Not all rows are the same size.\n\n";
					break;
				}

				matrix_.push_back(MatrixLine(line));
			}
		}

		Matrix(const Vect3& vector_x, const Vect3& vector_y, const Vect3& vector_z) {
			*this = Matrix({
				{ vector_x.x, vector_y.x, vector_z.x, 0.0 },
				{ vector_x.y, vector_y.y, vector_z.y, 0.0 },
				{ vector_x.z, vector_y.z, vector_z.z, 0.0 },
				{        0.0,        0.0,        0.0, 1.0 },
			});
		}

		Matrix(size_t count_lines, size_t count_columns, double value = 0) {
			matrix_.resize(count_lines, MatrixLine(count_columns, value));
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

		template <typename T>
		explicit operator std::vector<T>() const {
			std::vector<T> value(matrix_.size() * count_columns());
			for (size_t j = 0; j < count_columns(); ++j) {
				for (size_t i = 0; i < matrix_.size(); ++i) {
					value[j * matrix_.size() + i] = static_cast<T>(matrix_[i][j]);
				}
			}

			return value;
		}

		// In case of an error returns last line
		MatrixLine& operator[](size_t index) {
			if (matrix_.size() <= index) {
				std::cout << "ERROR::MATRIX::OPERATOR[](SIZE_T)\n" << "Invalid index.\n\n";
				return matrix_.back();
			}

			return matrix_[index];
		}

		// In case of an error returns last line
		const MatrixLine& operator[](size_t index) const {
			if (matrix_.size() <= index) {
				std::cout << "ERROR::MATRIX::OPERATOR[](SIZE_T)\n" << "Invalid index.\n\n";
				return matrix_.back();
			}

			return matrix_[index];
		}

		bool operator ==(const Matrix& other) const {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				return false;
			}

			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					if (!equality(matrix_[i][j], other[i][j], eps_)) {
						return false;
					}
				}
			}

			return true;
		}

		bool operator !=(const Matrix& other) const {
			return !(*this == other);
		}

		// In case of an error skips operation
		Matrix& operator +=(const Matrix& other)& {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				std::cout << "ERROR::MATRIX::OPERATOR+=(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			*this = *this + other;
			return *this;
		}

		// In case of an error skips operation
		Matrix& operator -=(const Matrix& other)& {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				std::cout << "ERROR::MATRIX::OPERATOR-=(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			*this = *this - other;
			return *this;
		}

		Matrix& operator *=(double other)& {
			*this = *this * other;
			return *this;
		}

		// In case of an error skips operation
		Matrix& operator *=(const Matrix& other)& {
			if (count_columns() != other.count_strings()) {
				std::cout << "ERROR::MATRIX::OPERATOR*=(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			*this = *this * other;
			return *this;
		}

		// In case of an error skips operation
		Matrix& operator /=(double other)& {
			if (equality(other, 0.0, eps_)) {
				std::cout << "ERROR::MATRIX::OPERATOR/=(DOUBLE)\n" << "Division by zero.\n\n";
				return *this;
			}

			*this = *this / other;
			return *this;
		}

		// In case of an error skips operation
		Matrix& operator ^=(uint64_t other)& {
			if (matrix_.size() != count_columns()) {
				std::cout << "ERROR::MATRIX::OPERATOR^=(UINT64_T)\n" << "Invalid matrix size.\n\n";
				return *this;
			}

			*this = *this ^ other;
			return *this;
		}

		// In case of an error skips operation
		Matrix& operator |=(const Matrix& other)& {
			if (matrix_.size() != other.count_strings()) {
				std::cout << "ERROR::MATRIX::OPERATOR|=(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			*this = *this | other;
			return *this;
		}

		Matrix operator -() const {
			Matrix result = *this;
			for (size_t i = 0; i < matrix_.size(); i++) {
				result[i] *= -1;
			}

			return result;
		}

		// In case of an error skips operation
		Matrix operator +(const Matrix& other) const {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				std::cout << "ERROR::MATRIX::OPERATOR+(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			Matrix result = *this;
			for (size_t i = 0; i < matrix_.size(); i++) {
				result[i] += other[i];
			}

			return result;
		}

		// In case of an error skips operation
		Matrix operator -(const Matrix& other) const {
			if (matrix_.size() != other.count_strings() || count_columns() != other.count_columns()) {
				std::cout << "ERROR::MATRIX::OPERATOR-(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			return *this + (-other);
		}

		Matrix operator *(double other) const {
			Matrix result = *this;
			for (size_t i = 0; i < matrix_.size(); i++) {
				result[i] *= other;
			}

			return result;
		}

		// In case of an error skips operation
		Matrix operator *(const Matrix& other) const {
			if (count_columns() != other.count_strings()) {
				std::cout << "ERROR::MATRIX::OPERATOR*(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
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

		// In case of an error skips operation
		Vect3 operator *(const Vect3& other) const {
			if (matrix_.size() != 4 || matrix_[0].size() != 4) {
				std::cout << "ERROR::MATRIX::OPERATOR*(VECT3)\n" << "Invalid matrix size.\n\n";
				return other;
			}

			Vect3 result;
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					result[i] += matrix_[i][j] * other[j];
				}
				result[i] += matrix_[i][3];
			}
			return result;
		}

		// In case of an error skips operation
		Matrix operator /(double other) const {
			if (equality(other, 0.0, eps_)) {
				std::cout << "ERROR::MATRIX::OPERATOR/(DOUBLE)\n" << "Division by zero.\n\n";
				return *this;
			}

			return *this * (1.0 / other);
		}

		// In case of an error skips operation
		Matrix operator ^(uint64_t other) const {
			if (matrix_.size() != count_columns()) {
				std::cout << "ERROR::MATRIX::OPERATOR^(UINT64_T)\n" << "Invalid matrix size.\n\n";
				return *this;
			}

			return binary_exponentiation(*this, other, one_matrix(matrix_.size()));
		}

		// In case of an error skips operation
		Matrix operator |(const Matrix& other) const {
			if (matrix_.size() != other.count_strings()) {
				std::cout << "ERROR::MATRIX::OPERATOR|(MATRIX)\n" << "Invalid matrix sizes.\n\n";
				return *this;
			}

			Matrix result(matrix_.size(), count_columns() + other.count_columns(), *this);
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < other.count_columns(); ++j) {
					result[i][count_columns() + j] = other[i][j];
				}
			}
			return result;
		}

		// In case of an error returns the original matrix
		Matrix get_submatrix(size_t line, size_t column, size_t height, size_t width) const {
			if (line + height > matrix_.size() || height + width > count_columns()) {
				std::cout << "ERROR::MATRIX::GET_SUB_MATRIX\n" << "Invalid submatrix size.\n\n";
				return *this;
			}

			Matrix result(height, width);
			for (size_t i = 0; i < height; ++i) {
				for (size_t j = 0; j < width; ++j) {
					result[i][j] = matrix_[line + i][column + j];
				}
			}
			return result;
		}

		Matrix get_improved_step_view() const {
			Matrix result = *this;
			for (size_t j = 0, i0 = 0; j < count_columns() && i0 < matrix_.size(); ++j) {
				size_t k = matrix_.size();
				for (size_t i = i0; i < matrix_.size(); ++i) {
					if (!equality(result[i][j], 0.0, eps_)) {
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

		Matrix transpose() const {
			Matrix result(count_columns(), matrix_.size());
			for (size_t i = 0; i < matrix_.size(); ++i) {
				for (size_t j = 0; j < matrix_[i].size(); ++j) {
					result[j][i] = matrix_[i][j];
				}
			}
			return result;
		}

		// In case of an error skips operation
		Matrix inverse() const {
			if (matrix_.size() != count_columns()) {
				std::cout << "ERROR::MATRIX::INVERSE\n" << "Not a square matrix.\n\n";
				return *this;
			}

			Matrix result = (*this | one_matrix(matrix_.size())).get_improved_step_view();
			if (result.get_submatrix(0, 0, matrix_.size(), matrix_.size()) != one_matrix(matrix_.size())) {
				std::cout << "ERROR::MATRIX::INVERSE\n" << "The matrix is not invertible.\n\n";
				return *this;
			}
			return result.get_submatrix(0, matrix_.size(), matrix_.size(), matrix_.size());
		}

		size_t count_strings() const {
			return matrix_.size();
		}

		size_t count_columns() const {
			if (matrix_.empty()) {
				return 0;
			}
			return matrix_[0].size();
		}

		// In case of an error returns null vector
		Matrix solve_equation(const Matrix& value) const {
			if (value.count_columns() != 1 || matrix_.size() != value.count_strings()) {
				std::cout << "ERROR::MATRIX::SOLVE_EQUATION\n" << "Invalid matrix size.\n\n";
				return Matrix(count_columns(), 1, 0);
			}

			Matrix isv_matrix = (*this | Matrix(value)).get_improved_step_view();
			Matrix result(count_columns(), 1, 0);
			for (size_t i = 0, j = 0; i < matrix_.size(); ++i) {
				for (; j <= count_columns() && !equality(isv_matrix[i][j], 1.0, eps_); ++j) {}

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

		static Matrix one_matrix(size_t size) {
			Matrix result(size, size, 0);
			for (size_t i = 0; i < size; ++i) {
				result[i][i] = 1;
			}
			return result;
		}

		static Matrix scale_matrix(const Vect3& scale) {
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
		
		static Matrix translation_matrix(const Vect3& translation) {
			return Matrix({
				{ 1.0, 0.0, 0.0, translation.x },
				{ 0.0, 1.0, 0.0, translation.y },
				{ 0.0, 0.0, 1.0, translation.z },
				{ 0.0, 0.0, 0.0,           1.0 },
			});
		}
		
		// In case of an error returns identity matrix
		static Matrix rotation_matrix(const Vect3& axis, double angle) {
			if (equality(axis.length(), 0.0)) {
				std::cout << "ERROR::MATRIX::ROTATION_MATRIX\n" << "The axis vector has zero length.\n\n";
				return one_matrix(4);
			}

			Vect3 norm_axis = axis.normalize();
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
	};

	std::istream& operator>>(std::istream& fin, Matrix& matrix) {
		size_t count_strings, count_columns;
		fin >> count_strings >> count_columns;

		matrix = Matrix(count_strings, count_columns);
		for (size_t j = 0; j < count_columns; ++j) {
			for (size_t i = 0; i < count_strings; ++i) {
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

	Matrix operator *(double value, const Matrix& matrix) {
		return matrix * value;
	}
}
