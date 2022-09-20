#pragma once

#include "Vect3.h"


class Line3 {
	double eps = 0.00001;

	Vect3 direct;

public:
	Vect3 p0;

	Line3() {
		p0 = Vect3(0, 0, 0);
		direct = Vect3(1, 0, 0);
	}

	Line3(Vect3 point1, Vect3 point2) {
		if ((point2 - point1).length() < eps)
			direct = Vect3(1, 0, 0);
		else
			direct = (point2 - point1).normalize();

		p0 = point1;
	}

	Vect3 get_direction() {
		return direct;
	}

	Vect3 project_point(Vect3 point) {
		return direct * (direct * (point - p0)) + p0;
	}

	bool on_line(Vect3 point) {
		return ((point - p0) ^ direct).length() < eps;
	}

	bool is_intersect(Line3 line) {
		Vect3 normal = direct ^ line.get_direction();

		if (normal.length() < eps)
			return false;

		return abs(normal * (p0 - line.p0)) < eps;
	}

	Vect3 intersect(Line3 line) {
		Vect3 normal = (direct ^ line.get_direction()) ^ direct;

		if (normal.length() < eps)
			return line.p0;

		normal = normal.normalize();
		double k = p0 * normal;

		double alf = (k - normal * line.p0) / (line.get_direction() * normal);

		return line.p0 + alf * line.get_direction();
	}

	Vect3 symmetry(Vect3 point) {
		Vect3 proj = project_point(point);

		return point.symmetry(proj);
	}
};
