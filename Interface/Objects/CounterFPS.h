#pragma once


class CounterFPS : public InterfaceObject {
	double fps_sum = 0, time = 0;
	int cnt = 0, fps = 0;

	double update_time;
	sf::Text text;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(text, states);
	}

public:
	CounterFPS(double update_time, sf::Vector2f position, sf::Font* font, int size = 20) {
		this->update_time = update_time;

		text.setFont(*font);
		text.setFillColor(INTERFACE_TEXT_COLOR);
		text.setPosition(position);
		text.setCharacterSize(size);
	}

	void update(double dt) {
		fps_sum += 1 / dt;
		time += dt;
		cnt++;
		if (time >= update_time) {
			fps = fps_sum / ((double)cnt);
			fps_sum = 0;
			time = 0;
			cnt = 0;
		}
		text.setString("FPS: " + std::to_string(fps));
	}

	void move(sf::Vector2f delt) {
		text.move(delt);
	}
};
