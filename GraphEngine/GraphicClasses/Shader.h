#pragma once

#include <fstream>
#include "GraphicFunctions.h"


namespace eng {
	template <size_t T = 0>
	class Shader {
		static const GLsizei MAX_SHADER_INFO_LOG_SIZE = 1024;
		inline static const std::string VERTEX_SHADER_EXTENSION = ".vert";
		inline static const std::string FRAGMENT_SHADER_EXTENSION = ".frag";

		size_t* count_links_ = nullptr;
		std::string* vertex_shader_code_ = nullptr;
		std::string* fragment_shader_code_ = nullptr;
		GLuint program_ = 0;

		GLuint load_vertex_shader(const std::string& vertex_shader_path) {
			std::ifstream vertex_shader_file(vertex_shader_path + VERTEX_SHADER_EXTENSION);
			if (vertex_shader_file.fail()) {
				throw EngRuntimeError(__FILE__, __LINE__, "load_vertex_shader, the vertex shader file does not exist.");
			}

			vertex_shader_code_ = new std::string();
			for (std::string line; std::getline(vertex_shader_file, line);) {
				*vertex_shader_code_ += line + "\n";
			}

			const char* vertex_shader_code_c = vertex_shader_code_->c_str();
			GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader, 1, &vertex_shader_code_c, NULL);
			glCompileShader(vertex_shader);

			GLint success;
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
			if (success == GL_FALSE) {
				GLchar info_log[MAX_SHADER_INFO_LOG_SIZE];
				glGetShaderInfoLog(vertex_shader, MAX_SHADER_INFO_LOG_SIZE, NULL, info_log);
				throw EngRuntimeError(__FILE__, __LINE__, "load_vertex_shader, compilation failed, description \\/\n" + std::string(info_log));
			}

			check_gl_errors(__FILE__, __LINE__, __func__);
			return vertex_shader;
		}

		GLuint load_fragment_shader(const std::string& fragment_shader_path) {
			std::ifstream fragment_shader_file(fragment_shader_path + FRAGMENT_SHADER_EXTENSION);
			if (fragment_shader_file.fail()) {
				throw EngRuntimeError(__FILE__, __LINE__, "load_fragment_shader, the fragment shader file does not exist.");
			}

			fragment_shader_code_ = new std::string();
			for (std::string line; std::getline(fragment_shader_file, line);) {
				*fragment_shader_code_ += line + "\n";
			}

			const char* fragment_shader_code_c = fragment_shader_code_->c_str();
			GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &fragment_shader_code_c, NULL);
			glCompileShader(fragment_shader);

			GLint success;
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
			if (success == GL_FALSE) {
				GLchar info_log[MAX_SHADER_INFO_LOG_SIZE];
				glGetShaderInfoLog(fragment_shader, MAX_SHADER_INFO_LOG_SIZE, NULL, info_log);
				throw EngRuntimeError(__FILE__, __LINE__, "load_fragment_shader, compilation failed, description \\/\n" + std::string(info_log));
			}

			check_gl_errors(__FILE__, __LINE__, __func__);
			return fragment_shader;
		}

		void Deallocate() noexcept {
			if (count_links_ != nullptr) {
				--(*count_links_);
				if (*count_links_ == 0) {
					delete count_links_;
					delete vertex_shader_code_;
					delete fragment_shader_code_;
					glDeleteProgram(program_);
				}
			}
			count_links_ = nullptr;
			vertex_shader_code_ = nullptr;
			fragment_shader_code_ = nullptr;
			program_ = 0;
		}

		void Swap(Shader<T>& other) noexcept {
			std::swap(count_links_, other.count_links_);
			std::swap(program_, other.program_);
			std::swap(vertex_shader_code_, other.vertex_shader_code_);
			std::swap(fragment_shader_code_, other.fragment_shader_code_);
		}

		static GLuint link_shaders(GLuint vertex_shader, GLuint fragment_shader) {
			GLuint program = glCreateProgram();
			glAttachShader(program, vertex_shader);
			glAttachShader(program, fragment_shader);
			glLinkProgram(program);

			GLint success;
			glGetProgramiv(program, GL_LINK_STATUS, &success);
			if (success == GL_FALSE) {
				GLchar info_log[MAX_SHADER_INFO_LOG_SIZE];
				glGetProgramInfoLog(program, MAX_SHADER_INFO_LOG_SIZE, NULL, info_log);
				throw EngRuntimeError(__FILE__, __LINE__, "link_shaders, linking failed, description \\/\n" + std::string(info_log));
			}

			check_gl_errors(__FILE__, __LINE__, __func__);
			return program;
		}

		static std::string find_value(const std::string& code, const std::string& variable_name) {
			std::vector<std::string> split_code = split(code, [](const char c) { return c == ' ' || c == '\n'; });
			if (split_code.size() < 3) {
				throw EngInvalidArgument(__FILE__, __LINE__, "find_value, variable not found.\n\n");
			}

			for (size_t i = 0; i < split_code.size() - 2; ++i) {
				if (split_code[i] == variable_name && split_code[i + 1] == "=") {
					split_code[i + 2].pop_back();
					return split_code[i + 2];
				}
			}
			throw EngInvalidArgument(__FILE__, __LINE__, "find_value, variable not found.\n\n");
		}

	public:
		Shader() {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "Shader, failed to initialize GLEW.\n\n");
			}
		}

		Shader(const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "Shader, failed to initialize GLEW.\n\n");
			}

			count_links_ = new size_t(1);

			GLuint vertex_shader = load_vertex_shader(vertex_shader_path);
			GLuint fragment_shader = load_fragment_shader(fragment_shader_path);

			program_ = link_shaders(vertex_shader, fragment_shader);

			glDeleteShader(vertex_shader);
			glDeleteShader(fragment_shader);

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		Shader(const Shader<T>& other) noexcept {
			vertex_shader_code_ = other.vertex_shader_code_;
			fragment_shader_code_ = other.fragment_shader_code_;
			program_ = other.program_;
			count_links_ = other.count_links_;
			if (count_links_ != nullptr) {
				++(*count_links_);
			}
		}

		Shader(Shader<T>&& other) noexcept {
			Swap(other);
		}

		Shader<T>& operator=(const Shader<T>& other)& noexcept {
			Shader<T> object(other);
			Swap(object);
			return *this;
		}

		Shader<T>& operator=(Shader<T>&& other)& noexcept {
			Deallocate();
			Swap(other);
			return *this;
		}

		void set_uniform_1f(const GLchar* uniform_name, GLfloat v0) const {
			glUniform1f(glGetUniformLocation(program_, uniform_name), v0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_2f(const GLchar* uniform_name, GLfloat v0, GLfloat v1) const {
			glUniform2f(glGetUniformLocation(program_, uniform_name), v0, v1);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_3f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2) const {
			glUniform3f(glGetUniformLocation(program_, uniform_name), v0, v1, v2);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_4f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const {
			glUniform4f(glGetUniformLocation(program_, uniform_name), v0, v1, v2, v3);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_1i(const GLchar* uniform_name, GLint v0) const {
			glUniform1i(glGetUniformLocation(program_, uniform_name), v0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_2i(const GLchar* uniform_name, GLint v0, GLint v1) const {
			glUniform2i(glGetUniformLocation(program_, uniform_name), v0, v1);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_3i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2) const {
			glUniform3i(glGetUniformLocation(program_, uniform_name), v0, v1, v2);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_4i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3) const {
			glUniform4i(glGetUniformLocation(program_, uniform_name), v0, v1, v2, v3);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_1ui(const GLchar* uniform_name, GLuint v0) const {
			glUniform1ui(glGetUniformLocation(program_, uniform_name), v0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_2ui(const GLchar* uniform_name, GLuint v0, GLuint v1) const {
			glUniform2ui(glGetUniformLocation(program_, uniform_name), v0, v1);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_3ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2) const {
			glUniform3ui(glGetUniformLocation(program_, uniform_name), v0, v1, v2);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_4ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2, GLuint v3) const {
			glUniform4ui(glGetUniformLocation(program_, uniform_name), v0, v1, v2, v3);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_1fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			glUniform1fv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_2fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			glUniform2fv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_3fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			glUniform3fv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_4fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			glUniform4fv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_1iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			glUniform1iv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_2iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			glUniform2iv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_3iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			glUniform3iv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_4iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			glUniform4iv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_1uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			glUniform1uiv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_2uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			glUniform1uiv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_3uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			glUniform1uiv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_4uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			glUniform1uiv(glGetUniformLocation(program_, uniform_name), count, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix2fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix2fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix3fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix3fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix4fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix4fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix2x3fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix2x3fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix3x2fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix3x2fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix2x4fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix2x4fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix4x2fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix4x2fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix3x4fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix3x4fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix4x3fv(const GLchar* uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value) const {
			glUniformMatrix4x3fv(glGetUniformLocation(program_, uniform_name), count, transpose, value);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		std::string get_value_vert(const std::string& variable_name) const {
			return find_value(*vertex_shader_code_, variable_name);
		}

		std::string get_value_frag(const std::string& variable_name) const {
			return find_value(*fragment_shader_code_, variable_name);
		}

		void use() const {
			glUseProgram(program_);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		~Shader() noexcept {
			Deallocate();
		}

		static size_t get_type() noexcept {
			return T;
		}
	};
}
