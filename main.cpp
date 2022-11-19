#include "GraphEngine/Engine.h"
#include <chrono>
#include <cassert>
#include <stdio.h>
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

        eng::Vec2 cross_position(0.5, 0.5);
        sf::Sprite cross(cross_tex);
        cross.setScale(sf::Vector2f(INTERFACE_SIZE * eng::FI / cross_tex.getSize().x, INTERFACE_SIZE * eng::FI / cross_tex.getSize().y));
        cross.setPosition(sf::Vector2f(window_width * cross_position.x - INTERFACE_SIZE * eng::FI / 2, window_height * cross_position.y - INTERFACE_SIZE * eng::FI / 2));
        cross.setColor(INTERFACE_BORDER_COLOR);

        sf::Sprite circle(circle_tex);
        circle.setScale(sf::Vector2f(INTERFACE_SIZE * eng::FI / cross_tex.getSize().x, INTERFACE_SIZE * eng::FI / cross_tex.getSize().y));
        circle.setPosition(sf::Vector2f(window_width * cross_position.x - INTERFACE_SIZE * eng::FI / 2, window_height * cross_position.y - INTERFACE_SIZE * eng::FI / 2));
        circle.setColor(INTERFACE_ADD_COLOR);

        Interface window_interface(&window);

        eng::GraphEngine scene(&window);
        scene.set_clear_color(eng::Vec3(INTERFACE_MAIN_COLOR) / 255.0);
        scene.set_border_color(eng::Vec3(INTERFACE_ADD_COLOR) / 255.0);
        scene.set_check_point(cross_position);
        scene.camera.set_fov(FOV);
        scene.camera.set_distance(MIN_DIST, MAX_DIST);
        scene.camera.sensitivity = SENSITIVITY;
        scene.camera.speed = SPEED;
        scene.camera.rotation_speed = ROTATION_SPEED;
        scene.camera.speed_delt = SPEED_DELT;

        double ratio = scene.camera.get_screen_ratio();
        double angle = atan(tan(FOV / 2) * sqrt(1 + ratio * ratio));
        eng::SpotLight spot_light(eng::Vec3(0, 0, 0), eng::Vec3(0, 0, 1), angle, 1.1 * angle);
        spot_light.set_diffuse(eng::Vec3(0.6, 0.6, 0.6));
        spot_light.set_specular(eng::Vec3(0.8, 0.8, 0.8));
        spot_light.set_quadratic(0.1);
        int spot_light_id = scene.lights.insert(&spot_light);

        /*eng::DirLight light(eng::Vec3(1, -1, 1));
        light.set_ambient(eng::Vec3(0.2, 0.2, 0.2));
        light.set_diffuse(eng::Vec3(0.6, 0.6, 0.6));
        light.set_specular(eng::Vec3(0.8, 0.8, 0.8));
        scene.set_light(1, &light);*/

        eng::SpotLight light(eng::Vec3(0, 5, 5), eng::Vec3(0, -1, 0), eng::PI / 4.0, 1.1 * eng::PI / 4.0);
        light.set_ambient(eng::Vec3(0.2, 0.2, 0.2));
        light.set_diffuse(eng::Vec3(0.6, 0.6, 0.6));
        light.set_specular(eng::Vec3(0.8, 0.8, 0.8));
        light.set_linear(0.1);
        light.set_quadratic(0.1);
        light.set_shadow_distance(1, 6.5);
        light.shadow = true;
        scene.lights.insert(&light);
        //scene.add_object(light.get_shadow_box());
        scene.objects.insert(light.get_light_object());

        RenderingSequence render(&scene);

        int obj_id = scene.objects.insert(eng::GraphObject(1));
        scene.objects[obj_id].importFromFile("Resources/Objects/ships/mjolnir.glb");
        int model_id = scene.objects[obj_id].models.insert(eng::Matrix::scale_matrix(eng::Vec3(-1, 1, 1)) * eng::Matrix::translation_matrix(eng::Vec3(0, -0.5, 5)) * eng::Matrix::rotation_matrix(eng::Vec3(0, 1, 0), eng::PI));
        render.add_object(new Object({ obj_id, model_id }, &scene));
        //scene[obj_id].importFromFile("Resources/Objects/maps/system_velorum_position_processing_rig.glb");
        //scene[obj_id].models.insert(eng::Matrix::scale_matrix(eng::Vec3(-1, 1, 1) * 0.03));

        eng::Mesh mesh(4);
        mesh.set_positions({
            eng::Vec3(-1, 0, 1),
            eng::Vec3(-1, 0, -1),
            eng::Vec3(1, 0, -1),
            eng::Vec3(1, 0, 1)
        }, true);
        mesh.material.set_ambient(eng::Vec3(0.5, 0.5, 0.5));
        mesh.material.set_diffuse(eng::Vec3(0.5, 0.5, 0.5));
        eng::GraphObject obj(1);
        obj.meshes.insert(mesh);
        obj.models.insert(eng::Matrix::translation_matrix(eng::Vec3(0, -1.5, 5)) * eng::Matrix::scale_matrix(10));
        scene.objects.insert(obj);

        for (; window_interface.running;) {
            for (sf::Event event; window.pollEvent(event); ) {
                switch (event.type) {
                case sf::Event::Closed: {
                    window_interface.running = false;
                    break;
                }

                case sf::Event::KeyReleased: {
                    if (event.key.code == sf::Keyboard::Escape) {
                        scene.camera.switch_active();
                    } else if (event.key.code == sf::Keyboard::F11) {
                        screenshot(window);
                    } else if (event.key.code == sf::Keyboard::F) {
                        if (spot_light_id == -1)
                            spot_light_id = scene.lights.insert(&spot_light);
                        else {
                            scene.lights.erase(spot_light_id);
                            spot_light_id = -1;
                        }
                    }
                    break;
                }

                default:
                    break;
                }
                scene.camera.compute_event(event);
                window_interface.compute_event(event);
                if (window_interface.view_state == 1)
                    render.compute_event(event, window_interface.get_active_button());
            }

            double delta_time = window_interface.update();
            scene.camera.update(delta_time);
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
