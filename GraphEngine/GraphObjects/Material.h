#pragma once

#include "../GraphicClasses/Shader.h"
#include "../GraphicClasses/Texture.h"


namespace eng {
	class Material {
		friend class Mesh;

		double shininess_ = 1.0;
		double alpha_ = 1.0;
		Vect3 ambient_ = Vect3(0.0, 0.0, 0.0);
		Vect3 diffuse_ = Vect3(0.0, 0.0, 0.0);
		Vect3 specular_ = Vect3(0.0, 0.0, 0.0);
		Vect3 emission_ = Vect3(0.0, 0.0, 0.0);

		void set_uniforms(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
			}

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

		void delete_uniforms(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
			}

			shader.use();
			diffuse_map.deactive(0);
			specular_map.deactive(1);
			emission_map.deactive(2);
		}

	public:
		bool shadow = true;
		bool use_vertex_color = false;

		Texture diffuse_map;
		Texture specular_map;
		Texture emission_map;

		Material() {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "Material, failed to initialize GLEW.\n\n");
			}
		}

		bool operator==(const Material& other) const noexcept {
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

		bool operator!=(const Material& other) const noexcept {
			return !(*this == other);
		}

		void set_shininess(double shininess) {
			if (shininess < 0.0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_shininess, invalid shininess value.\n\n");
			}

			shininess_ = shininess;
		}

		void set_alpha(double alpha) {
			if (alpha < 0.0 || 1.0 < alpha) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_alpha, invalid alpha value.\n\n");
			}

			alpha_ = alpha;
		}

		void set_ambient(const Vect3& ambient) {
			check_color_value(__FILE__, __LINE__, __func__, ambient);
			ambient_ = ambient;
		}

		void set_diffuse(const Vect3& diffuse) {
			check_color_value(__FILE__, __LINE__, __func__, diffuse);
			diffuse_ = diffuse;
		}

		void set_specular(const Vect3& specular) {
			check_color_value(__FILE__, __LINE__, __func__, specular);
			specular_ = specular;
		}

		void set_emission(const Vect3& emission) {
			check_color_value(__FILE__, __LINE__, __func__, emission);
			emission_ = emission;
		}
	};
}
