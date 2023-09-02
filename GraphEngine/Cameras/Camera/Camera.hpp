#pragma once

#include "../../GraphicClasses/graphic_classes.h"


// Camera and control system
namespace gre {
    class Camera;

    // Camera control system
    class ControlSystem {
    public:
        // Make camera inactive/active
        virtual void switch_active(sf::RenderWindow* window);

        // Compute window event
        virtual void on_event(Camera& camera, const sf::Event& event, sf::RenderWindow* window);

        // Compute time elapsed
        virtual void on_update(Camera& camera, double delta_time, sf::RenderWindow* window);

        virtual ~ControlSystem();
    };

    // Camera for 3D scene 
    class Camera {
        class CounterFPS {
            double fps_sum_ = 0.0;
            double time_elapsed_ = 0.0;
            double minimal_update_time_ = 0.25;
            uint64_t current_fps_ = 0;
            uint64_t number_of_flips_ = 0;

            sf::Clock timer_;

            void check_fps_value() noexcept;

        public:
            // Seters
            void set_minimal_update_time(double minimal_update_time);

            // Getters
            double get_minimal_update_time() const noexcept;

            uint64_t get_fps() const noexcept;

            // Update counter state
            double update();
        };

        Vec2 check_point_ = Vec2(0.5);
        Matrix4x4 change_matrix_ = Matrix4x4::one_matrix();

        double fov_;
        double min_distance_;
        double max_distance_;
        Vec2 viewport_size_;
        Vec2 viewport_position_;
        Vec3 direction_;
        Vec3 horizon_;
        Vec3 last_position_;
        Matrix4x4 projection_;
        ControlSystem* control_system_;
        CounterFPS fps_counter_;
        sf::RenderWindow* window_;

        void set_projection_matrix();

    public:
        Vec3 position;

        // Constructors
        Camera(sf::RenderWindow* window, ControlSystem* control_system);

        Camera(sf::RenderWindow* window, ControlSystem* control_system, const Vec2& viewport_position, const Vec2& viewport_size, const Vec3& position, const Vec3& direction, double fov, double min_distance, double max_distance);

        // Seters
        
        // position_on_width, position_on_height - proportions relative to overall size
        void set_check_point(double position_on_width, double position_on_height);

        void set_check_point(const Vec2& point);

        void set_control_system(ControlSystem* control_system) noexcept;

        void set_fov(double fov);

        void set_distance(double min_distance, double max_distance);

        // Final image size in pixels on window
        void set_viewport_size(double viewport_width, double viewport_height);

        void set_viewport_size(const Vec2& viewport_size);

        // Final image position in pixels on window
        void set_viewport_position(double viewport_position_on_width, double viewport_position_on_height) noexcept;

        void set_viewport_position(const Vec2& viewport_position) noexcept;

        void set_direction(const Vec3& direction) noexcept;

        void set_minimal_fps_update_time(double minimal_update_time);

        // Getters
        Vec2 get_check_point() const noexcept;

        Matrix4x4 get_change_matrix() const noexcept;

        double get_fov() const noexcept;

        double get_min_distance() const noexcept;

        double get_max_distance() const noexcept;

        Vec2 get_viewport_size() const noexcept;

        Vec2 get_viewport_position() const noexcept;

        Vec3 get_direction() const noexcept;

        Vec3 get_horizon() const noexcept;

        Vec3 get_vertical() const noexcept;

        Vec3 get_last_position() const noexcept;

        Matrix4x4 get_projection_matrix() const noexcept;

        double get_minimal_fps_update_time() const noexcept;

        uint64_t get_fps() const noexcept;

        Matrix4x4 get_view_matrix() const noexcept;

        Vec3 get_change_vector(const Vec3& stable_point) const noexcept;

        Vec3 get_change_vector() const noexcept;

        // Camera modification
        void drop_change_matrix_state() noexcept;

        void rotate(const Vec3& axis, double angle);

        // Event handlers
        void switch_active();

        void compute_event(const sf::Event& event);

        double update();

        // Converting check point by final z-distance
        Vec3 convert_point(double distance) const;

        // Uploading into shader

        // POST shader expected
        void set_viewport(const Shader& shader) const;

        // MAIN shader expected
        void set_uniforms(const Shader& shader) const;
    };
}  // namespace gre
