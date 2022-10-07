#pragma once


GraphObject get_cylinder(int count_points, bool real_normals = false, int max_count_models = 1) {
	if (count_points < 3) {
		std::cout << "ERROR::GET_CYLINDER\n" << "The number of points is less than three.\n";
		assert(0);
	}

	if (max_count_models < 0) {
		std::cout << "ERROR::GET_CYLINDER\n" << "Negative number of models.\n";
		assert(0);
	}

	GraphObject cylinder(max_count_models);

	std::vector < eng::Vect3 > positions(count_points);
	for (int i = 0; i < count_points; i++)
		positions[i] = eng::Vect3(cos((2 * eng::PI / count_points) * i), 0, sin((2 * eng::PI / count_points) * i));

	int polygon_id = cylinder.add_polygon(count_points);
	cylinder[polygon_id].set_positions(positions);
	cylinder[polygon_id].invert_points_order();

	polygon_id = cylinder.add_polygon(cylinder[polygon_id]);
	cylinder[polygon_id].change_matrix(eng::Matrix::translation_matrix(eng::Vect3(0, 1, 0)));
	cylinder[polygon_id].invert_points_order();

	for (int i = 0; i < count_points; i++) {
		int j = (i + 1) % count_points;

		polygon_id = cylinder.add_polygon(4);
		cylinder[polygon_id].set_positions({
		positions[i],
		positions[j],
		positions[j] + eng::Vect3(0, 1, 0),
		positions[i] + eng::Vect3(0, 1, 0)
			}, !real_normals);

		if (real_normals) {
			cylinder[polygon_id].set_normals({
			positions[i],
			positions[j],
			positions[j],
			positions[i]
				});
		}
	}

	cylinder.set_center();

	return cylinder;
}
