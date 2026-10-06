#pragma once

#include "vectordb/metadata.hpp"

#include <cstddef>
#include <vector>

namespace vectordb {

template <typename T>
struct VectorRecord {
    int id = 0;
    std::vector<T> vector;
    Metadata metadata;

    std::size_t dimension() const {
        return vector.size();
    }
};

} // namespace vectordb
