#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <random>
#include <type_traits>

namespace game_utils {

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
        static_assert(std::is_arithmetic<T>::value, "Only integral or floating point types are supported.");
    }
}

}


#endif // UTILS_H