#pragma once


class Camera {
	double eps = 0.000001;
	Matrix change_matrix = one_matrix(4);

	double screen_ratio, min_distance, max_distance, fov;
	Vect3 direction, horizont, last_position;
	Matrix projection;

	void set_projection_matrix() {
		projection = scale_matrix(Vect3(1.0 / tan(fov / 2), screen_ratio / tan(fov / 2), (max_distance + min_distance) / (max_distance - min_distance)));
		projection *= trans_matrix(Vect3(0, 0, -2.0 * max_distance * min_distance / (max_distance + min_distance)));
		projection[3][3] = 0;
		projection[3][2] = 1;
	}

public:
	double sensitivity = 0.001, speed = 3, rotate_speed = 2, speed_delt = 2;
	Vect3 position;

	Camera() {
		fov = PI / 2;
		min_distance = 0.1;
		max_distance = 1000;
		screen_ratio = 1;

		position = Vect3(0, 0, 0);
		direction = Vect3(0, 0, 1);

		last_position = position;
		horizont = -direction.horizont();

		set_projection_matrix();
	}

	Camera(double fov, double min_distance, double max_distance, double screen_ratio, Vect3 position = Vect3(0, 0, 0), Vect3 direction = Vect3(0, 0, 1)) {
		if (direction.length() < eps) {
			std::cout << "ERROR::CAMERA::BUILDER\n" << "The direction vector has zero length.\n";
			assert(0);
		}

		this->fov = fov;
		this->min_distance = min_distance;
		this->max_distance = max_distance;
		this->screen_ratio = screen_ratio;
		this->position = position;
		this->direction = direction.normalize();

		last_position = position;

		horizont = -direction.horizont();
		set_projection_matrix();
	}

	void set_direction(Vect3 direction) {
		if (direction.length() < eps) {
			std::cout << "ERROR::CAMERA::SET_DIRECTION\n" << "The direction vector has zero length.\n";
			assert(0);
		}

		this->direction = direction.normalize();

		horizont = -direction.horizont();
	}

	double get_screen_ratio() {
		return screen_ratio;
	}

	Vect3 get_direction() {
		return direction;
	}

	Vect3 get_horizont() {
		return horizont;
	}

	Vect3 get_vertical() {
		return direction ^ horizont;
	}

	Matrix get_projection_matrix() {
		return projection;
	}

	Matrix get_view_matrix() {
		return Matrix(horizont, get_vertical(), direction).transpose() * trans_matrix(-position);
	}

	double get_min_distance() {
		return min_distance;
	}

	double get_max_distance() {
		return max_distance;
	}

	Vect3 get_change_vector(Vect3 stable_point) {
		Vect3 new_point = change_matrix * (stable_point - last_position) + position;
		last_position = position;
		change_matrix = one_matrix(4);

		return new_point - stable_point;
	}

	void rotate(Vect3 axis, double angle) {
		Matrix rotate = rotate_matrix(axis, angle);

		direction = rotate * direction;
		horizont = rotate * horizont;
		change_matrix = rotate * change_matrix;
	}
};
