#include "Camera.hpp"


// ControlSystem
namespace gre {
    void ControlSystem::switch_active(sf::RenderWindow * window) {
    }

    void ControlSystem::on_event(Camera& camera, const sf::Event& event, sf::RenderWindow* window) {
    }

    void ControlSystem::on_update(Camera& camera, double delta_time, sf::RenderWindow* window) {
    }

    ControlSystem::~ControlSystem() {
    }
}  // namespace gre


// Camera::CounterFPS
namespace gre {
    // Seters
    void Camera::CounterFPS::set_minimal_update_time(double minimal_update_time) {
        GRE_ENSURE(minimal_update_time >= 0.0, GreInvalidArgument, "invalid minimal update time");

        minimal_update_time_ = minimal_update_time;
        check_fps_value();
    }

    // Getters
    double Camera::CounterFPS::get_minimal_update_time() const noexcept {
        return minimal_update_time_;
    }

    uint64_t Camera::CounterFPS::get_fps() const noexcept {
        return current_fps_;
    }

    // Public methods
    double Camera::CounterFPS::update() {
        double delta_time = timer_.restart().asSeconds();
        fps_sum_ += 1.0 / delta_time;
        time_elapsed_ += delta_time;
        ++number_of_flips_;

        check_fps_value();

        return delta_time;
    }

    // Private methods
    void Camera::CounterFPS::check_fps_value() noexcept {
        if (number_of_flips_ == 0 || time_elapsed_ < minimal_update_time_) {
            return;
        }

        current_fps_ = static_cast<uint64_t>(fps_sum_ / static_cast<double>(number_of_flips_));
        fps_sum_ = 0;
        time_elapsed_ = 0;
        number_of_flips_ = 0;
    }
}  // namespace gre


// Camera
namespace gre {
    // Constructors
    Camera::Camera(sf::RenderWindow* window, ControlSystem* control_system) {
        fov_ = PI / 2.0;
        min_distance_ = 0.1;
        max_distance_ = 10.0;

        direction_ = Vec3(0.0, 0.0, 1.0);
        last_position_ = Vec3(0.0);
        control_system_ = control_system;
        window_ = window;

        viewport_size_ = Vec2(window->getSize().x, window->getSize().y);
        horizon_ = direction_.horizon();
        viewport_position_ = Vec2(0.0);
        position = last_position_;

        set_projection_matrix();
    }

    Camera::Camera(sf::RenderWindow* window, ControlSystem* control_system, const Vec2& viewport_position, const Vec2& viewport_size, const Vec3& position, const Vec3& direction, double fov, double min_distance, double max_distance) {
        GRE_ENSURE(!less_equality(fov, 0.0) && !less_equality(PI, fov), GreInvalidArgument, "invalid FOV value");
        GRE_ENSURE(!less_equality(min_distance, 0.0) && !less_equality(max_distance, min_distance), GreInvalidArgument, "invalid distance value");
        GRE_ENSURE(!less_equality(viewport_size.x, 0.0) && !less_equality(viewport_size.y, 0.0), GreInvalidArgument, "invalid viewport size");

        fov_ = fov;
        min_distance_ = min_distance;
        max_distance_ = max_distance;
        viewport_size_ = viewport_size;
        control_system_ = control_system;
        window_ = window;
        viewport_position_ = viewport_position;
        this->position = position;

        direction_ = direction.normalize();
        horizon_ = direction.horizon();
        last_position_ = position;

        set_projection_matrix();
    }

    // Seters

    // position_on_width, position_on_height - proportions relative to overall size
    void Camera::set_check_point(double position_on_width, double position_on_height) {
        set_check_point(Vec2(position_on_width, position_on_height));
    }

    void Camera::set_check_point(const Vec2& point) {
        GRE_ENSURE(0.0 <= point.x && point.x <= 1.0 && 0.0 <= point.y && point.y <= 1.0, GreInvalidArgument, "invalid point coordinate");

        check_point_ = Vec2(point.x, 1.0 - point.y);
    }

    void Camera::set_control_system(ControlSystem* control_system) noexcept {
        control_system_ = control_system;
    }

    void Camera::set_fov(double fov) {
        GRE_ENSURE(!less_equality(fov, 0.0) && !less_equality(PI, fov), GreInvalidArgument, "invalid FOV value");

        fov_ = fov;
        set_projection_matrix();
    }

    void Camera::set_distance(double min_distance, double max_distance) {
        GRE_ENSURE(!less_equality(min_distance, 0.0) && !less_equality(max_distance, min_distance), GreInvalidArgument, "invalid distance value");

        min_distance_ = min_distance;
        max_distance_ = max_distance;
        set_projection_matrix();
    }

    // Final image size in pixels on window
    void Camera::set_viewport_size(double viewport_width, double viewport_height) {
        set_viewport_size(Vec2(viewport_width, viewport_height));
    }

    void Camera::set_viewport_size(const Vec2& viewport_size) {
        GRE_ENSURE(!less_equality(viewport_size.x, 0.0) && !less_equality(viewport_size.y, 0.0), GreInvalidArgument, "invalid viewport size");

        viewport_size_ = viewport_size;
        set_projection_matrix();
    }

    // Final image position in pixels on window
    void Camera::set_viewport_position(double viewport_position_on_width, double viewport_position_on_height) noexcept {
        set_viewport_position(Vec2(viewport_position_on_width, viewport_position_on_height));
    }

    void Camera::set_viewport_position(const Vec2& viewport_position) noexcept {
        viewport_position_ = viewport_position;
    }

    void Camera::set_direction(const Vec3& direction) noexcept {
        direction_ = direction.normalize();
        horizon_ = direction.horizon();
    }

    void Camera::set_minimal_fps_update_time(double minimal_update_time) {
        GRE_ENSURE(minimal_update_time >= 0.0, GreInvalidArgument, "invalid minimal update time");

        fps_counter_.set_minimal_update_time(minimal_update_time);
    }

    // Getters
    Vec2 Camera::get_check_point() const noexcept {
        return check_point_;
    }

    Matrix4x4 Camera::get_change_matrix() const noexcept {
        return change_matrix_;
    }

    double Camera::get_fov() const noexcept {
        return fov_;
    }

    double Camera::get_min_distance() const noexcept {
        return min_distance_;
    }

    double Camera::get_max_distance() const noexcept {
        return max_distance_;
    }

    Vec2 Camera::get_viewport_size() const noexcept {
        return viewport_size_;
    }

    Vec2 Camera::get_viewport_position() const noexcept {
        return viewport_position_;
    }

    Vec3 Camera::get_direction() const noexcept {
        return direction_;
    }

    Vec3 Camera::get_horizon() const noexcept {
        return horizon_;
    }

    Vec3 Camera::get_vertical() const noexcept {
        return direction_ ^ horizon_;
    }

    Vec3 Camera::get_last_position() const noexcept {
        return last_position_;
    }

    Matrix4x4 Camera::get_projection_matrix() const noexcept {
        return projection_;
    }

    double Camera::get_minimal_fps_update_time() const noexcept {
        return fps_counter_.get_minimal_update_time();
    }

    uint64_t Camera::get_fps() const noexcept {
        return fps_counter_.get_fps();
    }

    Matrix4x4 Camera::get_view_matrix() const noexcept {
        return Matrix4x4(horizon_, get_vertical(), direction_).transpose() * Matrix4x4::translation_matrix(-position);
    }

    Vec3 Camera::get_change_vector(const Vec3& stable_point) const noexcept {
        return change_matrix_ * (stable_point - last_position_) + position - stable_point;
    }

    Vec3 Camera::get_change_vector() const noexcept {
        return position - last_position_;
    }

    // Camera modification
    void Camera::drop_change_matrix_state() noexcept {
        change_matrix_ = Matrix4x4::one_matrix();
        last_position_ = position;
    }

    void Camera::rotate(const Vec3& axis, double angle) {
        Matrix4x4 rotate = Matrix4x4::rotation_matrix(axis, angle);
        direction_ = rotate * direction_;
        horizon_ = rotate * horizon_;
        change_matrix_ = rotate * change_matrix_;
    }

    // Event handlers
    void Camera::switch_active() {
        control_system_->switch_active(window_);
    }

    void Camera::compute_event(const sf::Event& event) {
        control_system_->on_event(*this, event, window_);
    }

    double Camera::update() {
        double delta_time = fps_counter_.update();
        control_system_->on_update(*this, delta_time, window_);
        return delta_time;
    }

    // Converting check point by final z-distance
    Vec3 Camera::convert_point(double distance) const {
        double divisor = max_distance_ - distance * (max_distance_ - min_distance_);

        GRE_CHECK(!equality(divisor, 0.0), "invalid point coordinate");
        GRE_CHECK(!equality(viewport_size_.x, 0.0), "invalid matrix settings");

        double tg = tan(fov_ / 2.0);
        double z_coord = max_distance_ * min_distance_ / divisor;
        return Matrix4x4(horizon_, get_vertical(), direction_) * Vec3(z_coord * tg * (2.0 * check_point_.x - 1.0), (z_coord * tg * viewport_size_.y / viewport_size_.x) * (2.0 * check_point_.y - 1.0), z_coord) + position;
    }

    // Uploading into shader

    // POST shader expected
    void Camera::set_viewport(const Shader& shader) const {
        glViewport(static_cast<GLint>(viewport_position_.x), static_cast<GLint>(window_->getSize().y - viewport_size_.y - viewport_position_.y), static_cast<GLsizei>(viewport_size_.x), static_cast<GLsizei>(viewport_size_.y));
        GRE_CHECK_GL_ERRORS;

        shader.set_uniform_f("screen_texture_size", static_cast<GLfloat>(viewport_size_.x / window_->getSize().x), static_cast<GLfloat>(viewport_size_.y / window_->getSize().y));
    }

    // MAIN shader expected
    void Camera::set_uniforms(const Shader& shader) const {
        glViewport(0, 0, static_cast<GLsizei>(viewport_size_.x), static_cast<GLsizei>(viewport_size_.y));
        GRE_CHECK_GL_ERRORS;

        shader.set_uniform_f("check_point", static_cast<GLfloat>(check_point_.x * viewport_size_.x), static_cast<GLfloat>(check_point_.y * viewport_size_.y));
        shader.set_uniform_f("view_pos", position);
        shader.set_uniform_matrix("view", get_view_matrix());
        shader.set_uniform_matrix("projection", projection_);
    }

    // Private functions
    void Camera::set_projection_matrix() {
        GRE_CHECK(!equality(tan(fov_ / 2.0), 0.0) && !equality(max_distance_, min_distance_) && !equality(max_distance_ + min_distance_, 0.0) && !equality(viewport_size_.y, 0.0), "invalid matrix settings");

        projection_ = Matrix4x4::scale_matrix(Vec3(1.0 / tan(fov_ / 2.0), viewport_size_.x / (viewport_size_.y * tan(fov_ / 2.0)), (max_distance_ + min_distance_) / (max_distance_ - min_distance_)));
        projection_ *= Matrix4x4::translation_matrix(Vec3(0.0, 0.0, -2.0 * max_distance_ * min_distance_ / (max_distance_ + min_distance_)));
        projection_[3][3] = 0.0;
        projection_[3][2] = 1.0;
    }
}  // namespace gre
