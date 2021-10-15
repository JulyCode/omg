#pragma once

namespace omg {
namespace analysis {

template<typename T>
struct Aggregates {
    T min, max, avg;

    Aggregates() : min(0), max(0), avg(0) {}

    template<typename Iter>
    Aggregates(Iter begin, Iter end) : Aggregates() {

        const std::size_t count = std::distance(begin, end);
        if (count == 0) {
            throw std::runtime_error("cannot aggregate empty data");
        }

        min = max = static_cast<T>(*begin);

        for (; begin != end; ++begin) {
            const T v = static_cast<T>(*begin);

            min = std::min(min, v);
            max = std::max(max, v);
            avg += v;
        }
        avg /= count;
    }
};

}
}
