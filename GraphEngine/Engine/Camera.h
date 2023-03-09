#pragma once

#include "../GraphicClasses/Shader.h"


namespace gre {
    class Camera;

    // Methods for overloading: switch_active, on_event, on_update
    class ControlSystem {
    public:
        virtual void switch_active(sf::RenderWindow* window) {
        }

        virtual void on_event(Camera& camera, const sf::Event& event, sf::RenderWindow* window) {
        }

        virtual void on_update(Camera& camera, double delta_time, sf::RenderWindow* window) {
        }

        virtual ~ControlSystem() {
        }
    };

    class Camera {
        class FPS_counter {
            double fps_sum_ = 0.0;
            double time_elapsed_ = 0.0;
            double minimal_update_time_ = 0.25;
            uint64_t current_fps_ = 0;
            uint64_t number_of_flips_ = 0;

            sf::Clock timer_;

            void check_fps_value() noexcept {
                if (number_of_flips_ == 0 || time_elapsed_ < minimal_update_time_) {
                    return;
                }

                current_fps_ = static_cast<uint64_t>(fps_sum_ / static_cast<double>(number_of_flips_));
                fps_sum_ = 0;
                time_elapsed_ = 0;
                number_of_flips_ = 0;
            }

        public:
            void set_minimal_update_time(double minimal_update_time) {
#ifdef _DEBUG
                if (minimal_update_time < 0.0) {
                    throw GreInvalidArgument(__FILE__, __LINE__, "set_minimal_update_time, invalid minimal update time value.");
                }
#endif // _DEBUG

                minimal_update_time_ = minimal_update_time;
                check_fps_value();
            }

            double get_minimal_update_time() const noexcept {
                return minimal_update_time_;
            }

            uint64_t get_fps() const noexcept {
                return current_fps_;
            }

            double update() {
                double delta_time = timer_.restart().asSeconds();
                fps_sum_ += 1.0 / delta_time;
                time_elapsed_ += delta_time;
                ++number_of_flips_;

                check_fps_value();

                return delta_time;
            }
        };

        Vec2 check_point_ = Vec2(0.5);
        Matrix4x4 change_matrix_ = Matrix4x4::one_matrix();

        double fov_;
        double min_distance_;
        double max_distance_;
        Vec2 viewport_size_;
        Vec2 viewport_position_;
        Vec3 direction_;
        Vec3 horizont_;
        Vec3 last_position_;
        Matrix4x4 projection_;
        ControlSystem* control_system_;
        FPS_counter fps_counter_;
        sf::RenderWindow* window_;

        void set_projection_matrix() {
#ifdef _DEBUG
            if (equality(tan(fov_ / 2.0), 0.0) || equality(max_distance_, min_distance_) || equality(max_distance_ + min_distance_, 0.0) || equality(viewport_size_.y, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "set_projection_matrix, invalid matrix settings.\n\n");
            }
#endif // _DEBUG

            projection_ = Matrix4x4::scale_matrix(Vec3(1.0 / tan(fov_ / 2.0), viewport_size_.x / (viewport_size_.y * tan(fov_ / 2.0)), (max_distance_ + min_distance_) / (max_distance_ - min_distance_)));
            projection_ *= Matrix4x4::translation_matrix(Vec3(0.0, 0.0, -2.0 * max_distance_ * min_distance_ / (max_distance_ + min_distance_)));
            projection_[3][3] = 0.0;
            projection_[3][2] = 1.0;
        }

    public:
        Vec3 position;

        Camera(sf::RenderWindow* window, ControlSystem* control_system) {
            fov_ = PI / 2.0;
            min_distance_ = 0.1;
            max_distance_ = 10.0;

            direction_ = Vec3(0.0, 0.0, 1.0);
            last_position_ = Vec3(0.0);
            control_system_ = control_system;
            window_ = window;

            viewport_size_ = Vec2(window->getSize().x, window->getSize().y);
            horizont_ = direction_.horizont();
            viewport_position_ = Vec2(0.0);
            position = last_position_;

            set_projection_matrix();
        }

        Camera(sf::RenderWindow* window, ControlSystem* control_system, const Vec2& viewport_position, const Vec2& viewport_size, const Vec3& position, const Vec3& direction, double fov, double min_distance, double max_distance) {
#ifdef _DEBUG
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
#else
            direction_ = direction.normalize();
#endif // _DEBUG

            fov_ = fov;
            min_distance_ = min_distance;
            max_distance_ = max_distance;
            viewport_size_ = viewport_size;
            control_system_ = control_system;
            window_ = window;
            viewport_position_ = viewport_position;
            this->position = position;

            horizont_ = direction.horizont();
            last_position_ = position;

            set_projection_matrix();
        }

        // POST shader expected
        void set_viewport(const Shader& shader) const {
            glViewport(static_cast<GLint>(viewport_position_.x), static_cast<GLint>(window_->getSize().y - viewport_size_.y - viewport_position_.y), static_cast<GLsizei>(viewport_size_.x), static_cast<GLsizei>(viewport_size_.y));
#ifdef _DEBUG
            check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

            shader.set_uniform_f("screen_texture_size", static_cast<GLfloat>(viewport_size_.x / window_->getSize().x), static_cast<GLfloat>(viewport_size_.y / window_->getSize().y));
        }

        // MAIN shader expected
        void set_uniforms(const Shader& shader) const {
            glViewport(0, 0, static_cast<GLsizei>(viewport_size_.x), static_cast<GLsizei>(viewport_size_.y));
#ifdef _DEBUG
            check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

            shader.set_uniform_f("check_point", static_cast<GLfloat>(check_point_.x * viewport_size_.x), static_cast<GLfloat>(check_point_.y * viewport_size_.y));
            shader.set_uniform_f("view_pos", position);
            shader.set_uniform_matrix("view", get_view_matrix());
            shader.set_uniform_matrix("projection", projection_);
        }
        
        // position_on_width, position_on_height - proportions relative to overall size
        void set_check_point(double position_on_width, double position_on_height) {
            set_check_point(Vec2(position_on_width, position_on_height));
        }

        void set_check_point(const Vec2& point) {
#ifdef _DEBUG
            if (point.x < 0.0 || 1.0 < point.x || point.y < 0.0 || 1.0 < point.y) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_check_point, invalid point coordinate.\n\n");
            }
#endif // _DEBUG

            check_point_ = Vec2(point.x, 1.0 - point.y);
        }

        void set_control_system(ControlSystem* control_system) noexcept {
            control_system_ = control_system;
        }

        void set_fov(double fov) {
#ifdef _DEBUG
            if (less_equality(fov, 0.0) || less_equality(PI, fov)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_fov, invalid FOV value.\n\n");
            }
#endif // _DEBUG

            fov_ = fov;
            set_projection_matrix();
        }

        void set_distance(double min_distance, double max_distance) {
#ifdef _DEBUG
            if (less_equality(min_distance, 0.0) || less_equality(max_distance, min_distance)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_distance, invalid distance value.\n\n");
            }
#endif // _DEBUG

            min_distance_ = min_distance;
            max_distance_ = max_distance;
            set_projection_matrix();
        }

        // Final image size in pixels on window
        void set_viewport_size(double viewport_width, double viewport_height) {
            set_viewport_size(Vec2(viewport_width, viewport_height));
        }

        void set_viewport_size(const Vec2& viewport_size) {
#ifdef _DEBUG
            if (less_equality(viewport_size.x, 0.0) || less_equality(viewport_size.y, 0.0)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_viewport_size, invalid viewport size.\n\n");
            }
#endif // _DEBUG

            viewport_size_ = viewport_size;
            set_projection_matrix();
        }

        // Final image position in pixels on window
        void set_viewport_position(double viewport_position_on_width, double viewport_position_on_height) noexcept {
            set_viewport_position(Vec2(viewport_position_on_width, viewport_position_on_height));
        }

        void set_viewport_position(const Vec2& viewport_position) noexcept {
            viewport_position_ = viewport_position;
        }

        void set_direction(const Vec3& direction) {
#ifdef _DEBUG
            try {
                direction_ = direction.normalize();
            }
            catch (GreDomainError) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_direction, the direction vector has zero length.\n\n");
            }
#else
            direction_ = direction.normalize();
#endif // _DEBUG

            horizont_ = direction.horizont();
        }

        void set_minimal_fps_update_time(double minimal_update_time) {
#ifdef _DEBUG
            if (minimal_update_time < 0.0) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_minimal_fps_update_time, invalid minimal update time value.");
            }
#endif // _DEBUG

            fps_counter_.set_minimal_update_time(minimal_update_time);
        }

        Vec2 get_check_point() const noexcept {
            return check_point_;
        }

        Matrix4x4 get_change_matrix() const noexcept {
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

        Vec2 get_viewport_position() const noexcept {
            return viewport_position_;
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

        Matrix4x4 get_projection_matrix() const noexcept {
            return projection_;
        }

        double get_minimal_fps_update_time() const noexcept {
            return fps_counter_.get_minimal_update_time();
        }

        uint64_t get_fps() const noexcept {
            return fps_counter_.get_fps();
        }

        Matrix4x4 get_view_matrix() const noexcept {
            return Matrix4x4(horizont_, get_vertical(), direction_).transpose() * Matrix4x4::translation_matrix(-position);
        }

        Vec3 get_change_vector(const Vec3& stable_point) const noexcept {
            return change_matrix_ * (stable_point - last_position_) + position - stable_point;
        }

        Vec3 get_change_vector() const noexcept {
            return position - last_position_;
        }

        void drop_change_matrix_state() noexcept {
            change_matrix_ = Matrix4x4::one_matrix();
            last_position_ = position;
        }

        void rotate(const Vec3& axis, double angle) {
            Matrix4x4 rotate = Matrix4x4::rotation_matrix(axis, angle);
            direction_ = rotate * direction_;
            horizont_ = rotate * horizont_;
            change_matrix_ = rotate * change_matrix_;
        }

        void switch_active() {
            control_system_->switch_active(window_);
        }

        void compute_event(const sf::Event& event) {
            control_system_->on_event(*this, event, window_);
        }

        double update() {
            double delta_time = fps_counter_.update();
            control_system_->on_update(*this, delta_time, window_);
            return delta_time;
        }

        Vec3 convert_point(double distance) const {
            double divisor = max_distance_ - distance * (max_distance_ - min_distance_);

#ifdef _DEBUG
            if (equality(divisor, 0.0)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "convert_point, invalid point coordinate.\n\n");
            }
            if (equality(viewport_size_.y, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "convert_point, invalid matrix settings.\n\n");
            }
#endif // _DEBUG

            double tg = tan(fov_ / 2.0);
            double z_coord = max_distance_* min_distance_ / divisor;
            return Matrix4x4(horizont_, get_vertical(), direction_) * Vec3(z_coord * tg * (2.0 * check_point_.x - 1.0), (z_coord * tg * viewport_size_.y / viewport_size_.x) * (2.0 * check_point_.y - 1.0), z_coord) + position;
        }
    };
}
