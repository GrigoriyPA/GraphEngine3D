#pragma once

#include "Vect3.h"
#include "Line3.h"


class Cut3 {
	double eps = 0.000001;

	eng::Line line;
	eng::Vect3 p1, p2;

public:

	Cut3(eng::Vect3 point1, eng::Vect3 point2) {
		p1 = point1;
		p2 = point2;
		line = eng::Line(p1, p2);
	}

	eng::Vect3 get_point1() {
		return p1;
	}

	eng::Vect3 get_point2() {
		return p2;
	}

	eng::Line get_line() {
		return line;
	}

	eng::Vect3 project_point(eng::Vect3 point) {
		if ((p2 - p1) * (point - p1) < 0)
			return p1;

		if ((p1 - p2) * (point - p2) < 0)
			return p2;

		return line.project_point(point);
	}

	bool is_intersect(eng::Line line) {
		eng::Vect3 direction = line.get_direction();

		return direction.in_angle(p1 - line.start_point, p2 - line.start_point) || (-direction).in_angle(p1 - line.start_point, p2 - line.start_point);
	}

	bool is_intersect(Cut3 cut) {
		return (p1 - p2).in_angle(cut.p1 - p2, cut.p2 - p2) && (p2 - p1).in_angle(cut.p1 - p1, cut.p2 - p1);
	}

	eng::Vect3 intersect(eng::Line line) {
		return (this->line).intersect(line);
	}

	eng::Vect3 intersect(Cut3 cut) {
		return line.intersect(cut.get_line());
	}
};
