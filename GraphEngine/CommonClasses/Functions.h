#pragma once

#include <functional>
#include <string>
#include <vector>


namespace gre {
    const double EPS = 1e-7;
    const double PI = acos(-1.0);


    class GreDomainError : public std::domain_error {
    public:
        GreDomainError(const char* filename, uint32_t line, const std::string& message) noexcept :
            domain_error("Domain error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };

    class GreInvalidArgument : public std::invalid_argument {
    public:
        GreInvalidArgument(const char* filename, uint32_t line, const std::string& message) noexcept :
            invalid_argument("Invalid argument error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };

    class GreOutOfRange : public std::out_of_range {
    public:
        GreOutOfRange(const char* filename, uint32_t line, const std::string& message) noexcept :
            out_of_range("Out of range error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };

    class GreRuntimeError : public std::runtime_error {
    public:
        GreRuntimeError(const char* filename, uint32_t line, const std::string& message) noexcept :
            runtime_error("Runtime error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };


    bool equality(double left, double right) noexcept {
        return std::abs(left - right) <= EPS;
    }

    bool less_equality(double left, double right) noexcept {
        return left < right || equality(left, right);
    }

    std::vector<std::string> split(const std::string& str, std::function<bool(char)> pred) {
        std::vector<std::string> split_str(1);
        for (char character : str) {
            bool skip_character = pred(character);
            if (split_str.back().size() > 0 && skip_character) {
                split_str.push_back("");
            }
            else if (!skip_character) {
                split_str.back().push_back(character);
            }
        }
        if (split_str.back().empty()) {
            split_str.pop_back();
        }
        return split_str;
    }

    template <typename T, class HT = std::hash<T>>  // Structures required: std::hash<T>
    void hash_combine(size_t& seed, const T& value, const HT& hasher = HT()) {
        seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
