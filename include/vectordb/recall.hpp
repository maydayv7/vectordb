#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <unordered_set>

namespace vectordb {

// Compare the first k IDs in each list
inline double recall_at_k(std::span<const int> retrieved,
                          std::span<const std::int32_t> ground_truth,
                          std::size_t k)
{
    if (k == 0 || k > ground_truth.size()) {
        throw std::invalid_argument(
            "Recall requires k ground-truth IDs and k > 0");
    }

    const auto expected = ground_truth.first(k);
    std::unordered_set<std::int32_t> remaining(expected.begin(),
                                               expected.end());
    if (remaining.size() != k) {
        throw std::invalid_argument(
            "Recall requires distinct ground-truth IDs");
    }

    std::size_t matches = 0;
    for (const int id : retrieved.first(std::min(k, retrieved.size()))) {
        matches += remaining.erase(id);
    }
    return static_cast<double>(matches) / static_cast<double>(k);
}

} // namespace vectordb
