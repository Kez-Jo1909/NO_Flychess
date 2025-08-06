#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <random>
#include <type_traits>

namespace game_utils {

enum class Color {
    UNDEFINED = -1,
    RED = 0,
    BLUE = 1,
    GREEN = 2,
    YELLOW = 3
};

inline std::string colorToString(Color c) {
    switch (c) {
        case Color::RED:      return "RED";
        case Color::BLUE:     return "BLUE";
        case Color::GREEN:    return "GREEN";
        case Color::YELLOW:   return "YELLOW";
        default:              return "UNDEFINED";
    }
}

inline std::string colorIntToString(int c) {
    switch(c) {
        case 0: return "RED";
        case 1: return "BLUE";
        case 2: return "GREEN";
        case 3: return "YELLOW";
        default: return "UNDEFINED";
    }
}

/**
 * @name get_random
 * @brief Generates a random number within the specified range.
 */
template <typename T>
T get_random(T min, T max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(gen);
    } else if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(gen);
    } else {
        static_assert(std::is_arithmetic<T>::value, "不支持的类型");
    }
}

}


#endif // UTILS_H