#pragma once

#include <cmath>
#include <numeric>

inline double clamp(double value, double min, double max) {
    if (std::isinf(value)) {
        return 0.;
    }
    if (std::isnan(value)) {
        return 0.;
    }
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

inline double norm(int value, double sensitivity) {
    auto f = static_cast<double>(value) / 255.;
    f += sensitivity - 1.;
    if (f < 0.) {
        f = 0.;
    }
    f /= sensitivity;
    return clamp(f, 0., 1.);
}

template <typename I>
inline float avg(I begin, I end) {
    return std::accumulate(begin, end, 0.) / (end - begin);
}

template <typename I>
inline float rms(I begin, I end) {
    return std::sqrt((std::inner_product(begin, end, begin, 0.)) / (end - begin));
}
