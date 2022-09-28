#pragma once


GraphObject get_sphere(int count_points, bool real_normals = false, int max_count_models = 1) {
	if (count_points < 3) {
		std::cout << "ERROR::GET_SPHERE\n" << "The number of points is less than three.\n";
		assert(0);
	}

	if (max_count_models < 0) {
		std::cout << "ERROR::GET_SPHERE\n" << "Negative number of models.\n";
		assert(0);
	}

	GraphObject sphere(max_count_models);
	std::vector < eng::Vect3 > last_position(2 * count_points, eng::Vect3(0, 1, 0));
	for (int i = 0; i < count_points; i++) {
		std::vector < eng::Vect3 > cur_positions(2 * count_points);

		double b = (eng::PI / count_points) * (i + 1);
		for (int j = 0; j < 2 * count_points; j++) {
			double a = (eng::PI / count_points) * j;
			cur_positions[j] = eng::Vect3(cos(a) * sin(b), cos(b), sin(a) * sin(b));
		}

		for (int j = 0; j < 2 * count_points; j++) {
			int next = (j + 1) % (2 * count_points), polygon_id;

			std::vector < eng::Vect3 > positions;
			if (i == 0) {
				polygon_id = sphere.add_polygon(Polygon(3));
				positions = { eng::Vect3(0, 1, 0), cur_positions[j], cur_positions[next] };
			}
			else if (i == count_points - 1) {
				polygon_id = sphere.add_polygon(Polygon(3));
				positions = { last_position[j], eng::Vect3(0, -1, 0), last_position[next] };
			}
			else {
				polygon_id = sphere.add_polygon(Polygon(4));
				positions = { last_position[next], last_position[j], cur_positions[j], cur_positions[next] };
			}

			sphere[polygon_id].set_positions(positions, !real_normals);
			if (real_normals)
				sphere[polygon_id].set_normals(positions);
		}
		last_position = cur_positions;
	}

	sphere.set_center();

	return sphere;
}
