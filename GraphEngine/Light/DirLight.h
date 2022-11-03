#pragma once

#include "Light.h"
#include "../GraphObjects/GraphObject.h"


namespace eng {
    class DirLight : public Light {
        static const uint8_t LIGHT_TYPE = 0;

        double shadow_width_ = 1.0;
        double shadow_height_ = 1.0;
        double shadow_depth_ = 1.0;

        Vect3 direction_;
        Matrix projection_;

        void set_projection_matrix() {
            if (equality(shadow_width_, 0.0, eps_) || equality(shadow_height_, 0.0, eps_) || equality(shadow_depth_, 0.0, eps_)) {
                throw EngDomainError(__FILE__, __LINE__, "set_projection_matrix, invalid matrix settings.\n\n");
            }

            projection_ = Matrix::scale_matrix(Vect3(2.0 / shadow_width_, 2.0 / shadow_height_, 2.0 / shadow_depth_));
            projection_ *= Matrix::translation_matrix(Vect3(0, 0, -shadow_depth_ / 2));
        }

        Matrix get_view_matrix() const noexcept {
            const Vect3& horizont = direction_.horizont();
            return Matrix(horizont, direction_ ^ horizont, direction_).transpose() * Matrix::translation_matrix(-shadow_position);
        }

    public:
        Vect3 shadow_position = Vect3(0.0, 0.0, 0.0);

        DirLight(const Vect3& direction) : projection_(4, 4) {
            if (!glew_is_ok()) {
                throw EngRuntimeError(__FILE__, __LINE__, "DirLight, failed to initialize GLEW.\n\n");
            }

            try {
                direction_ = direction.normalize();
            }
            catch (EngDomainError) {
                throw EngInvalidArgument(__FILE__, __LINE__, "DirLight, the direction vector has zero length.\n\n");
            }
            
            set_projection_matrix();
        }

        void set_uniforms(size_t id, const Shader<size_t>& shader) const {
            if (shader.description != ShaderType::MAIN) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
            }

            std::string name = "lights[" + std::to_string(id) + "].";
            set_light_uniforms(name, shader);

            shader.set_uniform_i((name + "type").c_str(), LIGHT_TYPE);
            shader.set_uniform_f((name + "direction").c_str(), direction_.x, direction_.y, direction_.z);
            if (shadow) {
                shader.set_uniform_matrix((name + "light_space").c_str(), get_light_space_matrix());
            }
        }

        DirLight& set_shadow_width(double shadow_width) {
            if (shadow_width < 0.0 || equality(shadow_width, 0.0, eps_)) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_shadow_width, not a positive shadow width.\n\n");
            }

            shadow_width_ = shadow_width;
            set_projection_matrix();
            return *this;
        }

        DirLight& set_shadow_height(double shadow_height) {
            if (shadow_height < 0.0 || equality(shadow_height, 0.0, eps_)) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_shadow_height, not a positive shadow height.\n\n");
            }

            shadow_height_ = shadow_height;
            set_projection_matrix();
            return *this;
        }

        DirLight& set_shadow_depth(double shadow_depth) {
            if (shadow_depth < 0.0 || equality(shadow_depth, 0.0, eps_)) {
                throw EngInvalidArgument(__FILE__, __LINE__, "set_shadow_depth, not a positive shadow depth.\n\n");
            }

            shadow_depth_ = shadow_depth;
            set_projection_matrix();
            return *this;
        }

        Matrix get_light_space_matrix() const noexcept {
            return projection_ * get_view_matrix();
        }

        GraphObject get_shadow_box() const {
            GraphObject shadow_box = GraphObject::cube(1);
            shadow_box.transparent = true;

            shadow_box.meshes.apply_func([](auto& mesh) {
                mesh.material.set_diffuse(Vect3(1, 1, 1));
                mesh.material.set_alpha(0.3);
            });

            Matrix model = Matrix::scale_matrix(Vect3(shadow_width_, shadow_height_, shadow_depth_));
            model = Matrix::translation_matrix(Vect3(0, 0, (1 - eps_) * shadow_depth_ / 2)) * model;
            model = get_view_matrix().inverse() * model;

            shadow_box.models.insert(model);
            return shadow_box;
        }
    };
}
