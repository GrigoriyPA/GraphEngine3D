#pragma once


class DirLight : public Light {
    double shadow_width = 10, shadow_height = 10, shadow_depth = 10;

    eng::Vect3 direction;
    eng::Matrix projection;

    void set_projection_matrix() {
        projection = eng::Matrix::scale_matrix(eng::Vect3(2.0 / shadow_width, 2.0 / shadow_height, 2.0 / shadow_depth)) * eng::Matrix::translation_matrix(eng::Vect3(0, 0, -shadow_depth / 2));
    }

    eng::Matrix get_view_matrix() {
        eng::Vect3 horizont = direction.horizont();

        return eng::Matrix(horizont, direction ^ horizont, direction).transpose() * eng::Matrix::translation_matrix(-shadow_position);
    }

public:
    eng::Vect3 shadow_position = eng::Vect3(0, 0, 0);

    DirLight(eng::Vect3 direction) : projection(eng::Matrix::one_matrix(4)) {
        if (direction.length() < eps) {
            std::cout << "ERROR::DIR_LIGHT::BUILDER\n" << "The direction vector has zero length.\n";
            assert(0);
        }

        this->direction = direction.normalize();

        set_projection_matrix();
    }

    void set_uniforms(int draw_id, eng::Shader<size_t>* shader_program) {
        if (draw_id < 0) {
            std::cout << "ERROR::DIR_LIGHT::SET_UNIFORMS\n" << "Invalid draw id.\n";
            assert(0);
        }

        try {
            std::string name = "lights[" + std::to_string(draw_id) + "].";
            shader_program->set_uniform_i((name + "shadow").c_str(), shadow);
            shader_program->set_uniform_i((name + "type").c_str(), 0);
            shader_program->set_uniform_f((name + "direction").c_str(), direction.x, direction.y, direction.z);
            shader_program->set_uniform_f((name + "ambient").c_str(), ambient.x, ambient.y, ambient.z);
            shader_program->set_uniform_f((name + "diffuse").c_str(), diffuse.x, diffuse.y, diffuse.z);
            shader_program->set_uniform_f((name + "specular").c_str(), specular.x, specular.y, specular.z);
            if (shadow)
                shader_program->set_uniform_matrix((name + "light_space").c_str(), this->get_light_space_matrix());
        }
        catch (const std::exception& error) {
            std::cout << "ERROR::DIR_LIGHT::SET_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
            assert(0);
        }
    }

    void set_shadow_width(double shadow_width) {
        if (shadow_width < eps) {
            std::cout << "ERROR::DIR_LIGHT::SET_SHADOW_WIDTH\n" << "Not a positive shadow width.\n";
            assert(0);
        }

        this->shadow_width = shadow_width;
        set_projection_matrix();
    }

    void set_shadow_height(double shadow_height) {
        if (shadow_height < eps) {
            std::cout << "ERROR::DIR_LIGHT::SET_SHADOW_HEIGHT\n" << "Not a positive shadow height.\n";
            assert(0);
        }

        this->shadow_height = shadow_height;
        set_projection_matrix();
    }

    void set_shadow_depth(double shadow_depth) {
        if (shadow_depth < eps) {
            std::cout << "ERROR::DIR_LIGHT::SET_SHADOW_DEPTH\n" << "Not a positive shadow depth.\n";
            assert(0);
        }

        this->shadow_depth = shadow_depth;
        set_projection_matrix();
    }

    eng::Matrix get_light_space_matrix() {
        return projection * get_view_matrix();
    }

    eng::GraphObject get_shadow_box() {
        eng::GraphObject shadow_box = get_cube();
        shadow_box.transparent = true;

        shadow_box.apply_func_meshes([](auto& mesh) {
            mesh.material.set_diffuse(eng::Vect3(1, 1, 1));
            mesh.material.set_alpha(0.3);
        });

        int model_id = shadow_box.add_model(eng::Matrix::scale_matrix(eng::Vect3(shadow_width, shadow_height, shadow_depth)));
        shadow_box.change_matrix_left(eng::Matrix::translation_matrix(eng::Vect3(0, 0, (1 - eps) * shadow_depth / 2)), model_id);
        shadow_box.change_matrix_left(get_view_matrix().inverse(), model_id);

        return shadow_box;
    }
};
