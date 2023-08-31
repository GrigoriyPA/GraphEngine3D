#pragma once

#include "../../Common/common.hpp"


// Proxy class for openGL shaders
namespace gre {
	class Shader {
		size_t* count_links_ = nullptr;
		std::string* vertex_shader_code_ = nullptr;
		std::string* fragment_shader_code_ = nullptr;
		GLuint program_id_ = 0;

		static std::string load_shader(const std::string& shader_path);

		static GLuint create_vertex_shader(const std::string& code);

		static GLuint create_fragment_shader(const std::string& code);

		static GLuint link_shaders(const std::string& vertex_shader_code, const std::string& fragment_shader_code);

		static std::string find_value(const std::string& code, const std::string& variable_name);

		static uint64_t find_version(const std::string& code);

		static std::string load_shader_info_log(GLuint shader);

		static std::string load_program_info_log(GLuint program);

	public:
		Shader();

		Shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code);

		Shader(const Shader& other) noexcept;

		Shader(Shader&& other) noexcept;

		Shader& operator=(const Shader& other)& noexcept;

		Shader& operator=(Shader&& other)& noexcept;

		void set_shader_code(const std::string& vertex_shader_code, const std::string& fragment_shader_code);

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0) const;

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1) const;

		void set_uniform_f(const GLchar* uniform_name, const Vec2& v) const;

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2) const;

		void set_uniform_f(const GLchar* uniform_name, const Vec3& v) const;

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const;

		void set_uniform_i(const GLchar* uniform_name, GLint v0) const;

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1) const;

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2) const;

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3) const;

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0) const;

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1) const;

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2) const;

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2, GLuint v3) const;

		void set_uniform_1fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const;

		void set_uniform_2fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const;

		void set_uniform_3fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const;

		void set_uniform_4fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const;

		void set_uniform_1iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const;

		void set_uniform_2iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const;

		void set_uniform_3iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const;

		void set_uniform_4iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const;

		void set_uniform_1uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const;

		void set_uniform_2uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const;

		void set_uniform_3uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const;

		void set_uniform_4uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const;

		void set_uniform_matrix(const GLchar* uniform_name, GLsizei count, const GLfloat* value, size_t height, size_t width, GLboolean transpose = GL_FALSE) const;

		void set_uniform_matrix(const GLchar* uniform_name, const Matrix4x4& matrix, GLboolean transpose = GL_FALSE) const;

		std::string get_value_vert(const std::string& variable_name) const;

		std::string get_value_frag(const std::string& variable_name) const;

		GLint get_uniform_location(const GLchar* uniform_name) const;

		GLuint get_program_id() const noexcept;

		bool check_window_settings(const sf::ContextSettings& settings) const;

		void use() const;

		bool validate_program(std::string& validate_status_description) const;

		bool validate_program() const;

		void load_from_file(const std::string& vertex_shader_path, const std::string& fragment_shader_path);

		void swap(Shader& other) noexcept;

		void clear();

		~Shader();

		static GLint get_current_program();
	};
}
