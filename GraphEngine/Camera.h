#pragma once


class Camera {
	double eps = 0.000001;
	eng::Matrix change_matrix = eng::one_matrix(4);

	double screen_ratio, min_distance, max_distance, fov;
	eng::Vect3 direction, horizont, last_position;
	eng::Matrix projection;

	void set_projection_matrix() {
		projection = scale_matrix(eng::Vect3(1.0 / tan(fov / 2), screen_ratio / tan(fov / 2), (max_distance + min_distance) / (max_distance - min_distance)));
		projection *= trans_matrix(eng::Vect3(0, 0, -2.0 * max_distance * min_distance / (max_distance + min_distance)));
		projection[3][3] = 0;
		projection[3][2] = 1;
	}

public:
	double sensitivity = 0.001, speed = 3, rotate_speed = 2, speed_delt = 2;
	eng::Vect3 position;

	Camera() {
		fov = eng::PI / 2;
		min_distance = 0.1;
		max_distance = 1000;
		screen_ratio = 1;

		position = eng::Vect3(0, 0, 0);
		direction = eng::Vect3(0, 0, 1);

		last_position = position;
		horizont = direction.horizont();

		set_projection_matrix();
	}

	Camera(double fov, double min_distance, double max_distance, double screen_ratio, eng::Vect3 position = eng::Vect3(0, 0, 0), eng::Vect3 direction = eng::Vect3(0, 0, 1)) {
		if (direction.length() < eps) {
			std::cout << "ERROR::CAMERA::BUILDER\n" << "The direction vector has zero length.\n";
			assert(0);
		}

		this->fov = fov;
		this->min_distance = min_distance;
		this->max_distance = max_distance;
		this->screen_ratio = screen_ratio;
		this->position = position;
		this->direction = direction.normalized();

		last_position = position;

		horizont = direction.horizont();
		set_projection_matrix();
	}

	void set_direction(eng::Vect3 direction) {
		if (direction.length() < eps) {
			std::cout << "ERROR::CAMERA::SET_DIRECTION\n" << "The direction vector has zero length.\n";
			assert(0);
		}

		this->direction = direction.normalized();

		horizont = direction.horizont();
	}

	double get_screen_ratio() {
		return screen_ratio;
	}

	eng::Vect3 get_direction() {
		return direction;
	}

	eng::Vect3 get_horizont() {
		return horizont;
	}

	eng::Vect3 get_vertical() {
		return direction ^ horizont;
	}

	eng::Matrix get_projection_matrix() {
		return projection;
	}

	eng::Matrix get_view_matrix() {
		return eng::Matrix(horizont, get_vertical(), direction).transpose() * trans_matrix(-position);
	}

	double get_min_distance() {
		return min_distance;
	}

	double get_max_distance() {
		return max_distance;
	}

	eng::Vect3 get_change_vector(eng::Vect3 stable_point) {
		eng::Vect3 new_point = change_matrix * (stable_point - last_position) + position;
		last_position = position;
		change_matrix = eng::one_matrix(4);

		return new_point - stable_point;
	}

	void rotate(eng::Vect3 axis, double angle) {
		eng::Matrix rotate = rotate_matrix(axis, angle);

		direction = rotate * direction;
		horizont = rotate * horizont;
		change_matrix = rotate * change_matrix;
	}
};
