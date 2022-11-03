#pragma once


class PointLight : public Light {
public:
    double constant = 1, linear = 0, quadratic = 0;

    eng::Vect3 position;

    PointLight(eng::Vect3 position) {
        this->position = position;
    }

    void set_uniforms(int draw_id, eng::Shader<size_t>* shader_program) {
        if (draw_id < 0) {
            std::cout << "ERROR::POINT_LIGHT::SET_UNIFORMS\n" << "Invalid draw id.\n";
            assert(0);
        }

        try {
            std::string name = "lights[" + std::to_string(draw_id) + "].";
            shader_program->set_uniform_i((name + "shadow").c_str(), 0);
            shader_program->set_uniform_i((name + "shadow").c_str(), 0);
            shader_program->set_uniform_i((name + "type").c_str(), 1);
            shader_program->set_uniform_f((name + "position").c_str(), position.x, position.y, position.z);
            shader_program->set_uniform_f((name + "constant").c_str(), constant);
            shader_program->set_uniform_f((name + "linear").c_str(), linear);
            shader_program->set_uniform_f((name + "quadratic").c_str(), quadratic);
            shader_program->set_uniform_f((name + "ambient").c_str(), ambient.x, ambient.y, ambient.z);
            shader_program->set_uniform_f((name + "diffuse").c_str(), diffuse.x, diffuse.y, diffuse.z);
            shader_program->set_uniform_f((name + "specular").c_str(), specular.x, specular.y, specular.z);
        }
        catch (const std::exception& error) {
            std::cout << "ERROR::POINT_LIGHT::SET_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
            assert(0);
        }
    }

    eng::Matrix get_light_space_matrix() {
        return eng::Matrix::one_matrix(4);
    }

    eng::GraphObject get_light_object() {
        eng::GraphObject light_object = eng::GraphObject::cube(1);

        light_object.meshes.apply_func([](auto& mesh) {
            mesh.material.set_emission(eng::Vect3(1, 1, 1));
        });

        int model_id = light_object.models.insert(eng::Matrix::scale_matrix(0.25));
        light_object.models.change_left(model_id, eng::Matrix::translation_matrix(position));

        return light_object;
    }
};
