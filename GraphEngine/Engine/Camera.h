#pragma once

#include "../CommonClasses/Matrix.h"


namespace eng {
	class Camera {
		inline static double eps_ = 1e-5;

		Matrix change_matrix_ = Matrix::one_matrix(4);

		double fov_;
		double min_distance_;
		double max_distance_;
		double screen_ratio_;
		Vect3 direction_;
		Vect3 horizont_;
		Vect3 last_position_;
		Matrix projection_;

		void set_projection_matrix() {
			if (equality(tan(fov_ / 2.0), 0.0, eps_) || equality(max_distance_, min_distance_, eps_) || equality(max_distance_ + min_distance_, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "set_projection_matrix, invalid matrix settings.\n\n");
			}

			projection_ = Matrix::scale_matrix(Vect3(1.0 / tan(fov_ / 2.0), screen_ratio_ / tan(fov_ / 2.0), (max_distance_ + min_distance_) / (max_distance_ - min_distance_)));
			projection_ *= Matrix::translation_matrix(Vect3(0.0, 0.0, -2.0 * max_distance_ * min_distance_ / (max_distance_ + min_distance_)));
			projection_[3][3] = 0.0;
			projection_[3][2] = 1.0;
		}

	public:
		// ...
		double sensitivity = 0.0;
		double speed = 0.0;
		double rotation_speed = 0.0;
		double speed_delt = 0.0;

		Vect3 position;

		Camera() : projection_(4, 4) {
			fov_ = PI / 2.0;
			min_distance_ = 1.0;
			max_distance_ = 2.0;
			screen_ratio_ = 1.0;

			direction_ = Vect3(0, 0, 1);
			last_position_ = Vect3(0.0, 0.0, 0.0);

			horizont_ = direction_.horizont();
			position = last_position_;

			set_projection_matrix();
		}

		// screen_ratio = screen_width / screen_height
		Camera(const Vect3& position, const Vect3& direction, double fov, double min_distance, double max_distance, double screen_ratio) : projection_(4, 4) {
			if (less_equality(fov, 0.0, eps_) || less_equality(PI, fov, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Camera, invalid FOV value.\n\n");
			}
			if (less_equality(min_distance, 0.0, eps_) || less_equality(max_distance, min_distance, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Camera, invalid distance value.\n\n");
			}
			if (less_equality(screen_ratio, 0.0, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Camera, invalid screen ratio value.\n\n");
			}
			
			try {
				direction_ = direction.normalize();
			}
			catch (EngDomainError) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Camera, the direction vector has zero length.\n\n");
			}

			fov_ = fov;
			min_distance_ = min_distance;
			max_distance_ = max_distance;
			screen_ratio_ = screen_ratio;
			this->position = position;

			horizont_ = direction.horizont();
			last_position_ = position;

			set_projection_matrix();
		}

		Camera& set_fov(double fov) {
			if (less_equality(fov, 0.0, eps_) || less_equality(PI, fov, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_fov, invalid FOV value.\n\n");
			}

			fov_ = fov;
			set_projection_matrix();
			return *this;
		}

		Camera& set_distance(double min_distance, double max_distance) {
			if (less_equality(min_distance, 0.0, eps_) || less_equality(max_distance, min_distance, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_distance, invalid distance value.\n\n");
			}

			min_distance_ = min_distance;
			max_distance_ = max_distance;
			set_projection_matrix();
			return *this;
		}

		// screen_ratio = screen_width / screen_height
		Camera& set_screen_ratio(double screen_ratio) {
			if (less_equality(screen_ratio, 0.0, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_screen_ratio, invalid screen ratio value.\n\n");
			}

			screen_ratio_ = screen_ratio;
			set_projection_matrix();
			return *this;
		}

		Camera& set_direction(const Vect3& direction) {
			try {
				direction_ = direction.normalize();
			}
			catch (EngDomainError) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_direction, the direction vector has zero length.\n\n");
			}

			horizont_ = direction.horizont();
			return *this;
		}

		Matrix get_change_matrix() const noexcept {
			return change_matrix_;
		}

		double get_fov() const noexcept {
			return fov_;
		}

		double get_min_distance() const noexcept {
			return min_distance_;
		}

		double get_max_distance() const noexcept {
			return max_distance_;
		}

		double get_screen_ratio() const noexcept {
			return screen_ratio_;
		}

		Vect3 get_direction() const noexcept {
			return direction_;
		}

		Vect3 get_horizont() const noexcept {
			return horizont_;
		}

		Vect3 get_vertical() const noexcept {
			return direction_ ^ horizont_;
		}

		Vect3 get_last_position() const noexcept {
			return last_position_;
		}

		Matrix get_projection_matrix() const noexcept {
			return projection_;
		}

		Matrix get_view_matrix() const noexcept {
			return Matrix(horizont_, get_vertical(), direction_).transpose() * Matrix::translation_matrix(-position);
		}

		Vect3 get_change_vector(const Vect3& stable_point) const noexcept {
			return change_matrix_ * (stable_point - last_position_) + position - stable_point;
		}

		Vect3 get_change_vector() const noexcept {
			return position - last_position_;
		}

		Camera& update() noexcept {
			change_matrix_ = Matrix::one_matrix(4);
			last_position_ = position;
			return *this;
		}

		Camera& rotate(const Vect3& axis, double angle) {
			try {
				Matrix rotate = Matrix::rotation_matrix(axis, angle);
				direction_ = rotate * direction_;
				horizont_ = rotate * horizont_;
				change_matrix_ = rotate * change_matrix_;
			}
			catch (EngInvalidArgument) {
				throw EngInvalidArgument(__FILE__, __LINE__, "rotate, the axis vector has zero length.\n\n");
			}
			return *this;
		}

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}
	};
}
