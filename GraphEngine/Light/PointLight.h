#pragma once

#include "Light.h"
#include "../GraphObjects/GraphObject.h"


namespace eng {
    class PointLight : public Light {
        static const uint8_t LIGHT_TYPE = 1;

        double constant_ = 1.0;
        double linear_ = 0.0;
        double quadratic_ = 0.0;

    public:
        Vect3 position;

        PointLight(const Vect3& position) {
            if (!glew_is_ok()) {
                throw EngRuntimeError(__FILE__, __LINE__, "PointLight, failed to initialize GLEW.\n\n");
            }

            this->position = position;
        }

        void set_uniforms(size_t id, const Shader<size_t>& shader) const override {
            if (shader.description != ShaderType::MAIN) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
            }

            std::string name = "lights[" + std::to_string(id) + "].";
            set_light_uniforms(name, shader);

            shader.set_uniform_i((name + "type").c_str(), LIGHT_TYPE);
            shader.set_uniform_f((name + "constant").c_str(), constant_);
            shader.set_uniform_f((name + "linear").c_str(), linear_);
            shader.set_uniform_f((name + "quadratic").c_str(), quadratic_);
            shader.set_uniform_f((name + "position").c_str(), position.x, position.y, position.z);
        }

        PointLight& set_constant(double coefficient) {
            if (coefficient < 0.0) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_constant, negative coefficient value.\n\n");
            }

            constant_ = coefficient;
            return *this;
        }

        PointLight& set_linear(double coefficient) {
            if (coefficient < 0.0) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_linear, negative coefficient value.\n\n");
            }

            linear_ = coefficient;
            return *this;
        }

        PointLight& set_quadratic(double coefficient) {
            if (coefficient < 0.0) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_quadratic, negative coefficient value.\n\n");
            }

            quadratic_ = coefficient;
            return *this;
        }

        Matrix get_light_space_matrix() const noexcept override {
            return Matrix(4, 4);
        }

        GraphObject get_light_object() const {
            GraphObject light_object = GraphObject::sphere(6, true, 1);

            light_object.meshes.apply_func([](auto& mesh) {
                mesh.material.set_emission(Vect3(1.0, 1.0, 1.0));
                mesh.material.shadow = false;
            });

            Matrix model = Matrix::scale_matrix(0.15);
            model = Matrix::translation_matrix(position) * model;

            light_object.models.insert(model);
            return light_object;
        }
    };
}
