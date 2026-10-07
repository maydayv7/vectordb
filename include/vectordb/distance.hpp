#pragma once

#include <cstddef>
#include <span>
#include <stdexcept>

namespace vectordb {

inline float squared_l2(std::span<const float> a, std::span<const float> b)
{
    if (a.size() != b.size()) {
        throw std::invalid_argument("Squared L2 requires equal dimensions");
    }

    float sum = 0.0F;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const float difference = a[i] - b[i];
        sum += difference * difference;
    }

    return sum;
}

} // namespace vectordb
