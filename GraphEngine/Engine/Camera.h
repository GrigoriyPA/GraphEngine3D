#pragma once

#include "../GraphicClasses/Shader.h"


namespace gre {
	class Camera {
		uint8_t active_state_ = 0;
		Vec2 check_point_ = Vec2(0.0);
		Matrix change_matrix_ = Matrix::one_matrix(4);

		double fov_;
		double min_distance_;
		double max_distance_;
		Vec2 viewport_size_;
		Vec3 direction_;
		Vec3 horizont_;
		Vec3 last_position_;
		Matrix projection_;
		sf::Vector2i mouse_position_;
		sf::RenderWindow* window_;

		void set_projection_matrix() {
			if (equality(tan(fov_ / 2.0), 0.0) || equality(max_distance_, min_distance_) || equality(max_distance_ + min_distance_, 0.0) || equality(viewport_size_.y, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "set_projection_matrix, invalid matrix settings.\n\n");
			}

			projection_ = Matrix::scale_matrix(Vec3(1.0 / tan(fov_ / 2.0), viewport_size_.x / (viewport_size_.y * tan(fov_ / 2.0)), (max_distance_ + min_distance_) / (max_distance_ - min_distance_)));
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

		Vec2 viewport_position;
		Vec3 position;

		explicit Camera(sf::RenderWindow* window) : projection_(4, 4) {
			fov_ = PI / 2.0;
			min_distance_ = 1.0;
			max_distance_ = 2.0;

			direction_ = Vec3(0, 0, 1);
			last_position_ = Vec3(0.0, 0.0, 0.0);
			window_ = window;

			viewport_size_ = Vec2(window->getSize().x, window->getSize().y);
			horizont_ = direction_.horizont();
			viewport_position = Vec2(0.0);
			position = last_position_;

			set_projection_matrix();
		}

		Camera(sf::RenderWindow* window, const Vec2& viewport_position, const Vec2& viewport_size, const Vec3& position, const Vec3& direction, double fov, double min_distance, double max_distance) : projection_(4, 4) {
			if (less_equality(fov, 0.0) || less_equality(PI, fov)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Camera, invalid FOV value.\n\n");
			}
			if (less_equality(min_distance, 0.0) || less_equality(max_distance, min_distance)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Camera, invalid distance value.\n\n");
			}
			if (less_equality(viewport_size.x, 0.0) || less_equality(viewport_size.y, 0.0)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Camera, invalid viewport size.\n\n");
			}
			
			try {
				direction_ = direction.normalize();
			}
			catch (GreDomainError) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Camera, the direction vector has zero length.\n\n");
			}

			fov_ = fov;
			min_distance_ = min_distance;
			max_distance_ = max_distance;
			viewport_size_ = viewport_size;
			window_ = window;
			this->viewport_position = viewport_position;
			this->position = position;

			horizont_ = direction.horizont();
			last_position_ = position;

			set_projection_matrix();
		}

		void set_viewport(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::POST) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_viewport, invalid shader type.\n\n");
			}

			glViewport(static_cast<GLint>(viewport_position.x), static_cast<GLint>(window_->getSize().y - viewport_size_.y - viewport_position.y), static_cast<GLsizei>(viewport_size_.x), static_cast<GLsizei>(viewport_size_.y));
			check_gl_errors(__FILE__, __LINE__, __func__);

			shader.set_uniform_f("screen_texture_size", viewport_size_.x / window_->getSize().x, viewport_size_.y / window_->getSize().y);
		}

		void set_uniforms(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
			}

			glViewport(0, 0, static_cast<GLsizei>(viewport_size_.x), static_cast<GLsizei>(viewport_size_.y));
			check_gl_errors(__FILE__, __LINE__, __func__);

			
			shader.set_uniform_f("check_point", static_cast<GLfloat>(check_point_.x * viewport_size_.x), static_cast<GLfloat>(check_point_.y * viewport_size_.y));
			shader.set_uniform_f("view_pos", position);
			shader.set_uniform_matrix("view", get_view_matrix());
			shader.set_uniform_matrix("projection", projection_);
		}

		Camera& set_check_point(const Vec2& point) {
			if (point.x < 0.0 || 1.0 < point.x || point.y < 0.0 || 1.0 < point.y) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_check_point, invalid point coordinate.\n\n");
			}

			check_point_ = Vec2(point.x, 1.0 - point.y);
			return *this;
		}

		Camera& set_fov(double fov) {
			if (less_equality(fov, 0.0) || less_equality(PI, fov)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_fov, invalid FOV value.\n\n");
			}

			fov_ = fov;
			set_projection_matrix();
			return *this;
		}

		Camera& set_distance(double min_distance, double max_distance) {
			if (less_equality(min_distance, 0.0) || less_equality(max_distance, min_distance)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_distance, invalid distance value.\n\n");
			}

			min_distance_ = min_distance;
			max_distance_ = max_distance;
			set_projection_matrix();
			return *this;
		}

		Camera& set_viewport_size(const Vec2& viewport_size) {
			if (less_equality(viewport_size.x, 0.0) || less_equality(viewport_size.y, 0.0)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_viewport_size, invalid viewport size.\n\n");
			}

			viewport_size_ = viewport_size;
			set_projection_matrix();
			return *this;
		}

		Camera& set_direction(const Vec3& direction) {
			try {
				direction_ = direction.normalize();
			}
			catch (GreDomainError) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_direction, the direction vector has zero length.\n\n");
			}

			horizont_ = direction.horizont();
			return *this;
		}

		Vec2 get_check_point() const noexcept {
			return check_point_;
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

		Vec2 get_viewport_size() const noexcept {
			return viewport_size_;
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
			catch (GreInvalidArgument) {
				throw GreInvalidArgument(__FILE__, __LINE__, "rotate, the axis vector has zero length.\n\n");
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

		Vec3 convert_point(double distance) const {
			double divisor = max_distance_ - distance * (max_distance_ - min_distance_);
			if (equality(divisor, 0.0)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "convert_point, invalid point coordinate.\n\n");
			}

			if (equality(viewport_size_.y, 0.0)) {
				throw GreDomainError(__FILE__, __LINE__, "convert_point, invalid matrix settings.\n\n");
			}

			double tg = tan(fov_ / 2.0);
			double z_coord = max_distance_* min_distance_ / divisor;
			return Matrix(horizont_, get_vertical(), direction_) * Vec3(z_coord * tg * (2.0 * check_point_.x - 1.0), (z_coord * tg * viewport_size_.y / viewport_size_.x) * (2.0 * check_point_.y - 1.0), z_coord) + position;
		}
	};
}
