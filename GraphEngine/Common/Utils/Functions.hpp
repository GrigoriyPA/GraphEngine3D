#pragma once

#include <GL/glew.h>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>


// GRE global constants
namespace gre {
#ifdef GRE_EPSILON
    constexpr double EPS = GRE_EPSILON;
#else // GRE_EPSILON
    constexpr double EPS = 1e-12;
#endif // !GRE_EPSILON
    constexpr double PI = 3.141592653589793;
}  // namespace gre


// GRE exceptions
namespace gre {
    // GRE exception for domain error, inheritor of std::domain_error
    class GreDomainError : public std::domain_error {
    public:
        GreDomainError(const char* filename, const char* function, uint32_t line, const std::string& message);
    };

    // GRE exception for invalid argument error, inheritor of std::invalid_argument
    class GreInvalidArgument : public std::invalid_argument {
    public:
        GreInvalidArgument(const char* filename, const char* function, uint32_t line, const std::string& message);
    };

    // GRE exception for out of range error, inheritor of std::out_of_range
    class GreOutOfRange : public std::out_of_range {
    public:
        GreOutOfRange(const char* filename, const char* function, uint32_t line, const std::string& message);
    };

    // GRE exception for runtime error, inheritor of std::runtime_error
    class GreRuntimeError : public std::runtime_error {
    public:
        GreRuntimeError(const char* filename, const char* function, uint32_t line, const std::string& message);
    };
}  // namespace gre


// GRE main log storage
namespace gre {
    // GRE main log manager
    class LogManager {
    private:
        inline static LogManager* log_manager_ = nullptr;

        std::ofstream log_out_;

        LogManager();

    public:
        LogManager(const LogManager& other) = delete;

        void operator=(const LogManager&) = delete;

        static LogManager* GetInstance();

        std::ofstream& log_stream() noexcept;
    };

#define GRE_WARNING_LOG_ENABLED

    // GRE log functions
#define GRE_LOG_MESSAGE(stream, type)                                                                       \
    do {                                                                                                    \
        auto log_stream(std::move(gre::LogManager::GetInstance()->log_stream()));                           \
        log_stream << type << " in file: " << __FILE__;                                                     \
        log_stream << ", function: " << __func__;                                                           \
        log_stream << ", line: " << __LINE__;                                                               \
        log_stream << "\n" << stream << "\n\n";                                                             \
        log_stream.close();                                                                                 \
    } while (false)  

#define GRE_LOG_ERROR(stream) GRE_LOG_MESSAGE(stream, "ERROR")

#define GRE_ENSURE(condition, error_type, stream)                                                           \
    do {                                                                                                    \
        if (!(condition)) {                                                                                 \
            GRE_LOG_ERROR(stream);                                                                          \
            std::stringstream str_stream;                                                                   \
            str_stream << stream;                                                                           \
            throw error_type(__FILE__, __func__, __LINE__, str_stream.str());                               \
        }                                                                                                   \
    } while (false)


#ifdef GRE_WARNING_LOG_ENABLED

    #define GRE_LOG_WARNING(stream) GRE_LOG_MESSAGE(stream, "WARNING")
    
    #define GRE_CHECK(condition, stream)                                                                    \
        do {                                                                                                \
            if (!(condition)) {                                                                             \
                GRE_LOG_WARNING(stream);                                                                    \
            }                                                                                               \
        } while (false)

    #define GRE_CHECK_GL_ERRORS                                                                             \
        do {                                                                                                \
            GLenum error_code = glGetError();                                                               \
            if (error_code == GL_NO_ERROR) {                                                                \
                break;                                                                                      \
            }                                                                                               \
                                                                                                            \
            std::string error;                                                                              \
            switch (error_code) {                                                                           \
                case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;                       \
                case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;                      \
                case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;                  \
                case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;                     \
                case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;                    \
                case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;                      \
                case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;      \
                default:                               error = "UNKNOWN"; break;                            \
            }                                                                                               \
                                                                                                            \
            GRE_LOG_WARNING("GL error with name \"" << error << "\"");                                      \
        } while (false)

#else

    #define GRE_LOG_WARNING(stream)
    #define GRE_CHECK(condition, stream)
    #define GRE_CHECK_GL_ERRORS

#endif
}  // namespace gre


// Common functions
namespace gre {
    // GRE equality checking up to the EPS value
    bool equality(double left, double right) noexcept;

    // GRE less or equality checking up to the EPS value
    bool less_equality(double left, double right) noexcept;

    // GRE split string by given predicate
    std::vector<std::string> split(const std::string& str, std::function<bool(char)> pred);

    // GRE composing hash and hasher(value)
    template <typename T, class Hasher = std::hash<T>>
    void hash_combine(size_t& hash, const T& value, const Hasher& hasher = Hasher()) {
        hash ^= hasher(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    // GLEW initialization
    bool glew_is_ok() noexcept;
}  // namespace gre
