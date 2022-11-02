#pragma once


eng::GraphObject get_cube(int max_count_models = 1) {
    if (max_count_models < 0) {
        //std::cout << "ERROR::GET_CUBE\n" << "Negative number of models.\n";
        //assert(0);
    }

    eng::GraphObject cube(max_count_models);

    eng::Mesh mesh(4);
    mesh.set_positions({
    eng::Vect3(0.5, 0.5, 0.5),
    eng::Vect3(0.5, -0.5, 0.5),
    eng::Vect3(-0.5, -0.5, 0.5),
    eng::Vect3(-0.5, 0.5, 0.5)
    }, true);
    mesh.set_tex_coords({
    eng::Vect2(1, 1),
    eng::Vect2(1, 0),
    eng::Vect2(0, 0),
    eng::Vect2(0, 1)
    });
    cube.meshes.insert(mesh);

    mesh.apply_matrix(eng::Matrix::rotation_matrix(eng::Vect3(0, 1, 0), eng::PI / 2));
    cube.meshes.insert(mesh);

    mesh.apply_matrix(eng::Matrix::rotation_matrix(eng::Vect3(0, 1, 0), eng::PI / 2));
    cube.meshes.insert(mesh);

    mesh.apply_matrix(eng::Matrix::rotation_matrix(eng::Vect3(0, 1, 0), eng::PI / 2));
    cube.meshes.insert(mesh);

    mesh.apply_matrix(eng::Matrix::rotation_matrix(eng::Vect3(0, 0, 1), eng::PI / 2));
    cube.meshes.insert(mesh);

    mesh.apply_matrix(eng::Matrix::rotation_matrix(eng::Vect3(0, 0, 1), eng::PI));
    cube.meshes.insert(mesh);

    return cube;
}
