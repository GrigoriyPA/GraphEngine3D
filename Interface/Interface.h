#pragma once

#include <vector>
#include "Objects/InterfaceObject.h"
#include "MovingPanel.h"


class Interface : public sf::Drawable {
    int active_button = -1, size = 5;
    std::vector < int > count_buttons = { 5, 4, 6, 4, 3, 2 };

    int window_width, window_height;
    sf::Clock timer;
    sf::Font main_font;
    std::vector < sf::Texture > textures;
    std::vector < InterfaceObject* > objects;
    MovingPanel left_bar;

    void load_files() {
        main_font.loadFromFile("Interface/Resources/Fonts/arial.ttf");

        int id = 0, sm = 0;
        for (int i = 0; id < count_buttons.size(); i++) {
            if (i >= sm + count_buttons[id]) {
                sm += count_buttons[id];
                id++;
            }

            if (id == count_buttons.size())
                break;

            textures.push_back(sf::Texture());
            textures[i].loadFromFile("Interface/Resources/Textures/" + std::to_string(id + 1) + "." + std::to_string(i - sm) + ".png");
            textures[i].setSmooth(true);
            textures[i].generateMipmap();
        }
    }

    void init_interface() {
        objects.push_back(new CounterFPS(1, sf::Vector2f(window_width - 80, window_height - 30), &main_font));
        left_bar = MovingPanel(INTERFACE_SIZE * size, window_height, INTERFACE_SPEED, (1.0 - FI) / size, INTERFACE_WIDTH * 2);

        int tex_id = 0;
        double button_size = INTERFACE_SIZE * (1 - 2 * INTERFACE_DELTA);
        double x_s = INTERFACE_SIZE / 2.0 - button_size / 2;
        double x_e = INTERFACE_SIZE * (1.0 / 2.0 + size - 1) + button_size / 2;
        double y = INTERFACE_SIZE * (1.5 - FI - INTERFACE_DELTA);
        for (int id = 0; id < count_buttons.size() - 1; id++) {
            for (int i = 0; i < count_buttons[id]; i++) {
                if (i > 0 && i % size == 0)
                    y += INTERFACE_SIZE;

                Button* button = new Button(sf::Vector2f(INTERFACE_SIZE * (1.0 / 2.0 + float(i % size)), y), button_size, (1.0 - FI) / 2, INTERFACE_WIDTH, &textures[tex_id], &textures[tex_id]);
                button->different_states = true;
                left_bar.buttons.push_back(button);
                tex_id++;
            }

            if (id < count_buttons.size() - 2) {
                InterfLine* line = new InterfLine(sf::Vector2f(x_s, y + INTERFACE_SIZE / 2), sf::Vector2f(x_e, y + INTERFACE_SIZE / 2));
                left_bar.objects.push_back(line);
            }

            y += INTERFACE_SIZE;
        }

        for (int i = 0; i < count_buttons.back(); i++) {
            Button* button = new Button(sf::Vector2f(INTERFACE_SIZE * (1.0 / 2.0 + float(i)), window_height - INTERFACE_SIZE * (1.5 - FI - INTERFACE_DELTA)), button_size, (1.0 - FI) / 2, INTERFACE_WIDTH, &textures[tex_id], &textures[tex_id]);
            left_bar.buttons.push_back(button);
            tex_id++;
        }

        y = left_bar.buttons.back()->get_position().y - INTERFACE_SIZE / 2;
        InterfLine* line = new InterfLine(sf::Vector2f(x_s, y), sf::Vector2f(x_e, y));
        left_bar.objects.push_back(line);
    }

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        for (InterfaceObject* object : objects)
            target.draw(*object, states);
        target.draw(left_bar, states);
	}

public:
    bool running = true;
    int view_state = 0;

    Interface(sf::RenderWindow* window) {
        window_width = window->getSize().x;
        window_height = window->getSize().y;

        load_files();
        init_interface();
	}

    std::pair < int, int> get_active_button() {
        if (active_button == -1)
            return std::make_pair(0, 0);

        int group_id = 1, button_id = active_button;
        for (int el : count_buttons) {
            if (el > button_id)
                return std::make_pair(group_id, button_id);

            group_id++;
            button_id -= el;
        }
    }

    void compute_event(sf::Event event) {
        switch (event.type) {
            case sf::Event::KeyReleased: {
                if (event.key.code == sf::Keyboard::Escape) {
                    view_state ^= 1;
                    left_bar.opening ^= 1;
                }
                break;
            }

            case sf::Event::MouseButtonPressed: {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (view_state == 0)
                        left_bar.mouse_button_press(true);
                }
                break;
            }

            case sf::Event::MouseButtonReleased: {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (view_state == 0) {
                        left_bar.mouse_button_press(false);

                        int target_button = left_bar.get_target_button_id(event.mouseButton.x, event.mouseButton.y);
                        if (target_button == -1)
                            break;

                        if (target_button == left_bar.buttons.size() - 1) {
                            running = false;
                        }
                        else if (target_button == active_button) {
                            active_button = -1;
                        }
                        else {
                            if (active_button != -1)
                                left_bar.buttons[active_button]->switch_state();
                            active_button = target_button;
                        }
                    }
                }
                break;
            }

            case sf::Event::MouseMoved: {
                if (view_state == 0)
                    left_bar.mouse_move(sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
                break;
            }

            default:
                break;
        }
    }

    double update() {
        double dt = timer.restart().asSeconds();
        for (InterfaceObject* object : objects)
            object->update(dt);
        left_bar.update(dt);
        left_bar.mouse_move(sf::Vector2f(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y));

        return dt;
    }
};
