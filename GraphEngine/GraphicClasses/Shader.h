#pragma once

#include <fstream>
#include "GraphicFunctions.h"
#include "../CommonClasses/Matrix.h"


namespace gre {
	template <typename T = void*>  // Constructors required: T(), T(T); Operators required: =(T, T)
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
		T description = T();

		Shader() {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Shader, failed to initialize GLEW.\n\n");
			}
		}

		explicit Shader(const T& desc_value) {
			description = desc_value;
		}

		Shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code, const T& desc_value = T()) {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Shader, failed to initialize GLEW.\n\n");
			}

			description = desc_value;
			set_shader_code(vertex_shader_code, fragment_shader_code);
		}

		Shader(const Shader<T>& other) noexcept {
			description = other.description;
			vertex_shader_code_ = other.vertex_shader_code_;
			fragment_shader_code_ = other.fragment_shader_code_;
			count_links_ = other.count_links_;
			if (count_links_ != nullptr) {
				++(*count_links_);
			}

			program_id_ = link_shaders(*vertex_shader_code_, *fragment_shader_code_);
		}

		Shader(Shader<T>&& other) noexcept {
			swap(other);
		}

		Shader<T>& operator=(const Shader<T>& other)& noexcept {
			Shader<T> object(other);
			swap(object);
			return *this;
		}

		Shader<T>& operator=(Shader<T>&& other)& noexcept {
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
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform1f(get_uniform_location(uniform_name), v0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform2f(get_uniform_location(uniform_name), v0, v1);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_f(const GLchar* uniform_name, const Vec2& v) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform2f(get_uniform_location(uniform_name), static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y));
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform3f(get_uniform_location(uniform_name), v0, v1, v2);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_f(const GLchar* uniform_name, const Vec3& v) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform3f(get_uniform_location(uniform_name), static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y), static_cast<GLfloat>(v.z));
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform4f(get_uniform_location(uniform_name), v0, v1, v2, v3);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform1i(get_uniform_location(uniform_name), v0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform2i(get_uniform_location(uniform_name), v0, v1);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform3i(get_uniform_location(uniform_name), v0, v1, v2);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform4i(get_uniform_location(uniform_name), v0, v1, v2, v3);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform1ui(get_uniform_location(uniform_name), v0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform2ui(get_uniform_location(uniform_name), v0, v1);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform3ui(get_uniform_location(uniform_name), v0, v1, v2);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2, GLuint v3) const {
			if (get_current_program() != program_id_) {
				use();
			}
			glUniform4ui(get_uniform_location(uniform_name), v0, v1, v2, v3);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		template <size_t N>
		void set_uniform_fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
			if (get_current_program() != program_id_) {
				use();
			}
			switch (N) {
			case 1:
				glUniform1fv(get_uniform_location(uniform_name), count, value);
				break;
			case 2:
				glUniform2fv(get_uniform_location(uniform_name), count, value);
				break;
			case 3:
				glUniform3fv(get_uniform_location(uniform_name), count, value);
				break;
			case 4:
				glUniform4fv(get_uniform_location(uniform_name), count, value);
				break;
			default:
				throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_fv, invalid vector size.\n\n");
				break;
			}
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		template <size_t N>
		void set_uniform_iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
			if (get_current_program() != program_id_) {
				use();
			}
			switch (N) {
			case 1:
				glUniform1iv(get_uniform_location(uniform_name), count, value);
				break;
			case 2:
				glUniform2iv(get_uniform_location(uniform_name), count, value);
				break;
			case 3:
				glUniform3iv(get_uniform_location(uniform_name), count, value);
				break;
			case 4:
				glUniform4iv(get_uniform_location(uniform_name), count, value);
				break;
			default:
				throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_iv, invalid vector size.\n\n");
				break;
			}
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		template <size_t N>
		void set_uniform_uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
			if (get_current_program() != program_id_) {
				use();
			}
			switch (N) {
			case 1:
				glUniform1uiv(get_uniform_location(uniform_name), count, value);
				break;
			case 2:
				glUniform2uiv(get_uniform_location(uniform_name), count, value);
				break;
			case 3:
				glUniform3uiv(get_uniform_location(uniform_name), count, value);
				break;
			case 4:
				glUniform4uiv(get_uniform_location(uniform_name), count, value);
				break;
			default:
				throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_uiv, invalid vector size.\n\n");
				break;
			}
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_uniform_matrix(const GLchar* uniform_name, const Matrix4x4& matrix, GLboolean transpose = GL_FALSE) const {
			if (get_current_program() != program_id_) {
				use();
			}
			set_uniform_matrix(uniform_name, 1, &std::vector<GLfloat>(matrix)[0], 4, 4, transpose);
		}

		void set_uniform_matrix(const GLchar* uniform_name, GLsizei count, const GLfloat* value, size_t height, size_t width, GLboolean transpose = GL_FALSE) const {
			if (get_current_program() != program_id_) {
				use();
			}
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
					throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
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
					throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
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
					throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
					break;
				}
				break;
			default:
				throw GreOutOfRange(__FILE__, __LINE__, "set_uniform_matrix, invalid matrix size.\n\n");
				break;
			}
			check_gl_errors(__FILE__, __LINE__, __func__);
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

		void swap(Shader<T>& other) noexcept {
			std::swap(description, other.description);
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
