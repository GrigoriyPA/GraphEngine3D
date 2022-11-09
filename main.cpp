#include "GraphEngine/Engine.h"
#include <chrono>
#include <cassert>
#include <iostream>
#include <stdio.h>
#include <string>
#include "config.h"
#include "Interface/Interface.h"
#include "InnerClasses/RenderingSequence.h"


void screenshot(sf::RenderWindow& window) {
    sf::Texture screen;
    screen.create(window.getSize().x, window.getSize().y);
    screen.update(window);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t cur_time = std::chrono::system_clock::to_time_t(now);

    char time[512];
    ctime_s(time, sizeof(time), &cur_time);
    std::string path = "Resources/Screenshots/" + std::string(time);
    path.pop_back();
    for (char& el : path) {
        if (el == ' ')
            el = '_';
        else if (el == ':')
            el = '-';
    }

    screen.copyToImage().saveToFile(path + ".png");
}


signed main() {
    try {
        sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "Editor", sf::Style::None);
        //window.setVerticalSyncEnabled(true);

        int window_width = window.getSize().x, window_height = window.getSize().y;

        sf::Texture cross_tex;
        cross_tex.loadFromFile("Interface/Resources/Textures/horizont_cross.png");
        cross_tex.setSmooth(true);

        sf::Texture circle_tex;
        circle_tex.loadFromFile("Interface/Resources/Textures/circle.png");
        circle_tex.setSmooth(true);

        sf::Sprite cross(cross_tex);
        cross.setScale(sf::Vector2f(INTERFACE_SIZE * eng::FI / cross_tex.getSize().x, INTERFACE_SIZE * eng::FI / cross_tex.getSize().y));
        cross.setPosition(sf::Vector2f(window_width / 2 - INTERFACE_SIZE * eng::FI / 2, window_height / 2 - INTERFACE_SIZE * eng::FI / 2));
        cross.setColor(INTERFACE_BORDER_COLOR);

        sf::Sprite circle(circle_tex);
        circle.setScale(sf::Vector2f(INTERFACE_SIZE * eng::FI / cross_tex.getSize().x, INTERFACE_SIZE * eng::FI / cross_tex.getSize().y));
        circle.setPosition(sf::Vector2f(window_width / 2 - INTERFACE_SIZE * eng::FI / 2, window_height / 2 - INTERFACE_SIZE * eng::FI / 2));
        circle.setColor(INTERFACE_ADD_COLOR);

        Interface window_interface(&window);

        eng::GraphEngine scene(&window);
        scene.set_clear_color(eng::Vect3(INTERFACE_MAIN_COLOR) / 255.0);
        scene.set_border_color(eng::Vect3(INTERFACE_ADD_COLOR) / 255.0);
        scene.camera.set_fov(FOV);
        scene.camera.set_distance(MIN_DIST, MAX_DIST);
        scene.camera.sensitivity = SENSITIVITY;
        scene.camera.speed = SPEED;
        scene.camera.rotation_speed = ROTATION_SPEED;
        scene.camera.speed_delt = SPEED_DELT;

        double ratio = scene.camera.get_screen_ratio();
        double angle = atan(tan(FOV / 2) * sqrt(1 + ratio * ratio));
        eng::SpotLight spot_light(eng::Vect3(0, 0, 0), eng::Vect3(0, 0, 1), angle, 1.1 * angle);
        spot_light.set_diffuse(eng::Vect3(0.6, 0.6, 0.6));
        spot_light.set_specular(eng::Vect3(0.8, 0.8, 0.8));
        spot_light.set_quadratic(0.1);
        scene.set_light(0, &spot_light);

        RenderingSequence render(&scene);

        int obj_id = scene.add_object(eng::GraphObject(1));
        eng::Mesh mesh(4);
        mesh.set_positions({
            eng::Vect3(-1, 0, 1),
            eng::Vect3(-1, 0, -1),
            eng::Vect3(1, 0, -1),
            eng::Vect3(1, 0, 1)
            }, true);
        mesh.material.set_ambient(eng::Vect3(0.5, 0.5, 0.5));
        mesh.material.set_diffuse(eng::Vect3(0.5, 0.5, 0.5));
        scene[obj_id].models.insert(eng::Matrix::translation_matrix(eng::Vect3(0, -1.5, 5)) * eng::Matrix::scale_matrix(10));
        scene[obj_id].meshes.insert(mesh);

        eng::DirLight light(eng::Vect3(-1, -1, 1));
        light.set_ambient(eng::Vect3(0.3, 0.3, 0.3));
        light.set_diffuse(eng::Vect3(0.6, 0.6, 0.6));
        light.set_specular(eng::Vect3(0.8, 0.8, 0.8));
        light.set_shadow_width(10);
        light.set_shadow_height(10);
        light.set_shadow_depth(10);
        light.shadow_position = eng::Vect3(3, 1, 1);
        light.shadow = true;
        scene.set_light(1, &light);
        //scene.add_object(light.get_shadow_box());

        int obj_id1 = scene.add_object(eng::GraphObject(1));
        scene[obj_id1].importFromFile("Resources/Objects/ships/mjolnir.glb");
        int model_id1 = scene[obj_id1].models.insert(eng::Matrix::scale_matrix(eng::Vect3(-1, 1, 1)) * eng::Matrix::translation_matrix(eng::Vect3(0, -0.5, 5)) * eng::Matrix::rotation_matrix(eng::Vect3(0, 1, 0), eng::PI));
        render.add_object(new Object({ obj_id1, model_id1 }, &scene));
        //scene[obj_id1].importFromFile("Resources/Objects/maps/system_velorum_position_processing_rig.glb");
        //scene[obj_id1].add_model(eng::Matrix::scale_matrix(eng::Vect3(-1, 1, 1) * 0.01));

        for (; window_interface.running;) {
            for (sf::Event event; window.pollEvent(event); ) {
                switch (event.type) {
                case sf::Event::Closed: {
                    window_interface.running = false;
                    break;
                }

                case sf::Event::KeyReleased: {
                    if (event.key.code == sf::Keyboard::Escape) {
                        scene.switch_active();
                    } else if (event.key.code == sf::Keyboard::F11) {
                        screenshot(window);
                    } else if (event.key.code == sf::Keyboard::F) {
                        if (scene.get_light(0) == nullptr)
                            scene.set_light(0, &spot_light);
                        else
                            scene.set_light(0, nullptr);
                    }
                    break;
                }

                default:
                    break;
                }
                scene.compute_event(event);
                window_interface.compute_event(event);
                if (window_interface.view_state == 1)
                    render.compute_event(event, window_interface.get_active_button());
            }

            double delta_time = window_interface.update();
            scene.update(delta_time);
            render.update(window_interface.get_active_button());

            spot_light.position = scene.camera.position;
            spot_light.set_direction(scene.camera.get_direction());

            scene.draw();

            window.pushGLStates();

            window.draw(window_interface);
            int cross_state = render.get_cross_state();
            if (cross_state == 1) {
                cross.setColor(INTERFACE_BORDER_COLOR);
                window.draw(cross);
            } else if (cross_state == 2) {
                cross.setColor(INTERFACE_ADD_COLOR);
                window.draw(cross);
            } else if (cross_state == 3) {
                window.draw(circle);
            }

            window.popGLStates();

            window.display();
        }

        return 0;
    }
    catch (const std::exception& error) {
        std::cout << error.what();
    }
    catch (...) {
        std::cout << "Unknown error.\n\n";
    }
    return -1;
}
