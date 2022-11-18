#pragma once

#include "../CommonClasses/Matrix.h"


namespace eng {
	class Camera {
		inline static double eps_ = 1e-5;

		uint8_t active_state_ = 0;
		Matrix change_matrix_ = Matrix::one_matrix(4);

		double fov_;
		double min_distance_;
		double max_distance_;
		double screen_ratio_;
		Vec3 direction_;
		Vec3 horizont_;
		Vec3 last_position_;
		Matrix projection_;
		sf::Vector2i mouse_position_;
		sf::RenderWindow* window_;

		void set_projection_matrix() {
			if (equality(tan(fov_ / 2.0), 0.0, eps_) || equality(max_distance_, min_distance_, eps_) || equality(max_distance_ + min_distance_, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "set_projection_matrix, invalid matrix settings.\n\n");
			}

			projection_ = Matrix::scale_matrix(Vec3(1.0 / tan(fov_ / 2.0), screen_ratio_ / tan(fov_ / 2.0), (max_distance_ + min_distance_) / (max_distance_ - min_distance_)));
			projection_ *= Matrix::translation_matrix(Vec3(0.0, 0.0, -2.0 * max_distance_ * min_distance_ / (max_distance_ + min_distance_)));
			projection_[3][3] = 0.0;
			projection_[3][2] = 1.0;
		}

	public:
		// ...
		double sensitivity = 0.0;
		double speed = 0.0;
		double rotation_speed = 0.0;
		double speed_delt = 0.0;

		Vec3 position;

		explicit Camera(sf::RenderWindow* window) : projection_(4, 4) {
			fov_ = PI / 2.0;
			min_distance_ = 1.0;
			max_distance_ = 2.0;
			screen_ratio_ = 1.0;

			direction_ = Vec3(0, 0, 1);
			last_position_ = Vec3(0.0, 0.0, 0.0);
			window_ = window;

			horizont_ = direction_.horizont();
			position = last_position_;

			set_projection_matrix();
		}

		// screen_ratio = screen_width / screen_height
		Camera(sf::RenderWindow* window, const Vec3& position, const Vec3& direction, double fov, double min_distance, double max_distance, double screen_ratio) : projection_(4, 4) {
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
			window_ = window;
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

		Camera& set_direction(const Vec3& direction) {
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

		Vec3 get_direction() const noexcept {
			return direction_;
		}

		Vec3 get_horizont() const noexcept {
			return horizont_;
		}

		Vec3 get_vertical() const noexcept {
			return direction_ ^ horizont_;
		}

		Vec3 get_last_position() const noexcept {
			return last_position_;
		}

		Matrix get_projection_matrix() const noexcept {
			return projection_;
		}

		Matrix get_view_matrix() const noexcept {
			return Matrix(horizont_, get_vertical(), direction_).transpose() * Matrix::translation_matrix(-position);
		}

		Vec3 get_change_vector(const Vec3& stable_point) const noexcept {
			return change_matrix_ * (stable_point - last_position_) + position - stable_point;
		}

		Vec3 get_change_vector() const noexcept {
			return position - last_position_;
		}

		Camera& update() noexcept {
			change_matrix_ = Matrix::one_matrix(4);
			last_position_ = position;
			return *this;
		}

		Camera& rotate(const Vec3& axis, double angle) {
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

		// ...
		void switch_active() noexcept {
			if (active_state_ == 0) {
				window_->setMouseCursorVisible(false);
				mouse_position_ = sf::Mouse::getPosition();
				sf::Mouse::setPosition(sf::Vector2i(window_->getSize().x / 2 + window_->getPosition().x, window_->getSize().y / 2 + window_->getPosition().y));
				active_state_ = 1;
			} else {
				window_->setMouseCursorVisible(true);
				sf::Mouse::setPosition(mouse_position_);
				active_state_ = 0;
			}
		}

		// ...
		void compute_event(const sf::Event& event) {
			if (active_state_ == 0) {
				return;
			}

			switch (event.type) {
				case sf::Event::MouseMoved:
					if (active_state_ == 2) {
						rotate(get_vertical(), (event.mouseMove.x - window_->getSize().x / 2.0) * sensitivity);
						rotate(horizont_, (event.mouseMove.y - window_->getSize().y / 2.0) * sensitivity);
						sf::Mouse::setPosition(sf::Vector2i(window_->getSize().x / 2 + window_->getPosition().x, window_->getSize().y / 2 + window_->getPosition().y));
					} else if (active_state_ == 1) {
						active_state_ = 2;
					}
					break;

				default:
					break;
			}
		}

		// ...
		void update(double delta_time) {
			if (active_state_ == 0) {
				return;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
				rotate(direction_, -rotation_speed * delta_time);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
				rotate(direction_, rotation_speed * delta_time);
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
				delta_time *= speed_delt;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				position += speed * delta_time * horizont_;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				position -= speed * delta_time * horizont_;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				position += speed * delta_time * direction_;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				position -= speed * delta_time * direction_;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
				position += speed * delta_time * get_vertical();
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
				position -= speed * delta_time * get_vertical();
			}
		}

		Vec3 convert_point(const Vec3& point) const {
			double divisor = max_distance_ - point.z * (max_distance_ - min_distance_);
			if (equality(divisor, 0.0, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "convert_point, invalid point coordinate.\n\n");
			}

			if (equality(screen_ratio_, 0.0, eps_)) {
				throw EngDomainError(__FILE__, __LINE__, "convert_point, invalid matrix settings.\n\n");
			}

			double tg = tan(fov_ / 2.0);
			return Matrix(horizont_, get_vertical(), direction_) * Vec3(tg * point.x, (tg / screen_ratio_) * point.y, max_distance_ * min_distance_ / divisor) + position;
		}

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}
	};
}
