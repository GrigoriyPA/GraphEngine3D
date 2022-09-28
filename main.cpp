#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string>
#include "GraphEngine/GraphEngine.h"
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
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "Editor", sf::Style::None, settings);
    if (window.getSettings().majorVersion < settings.majorVersion || window.getSettings().minorVersion < settings.minorVersion) {
        std::cout << "ERROR\n" << "Invalid OpenGL version.";
        assert(false);
    }
    window.setVerticalSyncEnabled(true);

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

    GraphEngine scene(&window, FOV, MIN_DIST, MAX_DIST);
    scene.set_clear_color(eng::Vect3(INTERFACE_MAIN_COLOR) / 255.0);
    scene.set_border_color(eng::Vect3(INTERFACE_ADD_COLOR) / 255.0);

    double ratio = scene.cam.get_screen_ratio();
    double angle = atan(tan(FOV / 2) * sqrt(1 + ratio * ratio));
    SpotLight spot_light(eng::Vect3(0, 0, 0), eng::Vect3(0, 0, 1), angle, 1.1 * angle);
    spot_light.diffuse = eng::Vect3(0.6, 0.6, 0.6);
    spot_light.specular = eng::Vect3(0.8, 0.8, 0.8);
    spot_light.quadratic = 0.1;
    scene.set_light(0, &spot_light);

    RenderingSequence render(&scene);

    int obj_id = scene.add_object(GraphObject(1));
    scene[obj_id].import_from_file("Resources/Objects/test_car/thunder_master_hypercar_mark_i_by_alex.ka..glb");
    scene[obj_id].add_model(scale_matrix(eng::Vect3(-1, 1, 1)) * trans_matrix(eng::Vect3(0, -0.5, 5)) * rotate_matrix(eng::Vect3(0, 1, 0), eng::PI));
    
    /*obj_id = scene.add_object(get_cube(3));
    Material material;
    material.shininess = 64;
    material.diffuse_map = Texture("Resources/Textures/diffuse/box.png");
    material.specular_map = Texture("Resources/Textures/specular/box.png", false);
    scene[obj_id].set_material(material);

    int model_id = scene[obj_id].add_model(trans_matrix(Vect3(1, 0.5, 0)));
    int loc_id = -1;
    RenderObject* obj = new Point(Vect3(0, 0, 0), loc_id, &scene);
    scene.delete_object(loc_id, 0);
    obj->scene_id = { obj_id, model_id };
    render.add_object(obj);

    model_id = scene[obj_id].add_model(trans_matrix(Vect3(-0.5, 0.5, 1)) * scale_matrix(1.2));
    loc_id = -1;
    obj = new Point(Vect3(0, 0, 0), loc_id, &scene);
    scene.delete_object(loc_id, 0);
    obj->scene_id = { obj_id, model_id };
    render.add_object(obj);

    model_id = scene[obj_id].add_model(trans_matrix(Vect3(0.5, 1.5, -1)) * scale_matrix(0.8) * rotate_matrix(Vect3(1, 2, -3), PI / 3));
    loc_id = -1;
    obj = new Point(Vect3(0, 0, 0), loc_id, &scene);
    scene.delete_object(loc_id, 0);
    obj->scene_id = { obj_id, model_id };
    render.add_object(obj);

    obj_id = scene.add_object(GraphObject(1));
    int pol_id = scene[obj_id].add_polygon(Polygon(4));
    scene[obj_id][pol_id].set_positions({
        Vect3(1, 0, 1),
        Vect3(1, 0, -1),
        Vect3(-1, 0, -1),
        Vect3(-1, 0, 1)
      });
    scene[obj_id][pol_id].set_tex_coords({
        Vect2(1, 1),
        Vect2(1, 0),
        Vect2(0, 0),
        Vect2(0, 1)
    });
    scene[obj_id][pol_id].material.diffuse_map = Texture("Resources/Textures/diffuse/parquet.jpg");

    model_id = scene[obj_id].add_model(trans_matrix(Vect3(0, 4, 0)) * scale_matrix(10));
    loc_id = -1;
    obj = new Point(Vect3(0, 0, 0), loc_id, &scene);
    scene.delete_object(loc_id, 0);
    obj->scene_id = { obj_id, model_id };
    render.add_object(obj);*

    SpotLight light(Vect3(2, 2, 2), Vect3(-1, -1, 1), PI / 4.0, 1.2 * PI / 4.0);
    light.ambient = Vect3(0.1, 0.1, 0.1);
    light.diffuse = Vect3(0.6, 0.6, 0.6);
    light.specular = Vect3(0.8, 0.8, 0.8);
    light.quadratic = 0.1;
    light.set_shadow_distance(1, 40);
    light.shadow = true;
    scene.set_light(1, &light);
    scene.add_object(light.get_light_object());*/
    //scene.add_object(light.get_shadow_box());

    obj_id = scene.add_object(GraphObject(1));
    int pol_id = scene[obj_id].add_polygon(Polygon(4));
    scene[obj_id][pol_id].set_positions({
        eng::Vect3(1, 0, 1),
        eng::Vect3(1, 0, -1),
        eng::Vect3(-1, 0, -1),
        eng::Vect3(-1, 0, 1)
    });
    scene[obj_id][pol_id].set_tex_coords({
        eng::Vect2(1, 1),
        eng::Vect2(1, 0),
        eng::Vect2(0, 0),
        eng::Vect2(0, 1)
    });
    scene[obj_id][pol_id].invert_points_order();
    scene[obj_id][pol_id].material.diffuse = eng::Vect3(1, 1, 1);
    scene[obj_id][pol_id].material.alpha = 0.5;
    scene[obj_id].transparent = true;
    scene[obj_id].add_model(trans_matrix(eng::Vect3(0, -0.5, 5)) * eng::scale_matrix(10));

    DirLight light(eng::Vect3(-1, -1, 1));
    light.ambient = eng::Vect3(0.1, 0.1, 0.1);
    light.diffuse = eng::Vect3(0.6, 0.6, 0.6);
    light.specular = eng::Vect3(0.8, 0.8, 0.8);
    light.shadow_position = eng::Vect3(2, 0, 2);
    light.shadow = true;
    scene.set_light(1, &light);
    

    for (; window_interface.running; ) {
        for (sf::Event event; window.pollEvent(event); ) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window_interface.running = false;
                    break;
                }

                case sf::Event::KeyReleased: {
                    if (event.key.code == sf::Keyboard::Escape) {
                        scene.switch_active();
                    }
                    else if (event.key.code == sf::Keyboard::F11) {
                        screenshot(window);
                    }
                    else if (event.key.code == sf::Keyboard::F) {
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

        spot_light.position = scene.cam.position;
        spot_light.set_direction(scene.cam.get_direction());

        scene.draw();

        window.pushGLStates();

        window.draw(window_interface);
        int cross_state = render.get_cross_state();
        if (cross_state == 1) {
            cross.setColor(INTERFACE_BORDER_COLOR);
            window.draw(cross);
        }
        else if (cross_state == 2) {
            cross.setColor(INTERFACE_ADD_COLOR);
            window.draw(cross);
        }
        else if (cross_state == 3) {
            window.draw(circle);
        }

        window.popGLStates();

        window.display();
    }

    return 0;
}
