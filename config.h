#pragma once


const double SCROLL_SENSITIVITY = 1.2;
const double SENSITIVITY = 0.001;
const double SPEED = 3;
const double ROTATION_SPEED = 2;
const double SPEED_DELT = 2;

const double GAMMA = 2.2;

const double FOV = eng::PI / 2;
const double MIN_DIST = 0.1;
const double MAX_DIST = 250;

const double POINT_RADIUS = 0.05;
const double BORDER_WIDTH = 0.1;
const int MAX_COUNT_MODELS = 1;

const sf::Color INTERFACE_MAIN_COLOR = sf::Color(0, 0, 0, 150);
const sf::Color INTERFACE_BORDER_COLOR = sf::Color(0, 0, 215);
const sf::Color INTERFACE_ADD_COLOR = sf::Color(215, 0, 0);
const sf::Color INTERFACE_TEXT_COLOR = sf::Color(255, 255, 255);

const double INTERFACE_SIZE = 70;
const double INTERFACE_WIDTH = 2;
const double INTERFACE_SPEED = 5;
const double INTERFACE_DELTA = (1.0 - eng::FI) / 2;
