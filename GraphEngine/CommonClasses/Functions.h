#pragma once

#include <functional>
#include <math.h>
#include <stdexcept>
#include <string>
#include <vector>


namespace eng {
    const double EPS = 1e-7;
    const double FI = (sqrt(5.0) - 1.0) / 2.0;
    const double PI = acos(-1.0);

    template <typename T>  // Operators required: <(T, T), ==(T, T)
    int32_t sgn(const T& x, const T& zero = T(0)) {
        if (x < zero) {
            return -1;
        }
        if (x == zero) {
            return 0;
        }
        return 1;
    }

    template <typename T>  // T - numeric type
    bool equality(const T& left, const T& right, const T& eps = T(EPS)) {
        return std::abs(left - right) < eps;
    }

    template <typename T>  // T - numeric type
    bool less_equality(const T& left, const T& right, const T& eps = T(EPS)) {
        return left < right || equality(left, right, eps);
    }

    std::vector<std::string> split(const std::string& str, std::function<bool(char)> pred) {
        std::vector<std::string> split_str(1);
        for (char character : str) {
            bool skip_character = pred(character);
            if (split_str.back().size() > 0 && skip_character) {
                split_str.push_back("");
            } else if (!skip_character) {
                split_str.back().push_back(character);
            }
        }
        return split_str;
    }

    template <typename T>  // Constructors required: T(T); Operators required: *(T, T), =(T, T)
    T binary_exponentiation(const T& base, uint32_t degree, const T& one = T(1)) {
        T result(one);
        for (size_t i = 8 * sizeof(uint32_t); i > 0; --i) {
            result = result * result;

            if (degree & (static_cast<uint32_t>(1) << (i - 1))) {
                result = result * base;
            }
        }
        return result;
    }

    template <typename T>  // Structures required: std::hash<T>
    void hash_combine(size_t& seed, const T& value) {
        seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T, typename It>  // Operators required: !=(It, It), ++(It), *(It); Structures required: std::iterator_traits<It>
    T get_value(It container_begin, It container_end, T value, std::function<void(const typename std::iterator_traits<It>::value_type&, T*)> func) {
        for (; container_begin != container_end; ++container_begin) {
            func(*container_begin, &value);
        }
        return value;
    }

    template <typename It>  // Operators required: !=(It, It), ++(It), *(It); Structures required: std::iterator_traits<It>
    void apply_func(It container_begin, It container_end, std::function<void(const typename std::iterator_traits<It>::value_type&)> func) {
        for (; container_begin != container_end; ++container_begin) {
            func(*container_begin);
        }
    }


    class EngDomainError : public std::domain_error {
    public:
        EngDomainError(const char* filename, uint32_t line, std::string message) : domain_error("Domain error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };

    class EngInvalidArgument : public std::invalid_argument {
    public:
        EngInvalidArgument(const char* filename, uint32_t line, std::string message) : invalid_argument("Invalid argument error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };

    class EngOutOfRange : public std::out_of_range {
    public:
        EngOutOfRange(const char* filename, uint32_t line, std::string message) : out_of_range("Out of range error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };

    class EngRuntimeError : public std::runtime_error {
    public:
        EngRuntimeError(const char* filename, uint32_t line, std::string message) : runtime_error("Runtime error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message) {
        }
    };
}
