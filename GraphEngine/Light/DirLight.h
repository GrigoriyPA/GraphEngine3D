#pragma once


class DirLight : public eng::Light {
    double shadow_width = 10, shadow_height = 10, shadow_depth = 10;

    eng::Vect3 direction;
    eng::Matrix projection;

    void set_projection_matrix() {
        projection = eng::Matrix::scale_matrix(eng::Vect3(2.0 / shadow_width, 2.0 / shadow_height, 2.0 / shadow_depth)) * eng::Matrix::translation_matrix(eng::Vect3(0, 0, -shadow_depth / 2));
    }

    eng::Matrix get_view_matrix() const {
        eng::Vect3 horizont = direction.horizont();

        return eng::Matrix(horizont, direction ^ horizont, direction).transpose() * eng::Matrix::translation_matrix(-shadow_position);
    }

public:
    eng::Vect3 shadow_position = eng::Vect3(0, 0, 0);

    DirLight(eng::Vect3 direction) : projection(eng::Matrix::one_matrix(4)) {
        if (direction.length() < eps_) {
            std::cout << "ERROR::DIR_LIGHT::BUILDER\n" << "The direction vector has zero length.\n";
            assert(0);
        }

        this->direction = direction.normalize();

        set_projection_matrix();
    }

    void set_uniforms(size_t draw_id, const eng::Shader<size_t>& shader_program) const {
        if (draw_id < 0) {
            std::cout << "ERROR::DIR_LIGHT::SET_UNIFORMS\n" << "Invalid draw id.\n";
            assert(0);
        }

        try {
            shader_program.set_uniform_i(("lights[" + std::to_string(draw_id) + "].exist").c_str(), 1);
            std::string name = "lights[" + std::to_string(draw_id) + "].";
            shader_program.set_uniform_i((name + "shadow").c_str(), shadow);
            shader_program.set_uniform_i((name + "type").c_str(), 0);
            shader_program.set_uniform_f((name + "direction").c_str(), direction.x, direction.y, direction.z);
            shader_program.set_uniform_f((name + "ambient").c_str(), ambient_.x, ambient_.y, ambient_.z);
            shader_program.set_uniform_f((name + "diffuse").c_str(), diffuse_.x, diffuse_.y, diffuse_.z);
            shader_program.set_uniform_f((name + "specular").c_str(), specular_.x, specular_.y, specular_.z);
            if (shadow)
                shader_program.set_uniform_matrix((name + "light_space").c_str(), this->get_light_space_matrix());
        }
        catch (const std::exception& error) {
            std::cout << "ERROR::DIR_LIGHT::SET_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
            assert(0);
        }
    }

    void set_shadow_width(double shadow_width) {
        if (shadow_width < eps_) {
            std::cout << "ERROR::DIR_LIGHT::SET_SHADOW_WIDTH\n" << "Not a positive shadow width.\n";
            assert(0);
        }

        this->shadow_width = shadow_width;
        set_projection_matrix();
    }

    void set_shadow_height(double shadow_height) {
        if (shadow_height < eps_) {
            std::cout << "ERROR::DIR_LIGHT::SET_SHADOW_HEIGHT\n" << "Not a positive shadow height.\n";
            assert(0);
        }

        this->shadow_height = shadow_height;
        set_projection_matrix();
    }

    void set_shadow_depth(double shadow_depth) {
        if (shadow_depth < eps_) {
            std::cout << "ERROR::DIR_LIGHT::SET_SHADOW_DEPTH\n" << "Not a positive shadow depth.\n";
            assert(0);
        }

        this->shadow_depth = shadow_depth;
        set_projection_matrix();
    }

    eng::Matrix get_light_space_matrix() const {
        return projection * get_view_matrix();
    }

    eng::GraphObject get_shadow_box() {
        eng::GraphObject shadow_box = eng::GraphObject::cube(1);
        shadow_box.transparent = true;

        shadow_box.meshes.apply_func([](auto& mesh) {
            mesh.material.set_diffuse(eng::Vect3(1, 1, 1));
            mesh.material.set_alpha(0.3);
        });

        int model_id = shadow_box.models.insert(eng::Matrix::scale_matrix(eng::Vect3(shadow_width, shadow_height, shadow_depth)));
        shadow_box.models.change_left(model_id, eng::Matrix::translation_matrix(eng::Vect3(0, 0, (1 - eps_) * shadow_depth / 2)));
        shadow_box.models.change_left(model_id, get_view_matrix().inverse());

        return shadow_box;
    }
};
