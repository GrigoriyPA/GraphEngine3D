#pragma once

#include "../GraphicClasses/Shader.h"


namespace eng {
    class Light {
    protected:
        inline static double eps_ = 1e-5;

        Vec3 ambient_ = Vec3(0.0, 0.0, 0.0);
        Vec3 diffuse_ = Vec3(0.0, 0.0, 0.0);
        Vec3 specular_ = Vec3(0.0, 0.0, 0.0);

        void set_light_uniforms(const std::string& name, const Shader<size_t>& shader) const {
            if (shader.description != ShaderType::MAIN) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_light_uniforms, invalid shader type.\n\n");
            }

            shader.set_uniform_f((name + "ambient").c_str(), ambient_);
            shader.set_uniform_f((name + "diffuse").c_str(), diffuse_);
            shader.set_uniform_f((name + "specular").c_str(), specular_);
            shader.set_uniform_i((name + "shadow").c_str(), shadow);
        }

    public:
        bool shadow = false;

        Light() {
            if (!glew_is_ok()) {
                throw EngRuntimeError(__FILE__, __LINE__, "Light, failed to initialize GLEW.\n\n");
            }
        }

        void set_ambient(const Vec3& ambient) {
            check_color_value(__FILE__, __LINE__, __func__, ambient);
            ambient_ = ambient;
        }

        void set_diffuse(const Vec3& diffuse) {
            check_color_value(__FILE__, __LINE__, __func__, diffuse);
            diffuse_ = diffuse;
        }

        void set_specular(const Vec3& specular) {
            check_color_value(__FILE__, __LINE__, __func__, specular);
            specular_ = specular;
        }

        virtual void set_uniforms(size_t id, const Shader<size_t>& shader) const = 0;

        virtual Matrix get_light_space_matrix() const = 0;

        virtual ~Light() {
        }

        static void set_epsilon(double eps) {
            if (eps <= 0) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
            }

            eps_ = eps;
        }
    };
}
