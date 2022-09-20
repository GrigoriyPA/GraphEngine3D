#pragma once

#include <vector>


class MovingPanel : public sf::Drawable {
	double cur_move = 0, eps = 0.00001;

	double width, height, delta, border_width, speed;
	sf::ConvexShape plane;

	void set_plane(double border_width) {
		plane.setPointCount(6);
		plane.setPoint(0, sf::Vector2f(0, 0));
		plane.setPoint(1, sf::Vector2f(width * (1 - delta), 0));
		plane.setPoint(2, sf::Vector2f(width, delta * width));
		plane.setPoint(3, sf::Vector2f(width, height - delta * width));
		plane.setPoint(4, sf::Vector2f(width * (1 - delta), height));
		plane.setPoint(5, sf::Vector2f(0, height));

		plane.setOutlineThickness(border_width);

		plane.setFillColor(INTERFACE_MAIN_COLOR);
		plane.setOutlineColor(INTERFACE_BORDER_COLOR);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(plane, states);
		for (Button* button : buttons)
			target.draw(*button, states);
		for (InterfaceObject* object : objects)
			target.draw(*object, states);
	}

public:
	bool opening = true;

	std::vector < InterfaceObject* > objects;
	std::vector < Button* > buttons;

	MovingPanel() {

	}

	MovingPanel(double width, double height, double speed, double delta, double border_width) {
		this->width = width;
		this->height = height;
		this->border_width = border_width;
		this->delta = delta;
		this->speed = speed;

		set_plane(border_width);
	}

	int get_target_button_id(double x, double y) {
		for (int i = 0; i < buttons.size(); i++) {
			if (buttons[i]->is_inside(x, y))
				return i;
		}
		return -1;
	}

	void update(double dt) {
		for (InterfaceObject* object : objects)
			object->update(dt);

		double dist = dt * speed * width * (2 * opening - 1);
		cur_move += dist;
		if (cur_move > 0) {
			dist -= cur_move;
			cur_move = 0;
		}
		else if (cur_move < -width - border_width) {
			dist += -width - border_width - cur_move;
			cur_move = -width - border_width;
		}

		if (abs(dist) <= eps)
			return;

		plane.move(sf::Vector2f(dist, 0));
		for (Button* button : buttons)
			button->move(sf::Vector2f(dist, 0));
		for (InterfaceObject* object : objects)
			object->move(sf::Vector2f(dist, 0));
	}

	void mouse_move(sf::Vector2f mouse_position) {
		for (Button* button : buttons)
			button->mouse_move(mouse_position);
	}

	void mouse_button_press(bool flag) {
		for (Button* button : buttons)
			button->mouse_button_press(flag);
	}
};
