#pragma once

#include "../Camera/Camera.hpp"


// Default control system for all cameras in engine
namespace gre {
    // Use the mouse and 'Q', 'E' to rotate camera
    // Use 'W', 'A', 'S', 'D', 'LeftAlt', 'Space' to move camera
    // Use 'LeftShift' to bust camera
    class DefaultControlSystem : public ControlSystem {
        uint8_t active_state_ = 0;
        sf::Vector2i mouse_position_ = sf::Vector2i(0, 0);

    public:
        double sensitivity = 0.001;   // Mouse sensitivity
        double speed = 3.0;           // Camera speed
        double rotation_speed = 2.0;  // Camera rotation speed
        double speed_delt = 2.0;      // Camera speed delta when SHIFT pressed

        // Event handlers
        void switch_active(sf::RenderWindow* window) override;

        void on_event(Camera& camera, const sf::Event& event, sf::RenderWindow* window) override;

        void on_update(Camera& camera, double delta_time, sf::RenderWindow* window) override;
    };
}  // namespace gre
