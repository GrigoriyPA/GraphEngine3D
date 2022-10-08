#pragma once

#include "Vect3.h"
#include "Line.h"
#include "Cut.h"


class Flat {
	double eps = 0.000001;

	eng::Vect3 normal;

public:
	double k;

	Flat(eng::Vect3 point1, eng::Vect3 point2, eng::Vect3 point3) {
		normal = (point1 - point2) ^ (point1 - point3);

		if (normal.length() < eps) {
			if ((point2 - point1).length() > eps) {
				point3 = (point2 - point1).horizont() + point1;
				normal = (point1 - point2) ^ (point1 - point3);
			}
			else {
				normal = eng::Vect3(0, 1, 0);
			}
		}

		normal = normal.normalize();
		k = normal * point1;
	}

	Flat(std::vector < eng::Vect3 > points) {
		if (points.size() < 3) {
			std::cout << "ERROR::FLAT::BUILDER\n" << "The number of points is less than three.\n";
			assert(0);
		}

		*this = Flat(points[0], points[1], points[2]);
	}

	eng::Vect3 get_normal() {
		return normal;
	}

	eng::Vect3 project_point(eng::Vect3 point) {
		return normal * (normal * (normal * k - point)) + point;
	}

	bool on_plane(eng::Vect3 point) {
		return point * normal == k;
	}

	bool is_intersect(eng::Line line) {
		return abs(line.get_direction() * normal) > eps;
	}

	bool is_intersect(eng::Cut cut) {
		if (!is_intersect(cut.get_line()))
			return false;

		double k1 = cut.get_point1() * normal - k, k2 = cut.get_point2() * normal - k;

		return k1 <= 0 && k2 >= 0 || k1 >= 0 && k2 <= 0;
	}

	bool is_intersect(Flat plane) {
		return (normal ^ plane.get_normal()).length() > eps;
	}

	eng::Vect3 intersect(eng::Line line) {
		eng::Vect3 direct = line.get_direction();
		double prod = direct * normal;

		if (abs(prod) < eps)
			return eng::Vect3(0, 0, 0);

		double alf = (k - normal * line.start_point) / prod;

		return line.start_point + direct * alf;
	}

	eng::Vect3 intersect(eng::Cut cut) {
		return intersect(cut.get_line());
	}

	eng::Line intersect(Flat plane) {
		eng::Vect3 direct = normal ^ plane.get_normal();

		if (direct.length() < eps)
			return eng::Line(eng::Vect3(0, 0, 0), normal);

		direct.normalize();
		eng::Vect3 p0 = normal * k;
		eng::Line ort_line(p0, p0 + (direct ^ normal));

		eng::Vect3 intersect = plane.intersect(ort_line);

		return eng::Line(intersect, intersect + direct);
	}

	eng::Vect3 symmetry(eng::Vect3 point) {
		eng::Vect3 proj = project_point(point);

		return point.symmetry(proj);
	}
};
