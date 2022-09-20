#pragma once


class Button : public InterfaceObject {
	bool active = true, state = false, target = false, scaled = false;
	double activation_scale = 1.03, inactive_color = 0.6;

	double delta, size;
	sf::Vector2f position;
	sf::VertexArray button_backlight;
	sf::Sprite icon_first, icon_second;
	sf::ConvexShape button, inactive_button;

	void set_button(double border_width) {
		button.setPointCount(8);
		button.setPoint(0, sf::Vector2f(size * (-1.0 / 2 + delta), -size / 2));
		button.setPoint(1, sf::Vector2f(size * (1.0 / 2 - delta), -size / 2));
		button.setPoint(2, sf::Vector2f(size / 2, size * (-1.0 / 2 + delta)));
		button.setPoint(3, sf::Vector2f(size / 2, size * (1.0 / 2 - delta)));
		button.setPoint(4, sf::Vector2f(size * (1.0 / 2 - delta), size / 2));
		button.setPoint(5, sf::Vector2f(size * (-1.0 / 2 + delta), size / 2));
		button.setPoint(6, sf::Vector2f(-size / 2, size * (1.0 / 2 - delta)));
		button.setPoint(7, sf::Vector2f(-size / 2, size * (-1.0 / 2 + delta)));

		button.move(position);
		button.setOutlineThickness(border_width);

		inactive_button = button;
		inactive_button.setFillColor(sf::Color(0, 0, 0, 255 * inactive_color));
		inactive_button.setOutlineColor(sf::Color(0, 0, 0, 255 * inactive_color));

		button.setFillColor(INTERFACE_MAIN_COLOR);
		button.setOutlineColor(INTERFACE_BORDER_COLOR);
	}

	void set_button_backlight() {
		button_backlight.setPrimitiveType(sf::TriangleFan);
		button_backlight.resize(10);
		button_backlight[1].position = sf::Vector2f(position.x + size * (-1.0 / 2 + delta), position.y - size / 2);
		button_backlight[2].position = sf::Vector2f(position.x + size * (1.0 / 2 - delta), position.y - size / 2);
		button_backlight[3].position = sf::Vector2f(position.x + size / 2, position.y + size * (-1.0 / 2 + delta));
		button_backlight[4].position = sf::Vector2f(position.x + size / 2, position.y + size * (1.0 / 2 - delta));
		button_backlight[5].position = sf::Vector2f(position.x + size * (1.0 / 2 - delta), position.y + size / 2);
		button_backlight[6].position = sf::Vector2f(position.x + size * (-1.0 / 2 + delta), position.y + size / 2);
		button_backlight[7].position = sf::Vector2f(position.x - size / 2, position.y + size * (1.0 / 2 - delta));
		button_backlight[8].position = sf::Vector2f(position.x - size / 2, position.y + size * (-1.0 / 2 + delta));
		button_backlight[9].position = sf::Vector2f(position.x + size * (-1.0 / 2 + delta), position.y - size / 2);

		button_backlight[0].color = INTERFACE_ADD_COLOR;
		for (int i = 1; i < 10; i++)
			button_backlight[i].color = INTERFACE_MAIN_COLOR;
	}

	void scale_button() {
		if (scaled)
			return;

		scaled = true;
		size *= activation_scale;
		icon_first.scale(sf::Vector2f(activation_scale, activation_scale));
		icon_second.scale(sf::Vector2f(activation_scale, activation_scale));
		button.setScale(sf::Vector2f(activation_scale, activation_scale));
		set_button_backlight();
	}

	void remove_scale() {
		if (!scaled)
			return;

		scaled = false;
		size /= activation_scale;
		icon_first.scale(sf::Vector2f(1 / activation_scale, 1 / activation_scale));
		icon_second.scale(sf::Vector2f(1 / activation_scale, 1 / activation_scale));
		button.setScale(sf::Vector2f(1, 1));
		set_button_backlight();
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(button, states);

		if (this->target)
			target.draw(button_backlight, states);

		if (!state)
			target.draw(icon_first, states);
		else
			target.draw(icon_second, states);

		if (!active)
			target.draw(inactive_button, states);
	}

public:
	bool different_states = false;

	Button(sf::Vector2f position, double size, double delta, double border_width, sf::Texture* icon_first = nullptr, sf::Texture* icon_second = nullptr) {
		this->position = position;
		this->size = size;
		this->delta = delta;
		
		set_button(border_width);
		set_button_backlight();

		if (icon_first != nullptr) {
			this->icon_first.setTexture(*icon_first);
			this->icon_first.setOrigin(sf::Vector2f(icon_first->getSize().x / 2, icon_first->getSize().y / 2));
			this->icon_first.setScale(size / icon_first->getSize().x, size / icon_first->getSize().y);
			this->icon_first.setPosition(position);
			this->icon_first.setColor(INTERFACE_TEXT_COLOR);
		}

		if (icon_second != nullptr) {
			this->icon_second.setTexture(*icon_second);
			this->icon_second.setOrigin(sf::Vector2f(icon_second->getSize().x / 2, icon_second->getSize().y / 2));
			this->icon_second.setScale(size / icon_second->getSize().x, size / icon_second->getSize().y);
			this->icon_second.setPosition(position);
			this->icon_second.setColor(INTERFACE_TEXT_COLOR);
		}
	}
	
	sf::Vector2f get_position() {
		return position;
	}

	void mouse_move(sf::Vector2f mouse_position) {
		if (active && is_inside(mouse_position.x, mouse_position.y)) {
			button_backlight[0].position = mouse_position;
			button.setFillColor(sf::Color(0, 0, 0, 0));
			target = true;
		}
		else {
			button.setFillColor(INTERFACE_MAIN_COLOR);
			target = false;
		}
	}

	void mouse_button_press(bool flag) {
		if (!active)
			return;

		if (!target) {
			remove_scale();
		}
		else if (!flag) {
			switch_state();
			remove_scale();
		}
		else {
			scale_button();
		}
	}

	void switch_state() {
		state ^= 1;

		if (!different_states)
			return;

		if (state)
			button.setOutlineColor(INTERFACE_ADD_COLOR);
		else
			button.setOutlineColor(INTERFACE_BORDER_COLOR);
	}

	void switch_active() {
		active ^= 1;
		target = false;
		remove_scale();
	}

	bool is_inside(double x, double y) {
		double max_dist = size * (1 - delta);
		double cur_dist = abs(x - position.x) + abs(y - position.y);

		return position.x - size / 2 <= x && x <= position.x + size / 2 && position.y - size / 2 <= y && y <= position.y + size / 2 && cur_dist <= max_dist;
	}

	void move(sf::Vector2f delt) {
		button.move(delt);
		inactive_button.move(delt);
		icon_first.move(delt);
		icon_second.move(delt);
		position += delt;
		set_button_backlight();

		if (!is_inside(button_backlight[0].position.x, button_backlight[0].position.y))
			target = false;
	}

	void update(double dt) {
	}
};
