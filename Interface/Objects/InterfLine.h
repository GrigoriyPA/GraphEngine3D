#pragma once


class InterfLine : public InterfaceObject {
	sf::VertexArray line = sf::VertexArray(sf::Lines, 2);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(line, states);
	}

public:
	InterfLine(sf::Vector2f p1, sf::Vector2f p2) {
		line[0] = sf::Vertex(p1, INTERFACE_BORDER_COLOR);
		line[1] = sf::Vertex(p2, INTERFACE_BORDER_COLOR);
	}

	void update(double dt) {
	}

	void move(sf::Vector2f delt) {
		line[0].position += delt;
		line[1].position += delt;
	}
};
