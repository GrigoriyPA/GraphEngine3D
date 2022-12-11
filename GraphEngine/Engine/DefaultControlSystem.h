#pragma once

#include "Camera.h"


namespace gre {
    // Use the mouse and 'Q', 'E' to rotate camera; Use 'W', 'A', 'S', 'D', 'LeftAlt', 'Space' to move camera; Use 'LeftShift' to bust camera
    class DefaultControlSystem : public ControlSystem {
        uint8_t active_state_ = 0;
        sf::Vector2i mouse_position_ = sf::Vector2i(0, 0);

    public:
        double sensitivity = 0.001;
        double speed = 3.0;
        double rotation_speed = 2.0;
        double speed_delt = 2.0;

        void switch_active(sf::RenderWindow* window) override {
            if (active_state_ == 0) {
                window->setMouseCursorVisible(false);
                mouse_position_ = sf::Mouse::getPosition();
                sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2 + window->getPosition().x, window->getSize().y / 2 + window->getPosition().y));
                active_state_ = 1;
            } else {
                window->setMouseCursorVisible(true);
                sf::Mouse::setPosition(mouse_position_);
                active_state_ = 0;
            }
        }

        void on_event(Camera& camera, const sf::Event& event, sf::RenderWindow* window) override {
            if (active_state_ == 0) {
                return;
            }

            switch (event.type) {
            case sf::Event::MouseMoved:
                if (active_state_ == 2) {
                    camera.rotate(camera.get_vertical(), (event.mouseMove.x - window->getSize().x / 2.0) * sensitivity);
                    camera.rotate(camera.get_horizont(), (event.mouseMove.y - window->getSize().y / 2.0) * sensitivity);
                    sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2 + window->getPosition().x, window->getSize().y / 2 + window->getPosition().y));
                } else if (active_state_ == 1) {
                    active_state_ = 2;
                }
                break;

            default:
                break;
            }
        }

        void on_update(Camera& camera, double delta_time, sf::RenderWindow* window) override {
            if (active_state_ == 0) {
                return;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                camera.rotate(camera.get_direction(), -rotation_speed * delta_time);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                camera.rotate(camera.get_direction(), rotation_speed * delta_time);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                delta_time *= speed_delt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                camera.position += speed * delta_time * camera.get_horizont();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                camera.position -= speed * delta_time * camera.get_horizont();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                camera.position += speed * delta_time * camera.get_direction();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                camera.position -= speed * delta_time * camera.get_direction();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                camera.position += speed * delta_time * camera.get_vertical();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
                camera.position -= speed * delta_time * camera.get_vertical();
            }
        }
    };
}
