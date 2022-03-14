#pragma once

#include <types.h>

#include <chrono>
#include <iostream>

namespace omg {

constexpr real_t EARTH_RADIUS = 6'371'009;
// constexpr real_t EARTH_RADIUS = 6367500;

// C++20 would have a better way of doing this,
// but I still don't want to use the C-style macro
constexpr real_t PI = M_PI;

template<typename T>
constexpr T toRadians(T value) {
    return value * PI / 180.0;
}

template<typename T>
constexpr T toDegrees(T value) {
    return value * 180.0 / PI;
}

// only works as an approximation for distances,
// does not consider position on earth
constexpr real_t degreesToMeters(real_t value) {
    return toRadians(value) * EARTH_RADIUS;
}

// only works as an approximation for distances,
// does not consider position on earth
constexpr real_t metersToDegrees(real_t value) {
    return toDegrees(value) / EARTH_RADIUS;
}

inline real_t geoDistance(const vec2_t& p1, const vec2_t& p2) {
    // according to https://en.wikipedia.org/wiki/Geographical_distance
    const real_t d_lon = p2[0] - p1[0];
    const real_t d_lat = p2[1] - p1[1];
    const real_t mean_lat = (p1[1] + p2[1]) / 2;
    const real_t lon = std::cos(mean_lat) * d_lon;

    return EARTH_RADIUS * std::sqrt(d_lat * d_lat + lon * lon);
}


class ScopeTimer {
    using TimePoint = std::chrono::_V2::system_clock::time_point;
public:
    ScopeTimer() : start(std::chrono::high_resolution_clock::now()), msg("Duration") {}

    explicit ScopeTimer(const std::string& msg) : start(std::chrono::high_resolution_clock::now()), msg(msg) {
        std::cout << msg << "..." << std::endl;
    }

    ~ScopeTimer() {
        TimePoint end = std::chrono::high_resolution_clock::now();
        int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	    std::cout << msg << ": " << duration << " ms" << std::endl;
    }

private:
    const TimePoint start;
    const std::string msg;
};

template <typename Iter, typename IndexIter>
Iter removeByIndices(Iter first, Iter last, IndexIter ifirst, IndexIter ilast) {
    // by papagaga from https://codereview.stackexchange.com/questions/206686/removing-by-indices-several-elements-from-a-vector

    if (ifirst == ilast || first == last) {
        return last;
    }

    if (!std::is_sorted(ifirst, ilast)) {
        std::sort(ifirst, ilast);
    }

    Iter out = std::next(first, *ifirst);
    Iter in  = std::next(out);

    while (++ifirst != ilast) {

        if (*std::prev(ifirst) + 1 == *ifirst) {
            ++in;
            continue;
        }

        out = std::move(in, std::next(first, *ifirst), out);
        in  = std::next(first, *ifirst + 1);
    }

    return std::move(in, last, out);
}

template <typename T, typename IndexIter>
void eraseByIndices(std::vector<T>& values, IndexIter ifirst, IndexIter ilast) {
    values.erase(removeByIndices(values.begin(), values.end(), ifirst, ilast), values.end());
}

}
