#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>


#ifdef _DEBUG
#define DEBUG_LVL2
#endif // _DEBUG

#ifdef DEBUG_LVL3
#define DEBUG_LVL2
#endif // DEBUG_LVL3

#ifdef DEBUG_LVL2
#define DEBUG_LVL1
#endif // DEBUG_LVL2


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
        GreDomainError(const char* filename, uint32_t line, const std::string& message);
    };

    // GRE exception for invalid argument error, inheritor of std::invalid_argument
    class GreInvalidArgument : public std::invalid_argument {
    public:
        GreInvalidArgument(const char* filename, uint32_t line, const std::string& message);
    };

    // GRE exception for out of range error, inheritor of std::out_of_range
    class GreOutOfRange : public std::out_of_range {
    public:
        GreOutOfRange(const char* filename, uint32_t line, const std::string& message);
    };

    // GRE exception for runtime error, inheritor of std::runtime_error
    class GreRuntimeError : public std::runtime_error {
    public:
        GreRuntimeError(const char* filename, uint32_t line, const std::string& message);
    };
}  // namespace gre


// GRE main log storage
namespace gre {
    // GRE main log storage
    class MainLog {
    public:
        struct LogEntry {
            std::string message;
            const char* filename;
            uint32_t line;
        };

        using const_iterator = std::vector<LogEntry>::const_iterator;

        static void push_entry(const std::string& message, const char* filename, uint32_t line);
        static void push_entry(const std::string& message);

        static const_iterator begin();
        static const_iterator end();

    private:
        inline static std::vector<LogEntry> log_entries_;
    };

    std::ostream& operator<<(std::ostream& fout, const MainLog::LogEntry& entry);
    std::ostream& operator<<(std::ostream& fout, const MainLog& main_log);
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
