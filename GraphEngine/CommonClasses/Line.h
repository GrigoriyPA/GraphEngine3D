#pragma once

#include "Vec3.h"


namespace eng {
	class Line {
		inline static double eps_ = 1e-5;

		Vec3 direction_ = Vec3(1, 0, 0);

	public:
		Vec3 start_point = Vec3(0, 0, 0);

		Line() noexcept {
		}

		Line(const Vec3& point1, const Vec3& point2) {
			try {
				direction_ = (point2 - point1).normalize();
			}
			catch (EngDomainError) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Line, points for initialization are the same.\n\n");
			}

			start_point = point1;
		}

		Line& set_direction(const Vec3& direction) {
			try {
				direction_ = direction.normalize();
			}
			catch (EngDomainError) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_direction, the direction vector has zero length.\n\n");
			}
			return *this;
		}

		Vec3 get_direction() const noexcept {
			return direction_;
		}

		Vec3 project_point(const Vec3& point) const noexcept {
			return direction_ * (direction_ * (point - start_point)) + start_point;
		}

		bool on_line(const Vec3& point) const noexcept {
			return equality((point - project_point(point)).length(), 0.0, eps_);
		}

		bool is_intersect(const Line& line) const noexcept {
			try {
				return equality((direction_ ^ line.direction_).normalize() * (start_point - line.start_point), 0.0, eps_);
			}
			catch (EngDomainError) {
				return false;
			}
		}

		// Returns some point on other object if there is no intersection
		Vec3 intersect(const Line& line) const noexcept {
			try {
				Vec3 normal = ((direction_ ^ line.direction_) ^ direction_).normalize();

				double k = start_point * normal;
				double alf = (k - normal * line.start_point) / (line.direction_ * normal);
				return line.start_point + alf * line.direction_;
			}
			catch (EngDomainError) {
				return line.start_point;
			}
		}

		Vec3 symmetry(const Vec3& point) const noexcept {
			return point.symmetry(project_point(point));
		}

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}
	};
}
