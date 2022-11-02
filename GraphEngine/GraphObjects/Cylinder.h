#pragma once


eng::GraphObject get_cylinder(int count_points, bool real_normals = false, int max_count_models = 1) {
	if (count_points < 3) {
		//std::cout << "ERROR::GET_CYLINDER\n" << "The number of points is less than three.\n";
		//assert(0);
	}

	if (max_count_models < 0) {
		//std::cout << "ERROR::GET_CYLINDER\n" << "Negative number of models.\n";
		//assert(0);
	}

	eng::GraphObject cylinder(max_count_models);

	std::vector < eng::Vect3 > positions(count_points);
	for (int i = 0; i < count_points; i++)
		positions[i] = eng::Vect3(cos((2 * eng::PI / count_points) * i), 0, sin((2 * eng::PI / count_points) * i));

	eng::Mesh mesh(count_points);
	mesh.set_positions(positions, true);
	mesh.invert_points_order();
	cylinder.meshes.insert(mesh);

	mesh.apply_matrix(eng::Matrix::translation_matrix(eng::Vect3(0, 1, 0)));
	mesh.invert_points_order();
	cylinder.meshes.insert(mesh);

	for (int i = 0; i < count_points; i++) {
		int j = (i + 1) % count_points;

		mesh = eng::Mesh(4);
		mesh.set_positions({
		positions[i],
		positions[j],
		positions[j] + eng::Vect3(0, 1, 0),
		positions[i] + eng::Vect3(0, 1, 0)
			}, !real_normals);

		if (real_normals) {
			mesh.set_normals({
			positions[i],
			positions[j],
			positions[j],
			positions[i]
				});
		}
		cylinder.meshes.insert(mesh);
	}

	return cylinder;
}
