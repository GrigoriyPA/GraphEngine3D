#pragma once


GraphObject get_cube(int max_count_models = 1) {
	if (max_count_models < 0) {
		std::cout << "ERROR::GET_CUBE\n" << "Negative number of models.\n";
		assert(0);
	}

	GraphObject cube(max_count_models);

    int polygon_id = cube.add_polygon(Polygon(4));
    cube[polygon_id].set_positions({
    Vect3(0.5, 0.5, 0.5),
    Vect3(0.5, -0.5, 0.5),
    Vect3(-0.5, -0.5, 0.5),
    Vect3(-0.5, 0.5, 0.5)
    });
    cube[polygon_id].set_tex_coords({
    Vect2(1, 1),
    Vect2(1, 0),
    Vect2(0, 0),
    Vect2(0, 1)
    });

    polygon_id = cube.add_polygon(cube[polygon_id]);
    cube[polygon_id].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));

    polygon_id = cube.add_polygon(cube[polygon_id]);
    cube[polygon_id].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));

    polygon_id = cube.add_polygon(cube[polygon_id]);
    cube[polygon_id].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));

    polygon_id = cube.add_polygon(cube[polygon_id]);
    cube[polygon_id].change_matrix(rotate_matrix(Vect3(0, 0, 1), PI / 2));

    polygon_id = cube.add_polygon(cube[polygon_id]);
    cube[polygon_id].change_matrix(rotate_matrix(Vect3(0, 0, 1), PI));

    cube.set_center();

	return cube;
}
