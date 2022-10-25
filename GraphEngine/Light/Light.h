#pragma once

#include <math.h>
#include <cassert>
#include <iostream>
#include <string>


class Light {
protected:
    double eps = 0.00001;

public:
    eng::Vect3 ambient = eng::Vect3(0, 0, 0), diffuse = eng::Vect3(0, 0, 0), specular = eng::Vect3(0, 0, 0);
    bool shadow = false;

    Light() {
    }

    virtual eng::Matrix get_light_space_matrix() = 0;

    virtual void set_uniforms(int draw_id, eng::Shader<eng::ShaderType::MAIN>* shader_program) = 0;
};

void set_default_light_uniforms(int draw_id, eng::Shader<eng::ShaderType::MAIN>* shader_program) {
    if (draw_id < 0) {
        std::cout << "ERROR::SET_DEFAULT_LIGHT_UNIFORMS\n" << "Invalid draw id.\n";
        assert(0);
    }

    try {
        std::string name = "lights[" + std::to_string(draw_id) + "].";
        shader_program->set_uniform_i((name + "shadow").c_str(), 0);
        shader_program->set_uniform_i((name + "type").c_str(), 0);
        shader_program->set_uniform_f((name + "direction").c_str(), 1, 0, 0);
        shader_program->set_uniform_f((name + "ambient").c_str(), 0, 0, 0);
        shader_program->set_uniform_f((name + "diffuse").c_str(), 0, 0, 0);
        shader_program->set_uniform_f((name + "specular").c_str(), 0, 0, 0);
    }
    catch (const std::exception& error) {
        std::cout << "ERROR::SET_DEFAULT_LIGHT_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
        assert(0);
    }
}

#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"
