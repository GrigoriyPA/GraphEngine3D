#pragma once

#include "Vect3.h"


namespace eng {
	std::vector < double > gauss(int n, int m, std::vector < std::vector < double > > mx, std::vector < double > ans, double eps = 0.000001) {
		for (int j = 0, i0 = 0; j < m; j++) {
			int k = -1;
			for (int i = i0; i < n; i++) {
				if (abs(mx[i][j]) > eps) {
					k = i;
					break;
				}
			}
			if (k == -1) {
				for (int i = 0; i < i0; i++)
					mx[i][j] = 0;
			} else {
				std::swap(mx[i0], mx[k]);
				std::swap(ans[i0], ans[k]);
				ans[i0] /= mx[i0][j];
				for (int i = m - 1; i >= j; i--)
					mx[i0][i] /= mx[i0][j];
				for (int i = 0; i < i0; i++) {
					ans[i] -= ans[i0] * mx[i][j];
					for (int ind = m - 1; ind >= j; ind--)
						mx[i][ind] -= mx[i0][ind] * mx[i][j];
				}
				for (int i = i0 + 1; i < n; i++) {
					ans[i] -= ans[i0] * mx[i][j];
					for (int ind = m - 1; ind >= j; ind--)
						mx[i][ind] -= mx[i0][ind] * mx[i][j];
				}
				i0++;
			}
		}
		std::vector < double > res(m, 0);
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				if (abs(mx[i][j]) > eps)
					res[j] = ans[i];
			}
		}
		return res;
	}

	class Matrix {
		double eps_ = 1e-5;

		int strings_, columns_;
		std::vector<std::vector<double>> mx;

	public:
		Matrix(std::vector < std::vector < double > > init = { {} }) {
			strings_ = init.size();
			columns_ = 0;
			if (strings_ > 0)
				columns_ = init[0].size();
			mx = init;
		}

		Matrix(Vect3 vx, Vect3 vy, Vect3 vz) {
			strings_ = 4;
			columns_ = 4;
			mx = {
				{ vx.x, vy.x, vz.x, 0 },
				{ vx.y, vy.y, vz.y, 0 },
				{ vx.z, vy.z, vz.z, 0 },
				{    0,    0,    0, 1 },
			};
		}

		Matrix(int s, int c, int vl) {
			if (s < 0 || c < 0) {
				std::cout << "ERROR::MATRIX::BUILDER\n" << "Negative matrix dimensions.\n";
				assert(0);
			}

			this->strings_ = s;
			this->columns_ = c;
			mx.resize(s, std::vector < double >(c, vl));
		}

		Matrix(int s, int c, Matrix init) {
			if (s < 0 || c < 0) {
				std::cout << "ERROR::MATRIX::BUILDER\n" << "Negative matrix dimensions.\n";
				assert(0);
			}

			this->strings_ = s;
			this->columns_ = c;
			mx.resize(s, std::vector < double >(c, 0));
			for (int i = 0; i < std::min(s, c); i++)
				mx[i][i] = 1;
			for (int i = 0; i < std::min(s, init.size_s()); i++) {
				for (int j = 0; j < std::min(c, init.size_c()); j++)
					mx[i][j] = init[i][j];
			}
		}

		std::vector < double >& operator[](int index) {
			if (index < 0 || mx.size() <= index) {
				std::cout << "ERROR::MATRIX::OPERATOR[]\n" << "Invalid index.\n";
				assert(0);
			}

			return mx[index];
		}

		bool operator ==(Matrix other) {
			if (strings_ != other.size_s() || columns_ != other.size_c())
				return false;

			for (int i = 0; i < strings_; i++) {
				for (int j = 0; j < columns_; j++) {
					if (abs(mx[i][j] - other[i][j]) > eps_)
						return false;
				}
			}

			return true;
		}

		bool operator !=(Matrix other) {
			return !(*this == other);
		}

		void operator *= (Matrix other) {
			*this = *this * other;
		}

		Matrix operator *(Matrix other) {
			if (columns_ != other.size_s()) {
				std::cout << "ERROR::MATRIX::OPERATOR*_MATRIX\n" << "Invalid matrix sizes.\n";
				assert(0);
			}

			other.transpose();
			std::vector < std::vector < double > > res(strings_, std::vector < double >(other.size_c(), 0));
			for (int i = 0; i < strings_; i++) {
				for (int j = 0; j < other.size_c(); j++) {
					for (int k = 0; k < columns_; k++)
						res[i][j] += mx[i][k] * other[j][k];
				}
			}

			return Matrix(res);
		}

		Vect3 operator *(Vect3 other) {
			if (strings_ != 4 || columns_ != 4) {
				std::cout << "ERROR::MATRIX::OPERATOR*_VECT3\n" << "Invalid matrix size.\n";
				assert(0);
			}

			Vect3 res;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++)
					res[i] += mx[i][j] * other[j];
				res[i] += mx[i][3];
			}

			return res;
		}

		Matrix transpose() {
			for (int i = 0; i < strings_; i++) {
				for (int j = i + 1; j < columns_; j++)
					std::swap(mx[i][j], mx[j][i]);
			}
			return *this;
		}

		Matrix inverse() {
			if (strings_ != columns_) {
				std::cout << "ERROR::MATRIX::INVERSE\n" << "Not a square matrix.\n";
				assert(0);
			}

			std::vector < double > ans(strings_ * columns_, 0);
			std::vector < std::vector < double > > mx_k(strings_ * columns_, std::vector < double >(strings_ * columns_, 0));
			for (int i = 0; i < strings_; i++) {
				ans[i * columns_ + i] = 1;
				for (int j = 0; j < columns_; j++) {
					for (int k = 0; k < strings_; k++)
						mx_k[i * columns_ + j][j * strings_ + k] = mx[i][k];
				}
			}

			std::vector < double > tmp = gauss(strings_ * columns_, strings_ * columns_, mx_k, ans);
			std::vector < std::vector < double > > res(strings_, std::vector < double >(columns_));
			for (int i = 0; i < strings_; i++) {
				for (int j = 0; j < columns_; j++)
					res[i][j] = tmp[j * strings_ + i];
			}

			return Matrix(res);
		}

		int size_s() {
			return strings_;
		}

		int size_c() {
			return columns_;
		}

		std::vector < float > value_vector() {
			std::vector < float > value(strings_ * columns_);
			for (int j = 0; j < columns_; j++) {
				for (int i = 0; i < strings_; i++)
					value[j * strings_ + i] = mx[i][j];
			}

			return value;
		}

		float* value_ptr() {
			float* value = new float[strings_ * columns_];
			for (int j = 0; j < columns_; j++) {
				for (int i = 0; i < strings_; i++)
					value[j * strings_ + i] = mx[i][j];
			}

			return value;
		}

		void print() {
			for (int i = 0; i < strings_; i++) {
				for (int j = 0; j < columns_; j++)
					std::cout << mx[i][j] << " ";
				std::cout << "\n";
			}
		}
	};

	Matrix one_matrix(int n) {
		if (n < 0) {
			std::cout << "ERROR::ONE_MATRIX\n" << "Negative matrix dimensions.\n";
			assert(0);
		}

		std::vector < std::vector < double > > res(n, std::vector < double >(n, 0));
		for (int i = 0; i < n; i++)
			res[i][i] = 1;

		return Matrix(res);
	}

	Matrix scale_matrix(Vect3 s) {
		return Matrix({
			{ s.x,   0,   0, 0 },
			{   0, s.y,   0, 0 },
			{   0,   0, s.z, 0 },
			{   0,   0,   0, 1 },
			});
	}

	Matrix scale_matrix(double s) {
		return Matrix({
			{ s, 0, 0, 0 },
			{ 0, s, 0, 0 },
			{ 0, 0, s, 0 },
			{ 0, 0, 0, 1 },
			});
	}

	Matrix trans_matrix(Vect3 trans) {
		return Matrix({
			{ 1, 0, 0, trans.x },
			{ 0, 1, 0, trans.y },
			{ 0, 0, 1, trans.z },
			{ 0, 0, 0, 1 },
			});
	}

	Matrix rotate_matrix(Vect3 axis, double angle) {
		if (axis.length() < 0.00001) {
			std::cout << "ERROR::ROTATE_MATRIX\n" << "The axis vector has zero length.\n";
			assert(0);
		}

		axis = axis.normalize();
		double c = cos(angle), s = sin(angle), x = axis.x, y = axis.y, z = axis.z;

		return Matrix({
			{     c + x * x * (1 - c), x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0 },
			{ y * x * (1 - c) + z * s,     c + y * y * (1 - c), y * z * (1 - c) - x * s, 0 },
			{ z * x * (1 - c) - y * s, z * y * (1 - c) + x * s,     c + z * z * (1 - c), 0 },
			{                       0,                       0,                       0, 1 },
			});
	}
}
