#pragma once

#include "Cut.h"


namespace eng {
	class Flat {
		inline static double eps_ = 1e-5;

		Vect3 normal_ = Vect3(0, 1, 0);

	public:
		double distance = 0;

		Flat() noexcept {
		}

		Flat(const Vect3& point1, const Vect3& point2, const Vect3& point3) {
			try {
				normal_ = ((point1 - point2) ^ (point1 - point3)).normalize();
			}
			catch (EngDomainError) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Flat, points to initialize are collinear.\n\n");
			}

			distance = normal_ * point1;
		}

		Flat(const std::initializer_list<Vect3>& points) {
			if (points.size() < 3) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Flat, the number of points is less than three.\n\n");
			}

			std::vector<Vect3> init;
			for (const Vect3& point : points) {
				init.push_back(point);

				if (init.size() == 3) {
					break;
				}
			}
			*this = Flat(init[0], init[1], init[2]);
		}

		explicit Flat(const std::vector<Vect3>& points) {
			if (points.size() < 3) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Flat, the number of points is less than three.\n\n");
			}

			*this = Flat(points[0], points[1], points[2]);
		}

		Flat& set_normal(const Vect3& normal) {
			try {
				normal_ = normal.normalize();
			}
			catch (EngDomainError) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_normal, the normal vector has zero length.\n\n");
			}
			return *this;
		}

		Vect3 get_normal() const noexcept {
			return normal_;
		}

		Vect3 project_point(const Vect3& point) const noexcept {
			return normal_ * (normal_ * (normal_ * distance - point)) + point;
		}

		bool on_plane(const Vect3& point) const noexcept {
			return equality(point * normal_, distance, eps_);
		}

		bool is_intersect(const Line& line) const noexcept {
			return !equality(line.get_direction() * normal_, 0.0, eps_);
		}

		bool is_intersect(const Cut& cut) const noexcept {
			if (!is_intersect(cut.get_line())) {
				return false;
			}

			int32_t diff1 = sgn(cut.get_point1() * normal_ - distance);
			int32_t diff2 = sgn(cut.get_point2() * normal_ - distance);
			return diff1 == 0 || diff2 == 0 || diff1 != diff2;
		}

		bool is_intersect(const Flat& plane) const noexcept {
			return !equality((normal_ ^ plane.normal_).length(), 0.0, eps_);
		}

		// Returns some point on other object if there is no intersection
		Vect3 intersect(const Line& line) const noexcept {
			double product = line.get_direction() * normal_;
			if (equality(product, 0.0, eps_)) {
				return line.start_point;
			}

			double alf = (distance - normal_ * line.start_point) / product;
			return line.start_point + alf * line.get_direction();
		}

		// Returns some point on other object if there is no intersection
		Vect3 intersect(const Cut& cut) const noexcept {
			return cut.project_point(intersect(cut.get_line()));
		}

		// Returns some line on other plane if there is no intersection
		Line intersect(const Flat& plane) const noexcept {
			try {
				Vect3 direction = (normal_ ^ plane.normal_).normalize();
				Vect3 start_point = normal_ * distance;
				Line ort_line(start_point, start_point + (direction ^ normal_));

				Vect3 intersection = plane.intersect(ort_line);
				return Line(intersection, intersection + direction);
			}
			catch (EngDomainError) {
				return Line(plane.distance * plane.get_normal(), plane.distance * plane.get_normal() + plane.get_normal().horizont());
			}
		}

		Vect3 symmetry(const Vect3& point) const noexcept {
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
