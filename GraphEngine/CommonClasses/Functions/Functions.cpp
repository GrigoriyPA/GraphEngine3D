#include "Functions.h"


// Exceptions
namespace gre {
    GreDomainError::GreDomainError(const char* filename, uint32_t line, const std::string& message)
        : domain_error("Domain error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message)
    {}

    GreInvalidArgument::GreInvalidArgument(const char* filename, uint32_t line, const std::string& message) 
        : invalid_argument("Invalid argument error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message)
    {}

    GreOutOfRange::GreOutOfRange(const char* filename, uint32_t line, const std::string& message)
        : out_of_range("Out of range error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message)
    {}

    GreRuntimeError::GreRuntimeError(const char* filename, uint32_t line, const std::string& message)
        : runtime_error("Runtime error.\nFilename: " + std::string(filename) + "\nLine: " + std::to_string(line) + "\nDescription: " + message)
    {}
}  // namespace gre


// MainLog
namespace gre {
    // Changing
    void MainLog::push_entry(const std::string& message, const char* filename, uint32_t line) {
        log_entries_.push_back({ .message = message, .filename = filename, .line = line });
#ifdef DEBUG_LVL3
        std::cout << log_entries_.back();
#endif // DEBUG_LVL2
    }

    void MainLog::push_entry(const std::string& message) {
        push_entry(message, nullptr, 0);
    }


    // Iterating
    MainLog::const_iterator MainLog::begin() {
        return log_entries_.begin();
    }

    MainLog::const_iterator MainLog::end() {
        return log_entries_.begin();
    }

    
    // External operators
    std::ostream& operator<<(std::ostream& fout, const MainLog::LogEntry& entry) {
        if (entry.filename != nullptr) {
            fout << "[GRE_LOG] Error in file " << entry.filename << " line " << entry.line << ", description:\n" << entry.message;
        }
        else {
            fout << "[GRE_LOG] Error with description: \n" << entry.message;
        }
        return fout;
    }

    std::ostream& operator<<(std::ostream& fout, const MainLog& main_log) {
        for (const MainLog::LogEntry& entry : main_log) {
            fout << entry;
        }
        return fout;
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
}  // namespace gre
