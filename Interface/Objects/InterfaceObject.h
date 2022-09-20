#pragma once

class InterfaceObject : public sf::Drawable {
public:
	virtual void update(double dt) = 0;

	virtual void move(sf::Vector2f delt) = 0;
};


#include "InterfLine.h"
#include "CounterFPS.h"
#include "Button.h"
