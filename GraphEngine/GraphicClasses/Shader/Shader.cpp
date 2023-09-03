#include "Shader.hpp"


// Shader
namespace gre {
    std::string Shader::load_shader(const std::string& shader_path) {
        std::ifstream shader_file(shader_path);
        GRE_ENSURE(!shader_file.fail(), GreRuntimeError, "the shader file does not exist");

        std::string shader_code;
        for (std::string line; std::getline(shader_file, line);) {
            shader_code += line + "\n";
        }
        return shader_code;
    }

    GLuint Shader::create_vertex_shader(const std::string& code) {
        const char* vertex_shader_code_c = code.c_str();
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_code_c, NULL);
        glCompileShader(vertex_shader);

        GLint success;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        GRE_ENSURE(success == GL_TRUE, GreRuntimeError, "compilation failed, description --/\n" << load_shader_info_log(vertex_shader));
        
        GRE_CHECK_GL_ERRORS;
        return vertex_shader;
    }

    GLuint Shader::create_fragment_shader(const std::string& code) {
        const char* fragment_shader_code_c = code.c_str();
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_code_c, NULL);
        glCompileShader(fragment_shader);

        GLint success;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        GRE_ENSURE(success == GL_TRUE, GreRuntimeError, "compilation failed, description --/\n" << load_shader_info_log(fragment_shader));
        
        GRE_CHECK_GL_ERRORS;
        return fragment_shader;
    }

    GLuint Shader::link_shaders(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
        GLuint vertex_shader = create_vertex_shader(vertex_shader_code);
        GLuint fragment_shader = create_fragment_shader(fragment_shader_code);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        GRE_ENSURE(success == GL_TRUE, GreRuntimeError, "linking failed, description --/\n" << load_program_info_log(program));

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        GRE_CHECK_GL_ERRORS;
        return program;
    }

    std::string Shader::find_value(const std::string& code, const std::string& variable_name) {
        std::vector<std::string> split_code = split(code, [](const char c) { return c == ' ' || c == '\n'; });
        GRE_ENSURE(split_code.size() >= 3, GreInvalidArgument, "variable not found");

        for (size_t i = 0; i < split_code.size() - 2; ++i) {
            if (split_code[i] == variable_name && split_code[i + 1] == "=") {
                split_code[i + 2].pop_back();
                return split_code[i + 2];
            }
        }
        GRE_ENSURE(false, GreInvalidArgument, "variable not found");
    }

    uint64_t Shader::find_version(const std::string& code) {
        std::vector<std::string> split_code = split(code, [](const char c) { return c == ' ' || c == '\n'; });
        GRE_ENSURE(split_code.size() >= 2, GreInvalidArgument, "version not found");

        for (size_t i = 0; i < split_code.size() - 1; ++i) {
            if (split_code[i] == "#version") {
                return std::stoull(split_code[i + 1]);
            }
        }
        GRE_ENSURE(false, GreInvalidArgument, "version not found");
    }

    std::string Shader::load_shader_info_log(GLuint shader) {
        GLint info_log_size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_size);

        GLchar* info_log = new GLchar[info_log_size];
        glGetShaderInfoLog(shader, info_log_size, NULL, info_log);

        GRE_CHECK_GL_ERRORS;

        std::string result(info_log);
        delete[] info_log;
        return result;
    }

    std::string Shader::load_program_info_log(GLuint program) {
        GLint info_log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_size);

        GLchar* info_log = new GLchar[info_log_size];
        glGetProgramInfoLog(program, info_log_size, NULL, info_log);

        GRE_CHECK_GL_ERRORS;

        std::string result(info_log);
        delete[] info_log;
        return result;
    }

    Shader::Shader() {
        GRE_ENSURE(glew_is_ok(), GreRuntimeError, "failed to initialize GLEW");
    }

    Shader::Shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
        GRE_ENSURE(glew_is_ok(), GreRuntimeError, "failed to initialize GLEW");

        set_shader_code(vertex_shader_code, fragment_shader_code);
    }

    Shader::Shader(const Shader& other) noexcept {
        vertex_shader_code_ = other.vertex_shader_code_;
        fragment_shader_code_ = other.fragment_shader_code_;
        count_links_ = other.count_links_;
        if (count_links_ != nullptr) {
            ++(*count_links_);
        }

        program_id_ = link_shaders(*vertex_shader_code_, *fragment_shader_code_);
    }

    Shader::Shader(Shader&& other) noexcept {
        swap(other);
    }

    Shader& Shader::operator=(const Shader& other)& noexcept {
        Shader object(other);
        swap(object);
        return *this;
    }

    Shader& Shader::operator=(Shader&& other)& noexcept {
        clear();
        swap(other);
        return *this;
    }

    void Shader::set_shader_code(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
        clear();

        count_links_ = new size_t(1);
        vertex_shader_code_ = new std::string(vertex_shader_code);
        fragment_shader_code_ = new std::string(fragment_shader_code);
        program_id_ = link_shaders(*vertex_shader_code_, *fragment_shader_code_);
    }

    void Shader::set_uniform_f(const GLchar* uniform_name, GLfloat v0) const {
        use();
        glUniform1f(get_uniform_location(uniform_name), v0);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1) const {
        use();
        glUniform2f(get_uniform_location(uniform_name), v0, v1);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_f(const GLchar* uniform_name, const Vec2& v) const {
        use();
        glUniform2f(get_uniform_location(uniform_name), static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y));
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2) const {
        use();
        glUniform3f(get_uniform_location(uniform_name), v0, v1, v2);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_f(const GLchar* uniform_name, const Vec3& v) const {
        use();
        glUniform3f(get_uniform_location(uniform_name), static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y), static_cast<GLfloat>(v.z));
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_f(const GLchar* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const {
        use();
        glUniform4f(get_uniform_location(uniform_name), v0, v1, v2, v3);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_i(const GLchar* uniform_name, GLint v0) const {
        use();
        glUniform1i(get_uniform_location(uniform_name), v0);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1) const {
        use();
        glUniform2i(get_uniform_location(uniform_name), v0, v1);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2) const {
        use();
        glUniform3i(get_uniform_location(uniform_name), v0, v1, v2);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_i(const GLchar* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3) const {
        use();
        glUniform4i(get_uniform_location(uniform_name), v0, v1, v2, v3);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_ui(const GLchar* uniform_name, GLuint v0) const {
        use();
        glUniform1ui(get_uniform_location(uniform_name), v0);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1) const {
        use();
        glUniform2ui(get_uniform_location(uniform_name), v0, v1);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2) const {
        use();
        glUniform3ui(get_uniform_location(uniform_name), v0, v1, v2);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_ui(const GLchar* uniform_name, GLuint v0, GLuint v1, GLuint v2, GLuint v3) const {
        use();
        glUniform4ui(get_uniform_location(uniform_name), v0, v1, v2, v3);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_1fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
        use();
        glUniform1fv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_2fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
        use();
        glUniform2fv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_3fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
        use();
        glUniform3fv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_4fv(const GLchar* uniform_name, GLsizei count, const GLfloat* value) const {
        use();
        glUniform4fv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_1iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
        use();
        glUniform1iv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_2iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
        use();
        glUniform2iv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_3iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
        use();
        glUniform3iv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_4iv(const GLchar* uniform_name, GLsizei count, const GLint* value) const {
        use();
        glUniform4iv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_1uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
        use();
        glUniform1uiv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_2uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
        use();
        glUniform2uiv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_3uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
        use();
        glUniform3uiv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_4uiv(const GLchar* uniform_name, GLsizei count, const GLuint* value) const {
        use();
        glUniform4uiv(get_uniform_location(uniform_name), count, value);
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_matrix(const GLchar* uniform_name, GLsizei count, const GLfloat* value, size_t height, size_t width, GLboolean transpose) const {
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
                GRE_ENSURE(false, GreInvalidArgument, "invalid matrix size");
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
                GRE_ENSURE(false, GreInvalidArgument, "invalid matrix size");
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
                GRE_ENSURE(false, GreInvalidArgument, "invalid matrix size");
                break;
            }
            break;
        default:
            GRE_ENSURE(false, GreInvalidArgument, "invalid matrix size");
            break;
        }
        GRE_CHECK_GL_ERRORS;
    }

    void Shader::set_uniform_matrix(const GLchar* uniform_name, const Matrix4x4& matrix, GLboolean transpose) const {
        use();
        glUniformMatrix4fv(get_uniform_location(uniform_name), 1, transpose, &std::vector<GLfloat>(matrix)[0]);
        GRE_CHECK_GL_ERRORS;
    }

    std::string Shader::get_value_vert(const std::string& variable_name) const {
        return find_value(*vertex_shader_code_, variable_name);
    }

    std::string Shader::get_value_frag(const std::string& variable_name) const {
        return find_value(*fragment_shader_code_, variable_name);
    }

    GLint Shader::get_uniform_location(const GLchar* uniform_name) const {
        GLint uniform_location = glGetUniformLocation(program_id_, uniform_name);
        GRE_CHECK_GL_ERRORS;
        return uniform_location;
    }

    GLuint Shader::get_program_id() const noexcept {
        return program_id_;
    }

    bool Shader::check_window_settings(const sf::ContextSettings& settings) const {
        uint64_t vert_version = find_version(*vertex_shader_code_);
        uint64_t frag_version = find_version(*fragment_shader_code_);
        if (vert_version / 100 > settings.majorVersion || (vert_version / 100 == settings.majorVersion && (vert_version % 100) / 10 > settings.minorVersion)) {
            return false;
        }
        return vert_version / 100 < settings.majorVersion || (vert_version / 100 == settings.majorVersion && (vert_version % 100) / 10 <= settings.minorVersion);
    }

    void Shader::use() const {
        glUseProgram(program_id_);
        GRE_CHECK_GL_ERRORS;
    }

    bool Shader::validate_program(std::string& validate_status_description) const {
        glValidateProgram(program_id_);

        GLint validate_status;
        glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &validate_status);

        GRE_CHECK_GL_ERRORS;

        if (validate_status == GL_TRUE) {
            validate_status_description = "Validate status: success\n\n";
            return true;
        }

        validate_status_description = "Validate status: fail\nReason:\n" + load_program_info_log(program_id_) + "\n\n";
        return false;
    }

    bool Shader::validate_program() const {
        glValidateProgram(program_id_);

        GLint validate_status;
        glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &validate_status);

        GRE_CHECK_GL_ERRORS;

        return validate_status == GL_TRUE;
    }

    void Shader::load_from_file(const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
        const std::string& vertex_shader_code = load_shader(vertex_shader_path);
        const std::string& fragment_shader_code = load_shader(fragment_shader_path);
        set_shader_code(vertex_shader_code, fragment_shader_code);
    }

    void Shader::swap(Shader& other) noexcept {
        std::swap(count_links_, other.count_links_);
        std::swap(program_id_, other.program_id_);
        std::swap(vertex_shader_code_, other.vertex_shader_code_);
        std::swap(fragment_shader_code_, other.fragment_shader_code_);
    }

    void Shader::clear() {
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
        GRE_CHECK_GL_ERRORS;

        program_id_ = 0;
    }

    Shader::~Shader() {
        clear();
    }

    GLint Shader::get_current_program() {
        GLint result = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &result);
        GRE_CHECK_GL_ERRORS;
        return result;
    }
}  // namespace gre
