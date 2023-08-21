#pragma once

#include "Light.h"


namespace gre {
    class DirLight : public Light {
        static const uint8_t LIGHT_TYPE = 0;

        double shadow_width_ = 10.0;
        double shadow_height_ = 10.0;
        double shadow_depth_ = 10.0;

        Vec3 direction_;
        Matrix4x4 projection_;

        void set_projection_matrix() {
#ifdef _DEBUG
            if (equality(shadow_width_, 0.0) || equality(shadow_height_, 0.0) || equality(shadow_depth_, 0.0)) {
                throw GreDomainError(__FILE__, __LINE__, "set_projection_matrix, invalid matrix settings.\n\n");
            }
#endif // _DEBUG

            projection_ = Matrix4x4::scale_matrix(Vec3(2.0 / shadow_width_, 2.0 / shadow_height_, 2.0 / shadow_depth_));
            projection_ *= Matrix4x4::translation_matrix(Vec3(0.0, 0.0, -shadow_depth_ / 2.0));
        }

        Matrix4x4 get_view_matrix() const noexcept {
            const Vec3& horizont = direction_.horizont();
            return Matrix4x4(horizont, direction_ ^ horizont, direction_).transpose() * Matrix4x4::translation_matrix(-shadow_position);
        }

    public:
        Vec3 shadow_position = Vec3(0.0);

        explicit DirLight(const Vec3& direction) {
            if (!glew_is_ok()) {
                throw GreRuntimeError(__FILE__, __func__, __LINE__, "DirLight, failed to initialize GLEW.\n\n");
            }

#ifdef _DEBUG
            try {
                direction_ = direction.normalize();
            }
            catch (GreDomainError) {
                throw GreInvalidArgument(__FILE__, __LINE__, "DirLight, the direction vector has zero length.\n\n");
            }
#else
            direction_ = direction.normalize();
#endif // _DEBUG
            
            set_projection_matrix();
        }

        // MAIN shader expected
        void set_uniforms(size_t id, const Shader& shader) const override {
            std::string name = "lights[" + std::to_string(id) + "].";
            set_light_uniforms(name, shader);

            shader.set_uniform_i((name + "type").c_str(), LIGHT_TYPE);
            shader.set_uniform_f((name + "direction").c_str(), direction_);
            if (shadow) {
                shader.set_uniform_matrix((name + "light_space").c_str(), get_light_space_matrix());
            }
        }

        void set_shadow_width(double shadow_width) {
#ifdef _DEBUG
            if (shadow_width < 0.0 || equality(shadow_width, 0.0)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_shadow_width, not a positive shadow width.\n\n");
            }
#endif // _DEBUG

            shadow_width_ = shadow_width;
            set_projection_matrix();
        }

        void set_shadow_height(double shadow_height) {
#ifdef _DEBUG
            if (shadow_height < 0.0 || equality(shadow_height, 0.0)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_shadow_height, not a positive shadow height.\n\n");
            }
#endif // _DEBUG

            shadow_height_ = shadow_height;
            set_projection_matrix();
        }

        void set_shadow_depth(double shadow_depth) {
#ifdef _DEBUG
            if (shadow_depth < 0.0 || equality(shadow_depth, 0.0)) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_shadow_depth, not a positive shadow depth.\n\n");
            }
#endif // _DEBUG

            shadow_depth_ = shadow_depth;
            set_projection_matrix();
        }

        void set_direction(const Vec3& direction) {
#ifdef _DEBUG
            try {
                direction_ = direction.normalize();
            }
            catch (GreDomainError) {
                throw GreInvalidArgument(__FILE__, __LINE__, "set_direction, the direction vector has zero length.\n\n");
            }
#else
            direction_ = direction.normalize();
#endif // _DEBUG
        }

        Matrix4x4 get_light_space_matrix() const noexcept override {
            return projection_ * get_view_matrix();
        }

        GraphObject get_shadow_box() const {
            GraphObject shadow_box = GraphObject::cube(1);
            shadow_box.transparent = true;

            shadow_box.meshes.apply_func([](Mesh& mesh) {
                mesh.material.set_diffuse(Vec3(1.0, 1.0, 1.0));
                mesh.material.set_alpha(0.3);
            });

            Matrix4x4 model = Matrix4x4::scale_matrix(Vec3(shadow_width_, shadow_height_, shadow_depth_));
            model = Matrix4x4::translation_matrix(Vec3(0.0, 0.0, (1.0 - EPS) * shadow_depth_ / 2.0)) * model;
            model = get_view_matrix().inverse() * model;

            shadow_box.models.insert(model);
            return shadow_box;
        }
    };
}
