#pragma once

#include "Line.h"


namespace eng {
	class Cut {
		double eps_ = 1e-5;

		Line line_;
		Vect3 point1_;
		Vect3 point2_;

	public:
		// In case of an error sets direction to (1, 0, 0)
		Cut(const Vect3& point1, const Vect3& point2) {
			Vect3 delt(0, 0, 0);
			if (equality((point2 - point1).length(), 0.0, eps_)) {
				std::cout << "ERROR::CUT::BULDER\n" << "Points for initialization are the same.\n\n";
				delt = Vect3(1, 0, 0);
			}

			point1_ = point1;
			point2_ = point2 + delt;
			line_ = Line(point1_, point2_ + delt);
		}

		Vect3 get_point1() const {
			return point1_;
		}

		Vect3 get_point2() const  {
			return point2_;
		}

		Line get_line() const {
			return line_;
		}

		Vect3 project_point(const Vect3& point) const {
			if ((point2_ - point1_) * (point - point1_) < 0.0) {
				return point1_;
			}
			if ((point1_ - point2_) * (point - point2_) < 0.0) {
				return point2_;
			}
			return line_.project_point(point);
		}

		bool on_cut(const Vect3& point) const {
			return line_.on_line(point) && (point1_ - point) * (point2_ - point) < 0.0;
		}

		bool is_intersect(const Line& line) const {
			Vect3 direction = line.get_direction();
			return direction.in_two_side_angle(point1_ - line.start_point, point2_ - line.start_point);
		}

		bool is_intersect(const Cut& cut) const {
			return (point1_ - point2_).in_angle(cut.point1_ - point2_, cut.point2_ - point2_) && (point2_ - point1_).in_angle(cut.point1_ - point1_, cut.point2_ - point1_);
		}

		// Returns some point on one of the objects if there is no intersection
		Vect3 intersect(const Line& line) const {
			return line_.intersect(line);
		}

		// Returns some point on one of the objects if there is no intersection
		Vect3 intersect(const Cut& cut) const {
			return line_.intersect(cut.line_);
		}
	};
}
