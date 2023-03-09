#pragma once

#include "Light.h"
#include "../GraphObjects/GraphObject.h"


namespace gre {
    class PointLight : public Light {
        static const uint8_t LIGHT_TYPE = 1;

        double constant_ = 1.0;
        double linear_ = 0.0;
        double quadratic_ = 0.0;

    public:
        Vec3 position;

        explicit PointLight(const Vec3& position) {
            if (!glew_is_ok()) {
                throw GreRuntimeError(__FILE__, __LINE__, "PointLight, failed to initialize GLEW.\n\n");
            }

            this->position = position;
        }

        // MAIN shader expected
        void set_uniforms(size_t id, const Shader& shader) const override {
            std::string name = "lights[" + std::to_string(id) + "].";
            set_light_uniforms(name, shader);

            shader.set_uniform_i((name + "type").c_str(), LIGHT_TYPE);
            shader.set_uniform_f((name + "constant").c_str(), static_cast<GLfloat>(constant_));
            shader.set_uniform_f((name + "linear").c_str(), static_cast<GLfloat>(linear_));
            shader.set_uniform_f((name + "quadratic").c_str(), static_cast<GLfloat>(quadratic_));
            shader.set_uniform_f((name + "position").c_str(), position);
        }

        void set_constant(double coefficient) {
#ifdef _DEBUG
            if (coefficient < 0.0) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_constant, negative coefficient value.\n\n");
            }
#endif // _DEBUG

            constant_ = coefficient;
        }

        void set_linear(double coefficient) {
#ifdef _DEBUG
            if (coefficient < 0.0) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_linear, negative coefficient value.\n\n");
            }
#endif // _DEBUG

            linear_ = coefficient;
        }

        void set_quadratic(double coefficient) {
#ifdef _DEBUG
            if (coefficient < 0.0) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_quadratic, negative coefficient value.\n\n");
            }
#endif // _DEBUG

            quadratic_ = coefficient;
        }

        Matrix4x4 get_light_space_matrix() const noexcept override {
            return Matrix4x4(0.0);
        }

        GraphObject get_light_object() const {
            GraphObject light_object = GraphObject::sphere(6, true, 1);

            light_object.meshes.apply_func([](Mesh& mesh) {
                mesh.material.set_emission(Vec3(1.0, 1.0, 1.0));
                mesh.material.shadow = false;
            });

            Matrix4x4 model = Matrix4x4::scale_matrix(0.15);
            model = Matrix4x4::translation_matrix(position) * model;

            light_object.models.insert(model);
            return light_object;
        }
    };
}
