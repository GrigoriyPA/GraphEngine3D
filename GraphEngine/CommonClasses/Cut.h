#pragma once

#include "Line.h"


namespace eng {
	class Cut {
		Line line_;
		Vect3 point1_;
		Vect3 point2_;

	public:
		Cut(const Vect3& point1, const Vect3& point2) {
			point1_ = point1;
			point2_ = point2;
			line_ = Line(point1_, point2_);
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
			return line_.intersect(cut.get_line());
		}
	};
}
