#pragma once

#include "Cut.h"


namespace eng {
	class Flat {
		double eps_ = 1e-5;

		Vect3 normal_ = Vect3(0, 1, 0);

	public:
		double distance = 0;

		Flat() {
		}

		// In case of an error sets normal to (0, 1, 0)
		Flat(const Vect3& point1, const Vect3& point2, const Vect3& point3) {
			normal_ = (point1 - point2) ^ (point1 - point3);
			if (equality(normal_.length(), 0.0, eps_)) {
				if (!equality((point2 - point1).length(), 0.0, eps_)) {
					std::cout << "ERROR::FLAT::BULDER\n" << "Points to initialize are collinear.\n\n";
					normal_ = (point1 - point2) ^ (point2 - point1).horizont();
				} else {
					std::cout << "ERROR::FLAT::BULDER\n" << "Points for initialization are the same.\n\n";
					normal_ = Vect3(0, 1, 0);
				}
			}
			normal_ = normal_.normalize();
			distance = normal_ * point1;
		}

		// In case of an error sets default values
		Flat(const std::initializer_list<Vect3>& points) {
			if (points.size() < 3) {
				std::cout << "ERROR::FLAT::BUILDER\n" << "The number of points is less than three.\n\n";
				return;
			}

			std::vector<Vect3> init;
			init.reserve(points.size());
			for (const Vect3& point : points) {
				init.push_back(point);

				if (init.size() == 3) {
					break;
				}
			}
			*this = Flat(init[0], init[1], init[2]);
		}

		// In case of an error sets default values
		explicit Flat(const std::vector<Vect3>& points) {
			if (points.size() < 3) {
				std::cout << "ERROR::FLAT::BUILDER\n" << "The number of points is less than three.\n\n";
				return;
			}

			*this = Flat(points[0], points[1], points[2]);
		}

		// In case of an error skips operation
		void set_normal(const Vect3& normal)& {
			if (equality(normal.length(), 0.0, eps_)) {
				std::cout << "ERROR::FLAT::SET_NORMAL\n" << "The normal vector has zero length.\n\n";
				return;
			}
			normal_ = normal.normalize();
		}

		Vect3 get_normal() const {
			return normal_;
		}

		Vect3 project_point(const Vect3& point) const {
			return normal_ * (normal_ * (normal_ * distance - point)) + point;
		}

		bool on_plane(const Vect3& point) const {
			return equality(point * normal_, distance, eps_);
		}

		bool is_intersect(const Line& line) const {
			return !equality(line.get_direction() * normal_, 0.0, eps_);
		}

		bool is_intersect(const Cut& cut) const {
			if (!is_intersect(cut.get_line())) {
				return false;
			}

			int32_t diff1 = sign(cut.get_point1() * normal_ - distance);
			int32_t diff2 = sign(cut.get_point2() * normal_ - distance);
			return diff1 == 0 || diff2 == 0 || diff1 != diff2;
		}

		bool is_intersect(const Flat& plane) const {
			return !equality((normal_ ^ plane.normal_).length(), 0.0, eps_);
		}

		// Returns some point on one of the objects if there is no intersection
		Vect3 intersect(const Line& line) const {
			double product = line.get_direction() * normal_;
			if (equality(product, 0.0, eps_)) {
				return line.start_point;
			}

			double alf = (distance - normal_ * line.start_point) / product;
			return line.start_point + alf * line.get_direction();
		}

		// Returns some point on one of the objects if there is no intersection
		Vect3 intersect(const Cut& cut) const {
			return intersect(cut.get_line());
		}

		// Returns some line on one of the planes if there is no intersection
		Line intersect(const Flat& plane) const {
			Vect3 direction = normal_ ^ plane.normal_;
			if (equality(direction.length(), 0.0, eps_)) {
				return Line(distance * normal_, distance * normal_ + normal_.horizont());
			}
			direction = direction.normalize();

			Vect3 start_point = normal_ * distance;
			Line ort_line(start_point, start_point + (direction ^ normal_));
			Vect3 intersection = plane.intersect(ort_line);
			return Line(intersection, intersection + direction);
		}

		Vect3 symmetry(const Vect3& point) const {
			Vect3 proj = project_point(point);
			return point.symmetry(proj);
		}
	};
}
