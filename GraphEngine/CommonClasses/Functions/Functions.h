#pragma once

#include <fstream>
#include <functional>
#include <iostream>
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

        std::ofstream& log_stream();
    };

    // GRE log functions
#define GRE_LOG_ERROR(stream) (gre::LogManager::GetInstance()->log_stream() << "ERROR in file: " << __FILE__ << ", function: " << __func__ << ", line: " << __LINE__ << "\n" << stream << "\n\n");
#define GRE_ENSURE(condition, error_type, stream)                                   \
    do {                                                                            \
        if (!(condition)) {                                                         \
            GRE_LOG_ERROR(stream);                                                  \
            std::stringstream str_stream;                                           \
            str_stream << stream;                                                   \
            throw error_type(__FILE__, __func__, __LINE__, str_stream.str());       \
        }                                                                           \
    } while (false)                                                                 \

#ifdef GRE_WARNING_LOG_ENABLED

    #define GRE_LOG_WARNING(stream) (gre::LogManager::GetInstance()->log_stream() << "WARNING in file: " << __FILE__ << ", function: " << __func__ << ", line: " << __LINE__ << "\n" << stream << "\n\n");
    #define GRE_CHECK(condition, stream)                                                \
        do {                                                                            \
            if (!(condition)) {                                                         \
                GRE_LOG_WARNING(stream);                                                \
            }                                                                           \
        } while (false)                                                                 \

#else

    #define GRE_LOG_WARNING(stream)
    #define GRE_CHECK(condition, error_type, stream)

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
}  // namespace gre