#pragma once

#include <math.h>
#include <algorithm>
#include <vector>
#include <string>


namespace eng {
    const double PI = acos(-1);
    const double FI = (sqrt(5) - 1.0) / 2.0;
    const double EPS = 1e-7;


    template<typename T>
    int32_t sign(const T& x, T zero = T(0)) {
        if (x < zero) {
            return -1;
        }
        if (x == zero) {
            return 0;
        }
        return 1;
    }

    template<typename T>
    bool equality(const T& left, const T& right, T eps = T(EPS)) {
        return abs(left - right) < eps;
    }

    std::vector<std::string> split(const std::string& str, bool (*pred)(char)) {
        std::vector<std::string> split_str(1);
        for (const char c : str) {
            if (split_str.back().size() > 0 && pred(c)) {
                split_str.push_back("");
            } else if (!pred(c)) {
                split_str.back().push_back(c);
            }
        }
        return split_str;
    }
}
