#include "Material.hpp"


// Material
namespace gre {
    // MAIN shader expected
    void Material::set_uniforms(const Shader& shader) const {
        shader.set_uniform_i("use_diffuse_map", diffuse_map.get_id() != 0);
        shader.set_uniform_i("use_specular_map", specular_map.get_id() != 0);
        shader.set_uniform_i("use_emission_map", emission_map.get_id() != 0);

        if (diffuse_map.get_id() == 0) {
            shader.set_uniform_f("object_material.ambient", ambient_);
            shader.set_uniform_f("object_material.diffuse", diffuse_);
            shader.set_uniform_f("object_material.alpha", static_cast<GLfloat>(alpha_));
        }

        if (specular_map.get_id() == 0) {
            shader.set_uniform_f("object_material.specular", specular_);
        }
        shader.set_uniform_f("object_material.shininess", static_cast<GLfloat>(shininess_));

        if (emission_map.get_id() == 0) {
            shader.set_uniform_f("object_material.emission", emission_);
        }

        shader.set_uniform_i("object_material.use_vertex_color", use_vertex_color);
        shader.set_uniform_i("object_material.shadow", shadow);

        diffuse_map.activate(0);
        specular_map.activate(1);
        emission_map.activate(2);
    }

    // MAIN shader expected
    void Material::delete_uniforms(const Shader& shader) const {
        shader.use();
        diffuse_map.deactive(0);
        specular_map.deactive(1);
        emission_map.deactive(2);
    }

    Material::Material() {
    }

    bool Material::operator==(const Material& other) const noexcept {
        if (shadow != other.shadow || use_vertex_color != other.use_vertex_color) {
            return false;
        }
        if (shininess_ != other.shininess_ || alpha_ != other.alpha_) {
            return false;
        }
        if (ambient_ != other.ambient_ || diffuse_ != other.diffuse_ || specular_ != other.specular_ || emission_ != other.emission_) {
            return false;
        }
        return diffuse_map == other.diffuse_map && specular_map == other.specular_map && emission_map == other.emission_map;
    }

    bool Material::operator!=(const Material& other) const noexcept {
        return !(*this == other);
    }

    void Material::set_shininess(double shininess) {
        GRE_ENSURE(shininess >= 0.0, GreInvalidArgument, "invalid shininess value");

        shininess_ = shininess;
    }

    void Material::set_alpha(double alpha) {
        GRE_ENSURE(0.0 <= alpha && alpha <= 1.0, GreInvalidArgument, "invalid alpha value");

        alpha_ = alpha;
    }

    void Material::set_ambient(double red, double green, double blue) {
        set_ambient(Vec3(red, green, blue));
    }

    void Material::set_ambient(const Vec3& ambient) {
        ambient_ = ambient;
    }

    void Material::set_diffuse(double red, double green, double blue) {
        set_diffuse(Vec3(red, green, blue));
    }

    void Material::set_diffuse(const Vec3& diffuse) {
        diffuse_ = diffuse;
    }

    void Material::set_specular(double red, double green, double blue) {
        set_specular(Vec3(red, green, blue));
    }

    void Material::set_specular(const Vec3& specular) {
        specular_ = specular;
    }

    void Material::set_emission(double red, double green, double blue) {
        set_emission(Vec3(red, green, blue));
    }

    void Material::set_emission(const Vec3& emission) {
        emission_ = emission;
    }
}  // namespace gre
