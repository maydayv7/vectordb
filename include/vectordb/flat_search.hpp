#pragma once

#include "vectordb/distance.hpp"
#include "vectordb/vector_record.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace vectordb {

// Returns up to k record IDs, ordered by squared distance, then by ID
inline std::vector<int> flat_search(std::span<const float> query,
                                    std::span<const VectorRecord<float>> base,
                                    std::size_t k)
{
    if (k == 0) {
        return {};
    }

    std::priority_queue<std::pair<float, int>> nearest;
    for (const auto &record : base) {
        const float distance = squared_l2(query, record.vector);
        if (!std::isfinite(distance)) {
            throw std::invalid_argument("Search requires finite distances");
        }
        const std::pair<float, int> candidate{distance, record.id};

        if (nearest.size() < k) {
            nearest.push(candidate);
        } else if (candidate < nearest.top()) {
            nearest.pop();
            nearest.push(candidate);
        }
    }

    std::vector<int> ids;
    ids.reserve(nearest.size());
    while (!nearest.empty()) {
        ids.push_back(nearest.top().second);
        nearest.pop();
    }
    std::reverse(ids.begin(), ids.end());
    return ids;
}

} // namespace vectordb
