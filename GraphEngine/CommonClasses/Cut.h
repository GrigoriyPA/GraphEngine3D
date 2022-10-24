#pragma once

#include "Line.h"


namespace eng {
	class Cut {
		inline static double eps_ = 1e-5;

		Line line_;
		Vect3 point1_;
		Vect3 point2_;

	public:
		Cut(const Vect3& point1, const Vect3& point2) {
			try {
				line_ = Line(point1_, point2_);
			}
			catch (eng_exceptions::EngInvalidArgument) {
				throw eng_exceptions::EngInvalidArgument(__FILE__, __LINE__, "Cut, points for initialization are the same.\n\n");
			}

			point1_ = point1;
			point2_ = point2;
		}

		Vect3 get_point1() const noexcept {
			return point1_;
		}

		Vect3 get_point2() const noexcept  {
			return point2_;
		}

		Line get_line() const noexcept {
			return line_;
		}

		Vect3 project_point(const Vect3& point) const noexcept {
			if ((point2_ - point1_) * (point - point1_) < 0.0) {
				return point1_;
			}
			if ((point1_ - point2_) * (point - point2_) < 0.0) {
				return point2_;
			}
			return line_.project_point(point);
		}

		bool on_cut(const Vect3& point) const noexcept {
			return line_.on_line(point) && (point1_ - point) * (point2_ - point) < 0.0;
		}

		bool is_intersect(const Line& line) const noexcept {
			return line.get_direction().in_two_side_angle(point1_ - line.start_point, point2_ - line.start_point);
		}

		bool is_intersect(const Cut& cut) const noexcept {
			return (point1_ - point2_).in_angle(cut.point1_ - point2_, cut.point2_ - point2_) && (point2_ - point1_).in_angle(cut.point1_ - point1_, cut.point2_ - point1_);
		}

		// Returns some point on other object if there is no intersection
		Vect3 intersect(const Line& line) const noexcept {
			return line_.intersect(line);
		}

		// Returns some point on other object if there is no intersection
		Vect3 intersect(const Cut& cut) const noexcept {
			return cut.project_point(line_.intersect(cut.line_));
		}

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw eng_exceptions::EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}
	};
}
