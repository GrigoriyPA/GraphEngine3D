#pragma once

#include "Vect3.h"


namespace eng {
	class Line {
		double eps_ = 1e-5;

		Vect3 direction_ = Vect3(1, 0, 0);

	public:
		Vect3 start_point = Vect3(0, 0, 0);

		Line() {
		}

		// In case of an error sets direction to (1, 0, 0)
		Line(const Vect3& point1, const Vect3& point2) {
			if (equality((point2 - point1).length(), 0.0, eps_)) {
				std::cout << "ERROR::LINE::BULDER\n" << "Points for initialization are the same.\n\n";
				direction_ = Vect3(1, 0, 0);
			} else {
				direction_ = (point2 - point1).normalize();
			}

			start_point = point1;
		}

		// In case of an error skips operation
		void set_direction(const Vect3& direction)& {
			if (equality(direction.length(), 0.0, eps_)) {
				std::cout << "ERROR::LINE::SET_DIRECTION\n" << "The direction vector has zero length.\n\n";
				return;
			}
			direction_ = direction.normalize();
		}

		Vect3 get_direction() const {
			return direction_;
		}

		Vect3 project_point(const Vect3& point) const {
			return direction_ * (direction_ * (point - start_point)) + start_point;
		}

		bool on_line(const Vect3& point) const {
			return equality((point - project_point(point)).length(), 0.0, eps_);
		}

		bool is_intersect(const Line& line) const {
			Vect3 normal = direction_ ^ line.direction_;

			if (equality(normal.length(), 0.0, eps_)) {
				return false;
			}
			return equality(normal * (start_point - line.start_point), 0.0, eps_);
		}

		// Returns some point on one of the objects if there is no intersection
		Vect3 intersect(const Line& line) const {
			Vect3 normal = (direction_ ^ line.direction_) ^ direction_;

			if (equality(normal.length(), 0.0, eps_)) {
				return line.start_point;
			}

			normal = normal.normalize();
			double k = start_point * normal;
			double alf = (k - normal * line.start_point) / (line.direction_ * normal);

			return line.start_point + alf * line.direction_;
		}

		Vect3 symmetry(const Vect3& point) const {
			Vect3 proj = project_point(point);
			return point.symmetry(proj);
		}
	};
}
