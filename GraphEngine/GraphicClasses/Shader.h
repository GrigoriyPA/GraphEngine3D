#pragma once

#include <fstream>
#include "GraphicFunctions.h"
#include "../CommonClasses/Matrix.h"


namespace gre {
	class Shader {
		size_t* count_links_ = nullptr;
		std::string* vertex_shader_code_ = nullptr;
		std::string* fragment_shader_code_ = nullptr;
		GLuint program_id_ = 0;

		std::string load_shader(const std::string& shader_path) {
			std::ifstream shader_file(shader_path);
			if (shader_file.fail()) {
				throw GreRuntimeError(__FILE__, __LINE__, "load_shader, the shader file does not exist.\n\n");
			}

			std::string shader_code;
			for (std::string line; std::getline(shader_file, line);) {
				shader_code += line + "\n";
			}
			return shader_code;
		}

		static GLuint create_vertex_shader(const std::string& code) {
			const char* vertex_shader_code_c = code.c_str();
			GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader, 1, &vertex_shader_code_c, NULL);
			glCompileShader(vertex_shader);

#ifdef _DEBUG
			GLint success;
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
			if (success == GL_FALSE) {
				throw GreRuntimeError(__FILE__, __LINE__, "create_vertex_shader, compilation failed, description \\/\n" + load_shader_info_log(vertex_shader) + "\n\n");
			}
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			return vertex_shader;
		}

		static GLuint create_fragment_shader(const std::string& code) {
			const char* fragment_shader_code_c = code.c_str();
			GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &fragment_shader_code_c, NULL);
			glCompileShader(fragment_shader);

#ifdef _DEBUG
			GLint success;
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
			if (success == GL_FALSE) {
				throw GreRuntimeError(__FILE__, __LINE__, "create_fragment_shader, compilation failed, description \\/\n" + load_shader_info_log(fragment_shader) + "\n\n");
			}
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			return fragment_shader;
		}

		static GLuint link_shaders(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
			GLuint vertex_shader = create_vertex_shader(vertex_shader_code);
			GLuint fragment_shader = create_fragment_shader(fragment_shader_code);

			GLuint program = glCreateProgram();
			glAttachShader(program, vertex_shader);
			glAttachShader(program, fragment_shader);
			glLinkProgram(program);

#ifdef _DEBUG
			GLint success;
			glGetProgramiv(program, GL_LINK_STATUS, &success);
			if (success == GL_FALSE) {
				throw GreRuntimeError(__FILE__, __LINE__, "link_shaders, linking failed, description \\/\n" + load_program_info_log(program) + "\n\n");
			}
#endif // _DEBUG

			glDeleteShader(vertex_shader);
			glDeleteShader(fragment_shader);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
			return program;
		}

		static std::string find_value(const std::string& code, const std::string& variable_name) {
			std::vector<std::string> split_code = split(code, [](const char c) { return c == ' ' || c == '\n'; });
#ifdef _DEBUG
			if (split_code.size() < 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "find_value, variable not found.\n\n");
			}
#endif // _DEBUG

			for (size_t i = 0; i < split_code.size() - 2; ++i) {
				if (split_code[i] == variable_name && split_code[i + 1] == "=") {
					split_code[i + 2].pop_back();
					return split_code[i + 2];
				}
			}
			throw GreInvalidArgument(__FILE__, __LINE__, "find_value, variable not found.\n\n");
		}

		static uint64_t find_version(const std::string& code) {
			std::vector<std::string> split_code = split(code, [](const char c) { return c == ' ' || c == '\n'; });
#ifdef _DEBUG
			if (split_code.size() < 2) {
				throw GreInvalidArgument(__FILE__, __LINE__, "find_version, version not found.\n\n");
			}
#endif // _DEBUG

			for (size_t i = 0; i < split_code.size() - 1; ++i) {
				if (split_code[i] == "#version") {
					return std::stoull(split_code[i + 1]);
				}
			}
			throw GreInvalidArgument(__FILE__, __LINE__, "find_version, version not found.\n\n");
		}

		static std::string load_shader_info_log(GLuint shader) {
			GLint info_log_size = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_size);

			GLchar* info_log = new GLchar[info_log_size];
			glGetShaderInfoLog(shader, info_log_size, NULL, info_log);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			std::string result(info_log);
			delete[] info_log;
			return result;
		}

		static std::string load_program_info_log(GLuint program) {
			GLint info_log_size = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_size);

			GLchar* info_log = new GLchar[info_log_size];
			glGetProgramInfoLog(program, info_log_size, NULL, info_log);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			std::string result(info_log);
			delete[] info_log;
			return result;
		}

	public:
		Shader() {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Shader, failed to initialize GLEW.\n\n");
			}
		}

		Shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Shader, failed to initialize GLEW.\n\n");
			}

			set_shader_code(vertex_shader_code, fragment_shader_code);
		}

		Shader(const Shader& other) noexcept {
			vertex_shader_code_ = other.vertex_shader_code_;
			fragment_shader_code_ = other.fragment_shader_code_;
			count_links_ = other.count_links_;
			if (count_links_ != nullptr) {
				++(*count_links_);
			}

			program_id_ = link_shaders(*vertex_shader_code_, *fragment_shader_code_);
		}

		Shader(Shader&& other) noexcept {
			swap(other);
		}

		Shader& operator=(const Shader& other)& noexcept {
			Shader object(other);
			swap(object);
			return *this;
		}

		Shader& operator=(Shader&& other)& noexcept {
			clear();
			swap(other);
			return *this;
		}

		void set_shader_code(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
			clear();

			count_links_ = new size_t(1);
			vertex_shader_code_ = new std::string(vertex_shader_code);
			fragment_shader_code_ = new std::string(fragment_shader_code);
			program_id_ = link_shaders(*vertex_shader_code_, *fragment_shader_code_);
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0) const {
			use();
			glUniform1f(get_uniform_location(uniform_name), v0);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1) const {
			use();
			glUniform2f(get_uniform_location(uniform_name), v0, v1);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_f(const GLchar* uniform_name, const Vec2& v) const {
			use();
			glUniform2f(get_uniform_location(uniform_name), static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y));
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2) const {
			use();
			glUniform3f(get_uniform_location(uniform_name), v0, v1, v2);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_f(const GLchar* uniform_name, const Vec3& v) const {
			use();
			glUniform3f(get_uniform_location(uniform_name), static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y), static_cast<GLfloat>(v.z));
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const {
			use();
			glUniform4f(get_uniform_location(uniform_name), v0, v1, v2, v3);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0) const {
			use();
			glUniform1i(get_uniform_location(uniform_name), v0);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1) const {
			use();
			glUniform2i(get_uniform_location(uniform_name), v0, v1);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2) const {
			use();
			glUniform3i(get_uniform_location(uniform_name), v0, v1, v2);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3) const {
			use();
			glUniform4i(get_uniform_location(uniform_name), v0, v1, v2, v3);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0) const {
			use();
			glUniform1ui(get_uniform_location(uniform_name), v0);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1) const {
			use();
			glUniform2ui(get_uniform_location(uniform_name), v0, v1);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2) const {
			use();
			glUniform3ui(get_uniform_location(uniform_name), v0, v1, v2);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2, GLuint v3) const {
			use();
			glUniform4ui(get_uniform_location(uniform_name), v0, v1, v2, v3);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_1fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			use();
			glUniform1fv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_2fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			use();
			glUniform2fv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_3fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			use();
			glUniform3fv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_4fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			use();
			glUniform4fv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_1iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			use();
			glUniform1iv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_2iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			use();
			glUniform2iv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_3iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			use();
			glUniform3iv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_4iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			use();
			glUniform4iv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_1uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			use();
			glUniform1uiv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_2uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			use();
			glUniform2uiv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_3uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			use();
			glUniform3uiv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_4uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			use();
			glUniform4uiv(get_uniform_location(uniform_name), count, value);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_matrix(const GLchar* uniform_name, GLsizei count, const GLfloat* value, size_t height, size_t width, GLboolean transpose = GL_FALSE) const {
			use();
			switch (height) {
			case 2:
				switch (height) {
				case 2:
					glUniformMatrix2fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				case 3:
					glUniformMatrix2x3fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				case 4:
					glUniformMatrix2x4fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				default:
					throw GreInvalidArgument(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
					break;
				}
				break;
			case 3:
				switch (height) {
				case 2:
					glUniformMatrix3x2fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				case 3:
					glUniformMatrix3fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				case 4:
					glUniformMatrix3x4fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				default:
					throw GreInvalidArgument(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
					break;
				}
				break;
			case 4:
				switch (height) {
				case 2:
					glUniformMatrix4x2fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				case 3:
					glUniformMatrix4x3fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				case 4:
					glUniformMatrix4fv(get_uniform_location(uniform_name), count, transpose, value);
					break;
				default:
					throw GreInvalidArgument(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
					break;
				}
				break;
			default:
				throw GreInvalidArgument(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
				break;
			}
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_uniform_matrix(const GLchar* uniform_name, const Matrix4x4& matrix, GLboolean transpose = GL_FALSE) const {
			use();
			glUniformMatrix4fv(get_uniform_location(uniform_name), 1, transpose, &std::vector<GLfloat>(matrix)[0]);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		std::string get_value_vert(const std::string& variable_name) const {
			return find_value(*vertex_shader_code_, variable_name);
		}

		std::string get_value_frag(const std::string& variable_name) const {
			return find_value(*fragment_shader_code_, variable_name);
		}

		GLint get_uniform_location(const GLchar* uniform_name) const {
			GLint uniform_location = glGetUniformLocation(program_id_, uniform_name);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
			return uniform_location;
		}

		GLuint get_program_id() const noexcept {
			return program_id_;
		}

		bool check_window_settings(const sf::ContextSettings& settings) const {
			uint64_t vert_version = find_version(*vertex_shader_code_);
			uint64_t frag_version = find_version(*fragment_shader_code_);
			if (vert_version / 100 > settings.majorVersion || (vert_version / 100 == settings.majorVersion && (vert_version % 100) / 10 > settings.minorVersion)) {
				return false;
			}
			return vert_version / 100 < settings.majorVersion || (vert_version / 100 == settings.majorVersion && (vert_version % 100) / 10 <= settings.minorVersion);
		}

		void use() const {
			glUseProgram(program_id_);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		bool validate_program(std::string& validate_status_description) const {
			glValidateProgram(program_id_);

			GLint validate_status;
			glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &validate_status);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			if (validate_status == GL_TRUE) {
				validate_status_description = "Validate status: success\n\n";
				return true;
			}

			validate_status_description = "Validate status: fail\nReason:\n" + load_program_info_log(program_id_) + "\n\n";
			return false;
		}

		void load_from_file(const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
			const std::string& vertex_shader_code = load_shader(vertex_shader_path);
			const std::string& fragment_shader_code = load_shader(fragment_shader_path);
			set_shader_code(vertex_shader_code, fragment_shader_code);
		}

		void swap(Shader& other) noexcept {
			std::swap(count_links_, other.count_links_);
			std::swap(program_id_, other.program_id_);
			std::swap(vertex_shader_code_, other.vertex_shader_code_);
			std::swap(fragment_shader_code_, other.fragment_shader_code_);
		}

		void clear() {
			if (count_links_ != nullptr) {
				--(*count_links_);
				if (*count_links_ == 0) {
					delete count_links_;
					delete vertex_shader_code_;
					delete fragment_shader_code_;
				}
			}
			count_links_ = nullptr;
			vertex_shader_code_ = nullptr;
			fragment_shader_code_ = nullptr;

			glDeleteProgram(program_id_);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			program_id_ = 0;
		}

		~Shader() {
			clear();
		}

		static GLint get_current_program() {
			GLint result = 0;
			glGetIntegerv(GL_CURRENT_PROGRAM, &result);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
			return result;
		}
	};
}
