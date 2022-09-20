#pragma once

#include "Vect3.h"
#include "Line3.h"


class Cut3 {
	double eps = 0.000001;

	Line3 line;
	Vect3 p1, p2;

public:

	Cut3(Vect3 point1, Vect3 point2) {
		p1 = point1;
		p2 = point2;
		line = Line3(p1, p2);
	}

	Vect3 get_point1() {
		return p1;
	}

	Vect3 get_point2() {
		return p2;
	}

	Line3 get_line() {
		return line;
	}

	Vect3 project_point(Vect3 point) {
		if ((p2 - p1) * (point - p1) < 0)
			return p1;

		if ((p1 - p2) * (point - p2) < 0)
			return p2;

		return line.project_point(point);
	}

	bool is_intersect(Line3 line) {
		Vect3 direction = line.get_direction();

		return direction.in_angle(p1 - line.p0, p2 - line.p0) || (-direction).in_angle(p1 - line.p0, p2 - line.p0);
	}

	bool is_intersect(Cut3 cut) {
		return (p1 - p2).in_angle(cut.p1 - p2, cut.p2 - p2) && (p2 - p1).in_angle(cut.p1 - p1, cut.p2 - p1);
	}

	Vect3 intersect(Line3 line) {
		return (this->line).intersect(line);
	}

	Vect3 intersect(Cut3 cut) {
		return line.intersect(cut.get_line());
	}
};
