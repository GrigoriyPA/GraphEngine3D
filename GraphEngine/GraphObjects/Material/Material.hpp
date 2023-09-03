#pragma once

#include "../../GraphicClasses/graphic_classes.h"


// Mesh material description
namespace gre {
    class Material {
        friend class Mesh;

        double shininess_ = 1.0;
        double alpha_ = 1.0;
        Vec3 ambient_ = Vec3(1.0);
        Vec3 diffuse_ = Vec3(1.0);
        Vec3 specular_ = Vec3(0.0);
        Vec3 emission_ = Vec3(0.0);

        // MAIN shader expected
        void set_uniforms(const Shader& shader) const;

        // MAIN shader expected
        void delete_uniforms(const Shader& shader) const;

    public:
        bool shadow = true;
        bool use_vertex_color = false;

        Texture diffuse_map;
        Texture specular_map;
        Texture emission_map;

        Material();

        bool operator==(const Material& other) const noexcept;

        bool operator!=(const Material& other) const noexcept;

        void set_shininess(double shininess);

        void set_alpha(double alpha);

        void set_ambient(double red, double green, double blue);

        void set_ambient(const Vec3& ambient);

        void set_diffuse(double red, double green, double blue);

        void set_diffuse(const Vec3& diffuse);

        void set_specular(double red, double green, double blue);

        void set_specular(const Vec3& specular);

        void set_emission(double red, double green, double blue);

        void set_emission(const Vec3& emission);
    };
}  // namespace gre
