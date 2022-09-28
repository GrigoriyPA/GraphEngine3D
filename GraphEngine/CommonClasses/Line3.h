#pragma once

#include "Vect3.h"


class Line3 {
	double eps = 0.00001;

	eng::Vect3 direct;

public:
	eng::Vect3 p0;

	Line3() {
		p0 = eng::Vect3(0, 0, 0);
		direct = eng::Vect3(1, 0, 0);
	}

	Line3(eng::Vect3 point1, eng::Vect3 point2) {
		if ((point2 - point1).length() < eps)
			direct = eng::Vect3(1, 0, 0);
		else
			direct = (point2 - point1).normalized();

		p0 = point1;
	}

	eng::Vect3 get_direction() {
		return direct;
	}

	eng::Vect3 project_point(eng::Vect3 point) {
		return direct * (direct * (point - p0)) + p0;
	}

	bool on_line(eng::Vect3 point) {
		return ((point - p0) ^ direct).length() < eps;
	}

	bool is_intersect(Line3 line) {
		eng::Vect3 normal = direct ^ line.get_direction();

		if (normal.length() < eps)
			return false;

		return abs(normal * (p0 - line.p0)) < eps;
	}

	eng::Vect3 intersect(Line3 line) {
		eng::Vect3 normal = (direct ^ line.get_direction()) ^ direct;

		if (normal.length() < eps)
			return line.p0;

		normal = normal.normalized();
		double k = p0 * normal;

		double alf = (k - normal * line.p0) / (line.get_direction() * normal);

		return line.p0 + alf * line.get_direction();
	}

	eng::Vect3 symmetry(eng::Vect3 point) {
		eng::Vect3 proj = project_point(point);

		return point.symmetry(proj);
	}
};
