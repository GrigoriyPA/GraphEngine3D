#pragma once

#include "Functions.h"


namespace eng {
	class MatrixLine {
		friend class Matrix;

		std::vector<double> line_;

		template <typename T>  // Casts required: double(T)
		MatrixLine(const std::initializer_list<T>& init) {
			line_.reserve(init.size());
			for (const T& element : init) {
				line_.push_back(static_cast<double>(element));
			}
		}

		template <typename T>  // Casts required: double(T)
		explicit MatrixLine(const std::vector<T>& init) {
			line_.reserve(init.size());
			for (const T& element : init) {
				line_.push_back(static_cast<double>(element));
			}
		}

		explicit MatrixLine(size_t count_columns, double value = 0.0) noexcept {
			line_.resize(count_columns, value);
		}

	public:
		double& operator[](size_t index) {
			if (line_.size() <= index) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return line_[index];
		}

		const double& operator[](size_t index) const {
			if (line_.size() <= index) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid index.\n\n");
			}

			return line_[index];
		}

		bool operator==(const MatrixLine& other) const noexcept {
			if (line_.size() != other.line_.size()) {
				return false;
			}

			for (size_t i = 0; i < line_.size(); ++i) {
				if (!equality(line_[i], other[i])) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const MatrixLine& other) const noexcept {
			return !(*this == other);
		}

		MatrixLine& operator+=(const MatrixLine& other)& {
			if (line_.size() != other.size()) {
				throw EngInvalidArgument(__FILE__, __LINE__, "operator+=, invalid line sizes.\n\n");
			}

			*this = *this + other;
			return *this;
		}

		MatrixLine& operator-=(const MatrixLine& other)& {
			if (line_.size() != other.size()) {
				throw EngInvalidArgument(__FILE__, __LINE__, "operator-=, invalid line sizes.\n\n");
			}

			*this = *this - other;
			return *this;
		}

		MatrixLine& operator*=(double other) & noexcept {
			*this = *this * other;
			return *this;
		}

		MatrixLine& operator/=(double other)& {
			if (equality(other, 0.0)) {
				throw EngDomainError(__FILE__, __LINE__, "operator/=, division by zero.\n\n");
			}

			*this = *this * (1.0 / other);
			return *this;
		}

		MatrixLine operator-() const noexcept {
			return *this * -1;
		}

		MatrixLine operator+(const MatrixLine& other) const {
			if (line_.size() != other.size()) {
				throw EngInvalidArgument(__FILE__, __LINE__, "operator+, invalid line sizes.\n\n");
			}

			MatrixLine result = *this;
			for (size_t i = 0; i < other.size(); ++i) {
				result[i] += other[i];
			}

			return result;
		}

		MatrixLine operator-(const MatrixLine& other) const {
			if (line_.size() != other.size()) {
				throw EngInvalidArgument(__FILE__, __LINE__, "operator-, invalid line sizes.\n\n");
			}

			MatrixLine result = *this;
			for (size_t i = 0; i < other.size(); ++i) {
				result[i] -= other[i];
			}

			return result;
		}

		MatrixLine operator*(double other) const noexcept {
			MatrixLine result = *this;
			for (double& element : result.line_) {
				element *= other;
			}
			return result;
		}

		MatrixLine operator/(double other) const {
			if (equality(other, 0.0)) {
				throw EngDomainError(__FILE__, __LINE__, "operator/, division by zero.\n\n");
			}

			return *this * (1.0 / other);
		}

		double operator*(const MatrixLine& other) const {
			if (line_.size() != other.size()) {
				throw EngInvalidArgument(__FILE__, __LINE__, "operator*, invalid line sizes.\n\n");
			}

			double result = 0;
			for (size_t i = 0; i < line_.size(); ++i) {
				result += line_[i] * other[i];
			}
			return result;
		}

		double& back() {
			if (line_.empty()) {
				throw EngOutOfRange(__FILE__, __LINE__, "back, called from empty line.\n\n");
			}

			return line_.back();
		}

		const double& back() const {
			if (line_.empty()) {
				throw EngOutOfRange(__FILE__, __LINE__, "back, called from empty line.\n\n");
			}

			return line_.back();
		}

		size_t size() const noexcept {
			return line_.size();
		}

		bool empty() const noexcept {
			return line_.size();
		}

		void swap(MatrixLine& other) noexcept {
			if (line_.size() != other.size()) {
				throw EngInvalidArgument(__FILE__, __LINE__, "swap, invalid line sizes.\n\n");
			}

			std::swap(line_, other.line_);
		}
	};

	MatrixLine operator*(double value, const MatrixLine& matrix) noexcept {
		return matrix * value;
	}
}
