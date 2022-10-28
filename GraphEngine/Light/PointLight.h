#pragma once


class PointLight : public Light {
public:
    double constant = 1, linear = 0, quadratic = 0;

    eng::Vect3 position;

    PointLight(eng::Vect3 position) {
        this->position = position;
    }

    template <size_t T>
    void set_uniforms(int draw_id, eng::Shader<T>* shader_program) {
        if (draw_id < 0) {
            std::cout << "ERROR::POINT_LIGHT::SET_UNIFORMS\n" << "Invalid draw id.\n";
            assert(0);
        }

        try {
            std::string name = "lights[" + std::to_string(draw_id) + "].";
            glUniform1i(glGetUniformLocation(shader_program->program, (name + "shadow").c_str()), 0);
            glUniform1i(glGetUniformLocation(shader_program->program, (name + "type").c_str()), 1);
            glUniform3f(glGetUniformLocation(shader_program->program, (name + "position").c_str()), position.x, position.y, position.z);
            glUniform1f(glGetUniformLocation(shader_program->program, (name + "constant").c_str()), constant);
            glUniform1f(glGetUniformLocation(shader_program->program, (name + "linear").c_str()), linear);
            glUniform1f(glGetUniformLocation(shader_program->program, (name + "quadratic").c_str()), quadratic);
            glUniform3f(glGetUniformLocation(shader_program->program, (name + "ambient").c_str()), ambient.x, ambient.y, ambient.z);
            glUniform3f(glGetUniformLocation(shader_program->program, (name + "diffuse").c_str()), diffuse.x, diffuse.y, diffuse.z);
            glUniform3f(glGetUniformLocation(shader_program->program, (name + "specular").c_str()), specular.x, specular.y, specular.z);
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
        eng::GraphObject light_object = get_cube();

        eng::Mesh::Material material;
        material.set_emission(eng::Vect3(1, 1, 1));
        material.shadow = true;
        light_object.set_material(material);

        int model_id = light_object.add_model(eng::Matrix::scale_matrix(0.25));
        light_object.change_matrix(eng::Matrix::translation_matrix(position), model_id);

        return light_object;
    }
};
