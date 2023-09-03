#include "Functions.hpp"


namespace {
    bool GLEW_IS_OK = false;
}  // anonymous namespace


// Exceptions
namespace gre {
    GreDomainError::GreDomainError(const char* filename, const char* function, uint32_t line, const std::string & message)
        : domain_error("Domain error.\nFilename: " + std::string(filename) + "\nFunction: " + std::string(function) + "\nLine: " + std::to_string(line) + "\nDescription: " + message + "\n\n")
    {}

    GreInvalidArgument::GreInvalidArgument(const char* filename, const char* function, uint32_t line, const std::string & message)
        : invalid_argument("Invalid argument error.\nFilename: " + std::string(filename) + "\nFunction: " + std::string(function) + "\nLine: " + std::to_string(line) + "\nDescription: " + message + "\n\n")
    {}

    GreOutOfRange::GreOutOfRange(const char* filename, const char* function, uint32_t line, const std::string & message)
        : out_of_range("Out of range error.\nFilename: " + std::string(filename) + "\nFunction: " + std::string(function) + "\nLine: " + std::to_string(line) + "\nDescription: " + message + "\n\n")
    {}

    GreRuntimeError::GreRuntimeError(const char* filename, const char* function, uint32_t line, const std::string & message)
        : runtime_error("Runtime error.\nFilename: " + std::string(filename) + "\nFunction: " + std::string(function) + "\nLine: " + std::to_string(line) + "\nDescription: " + message + "\n\n")
    {}
}  // namespace gre


// MainLog
namespace gre {
    LogManager::LogManager() {
    }

    LogManager* LogManager::GetInstance() {
        if (log_manager_ == nullptr) {
            log_manager_ = new LogManager();
        }
        return log_manager_;
    }

    std::ofstream& LogManager::log_stream() noexcept {
        log_out_.open("gre_log.txt");
        return log_out_;
    }
}  // namespace gre


// Common functions
namespace gre {
    // Comparing
    bool equality(double left, double right) noexcept {
        return std::abs(left - right) <= EPS;
    }

    bool less_equality(double left, double right) noexcept {
        return left < right || equality(left, right);
    }

    // Work with strings
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

    // GLEW initialization
    bool glew_is_ok() noexcept {
        glewExperimental = GL_TRUE;
        return GLEW_IS_OK = GLEW_IS_OK ? true : glewInit() == GLEW_OK;
    }
}  // namespace gre
