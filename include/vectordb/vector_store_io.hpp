#pragma once

#include "vectordb/vector_record.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace vectordb {

class VectorStoreIO {
  public:
    // Each record is a little-endian int32 dimension
    // followed by float32 (.fvecs) or int32 (.ivecs) values
    template <typename T>
    static std::vector<VectorRecord<T>> read_vecs(const std::string &file_path)
    {
        static_assert(std::is_same_v<T, float> ||
                          std::is_same_v<T, std::int32_t>,
                      "Vecs files support only float and int32_t elements");

        static_assert(sizeof(float) == 4 && sizeof(std::int32_t) == 4 &&
                          std::numeric_limits<float>::is_iec559,
                      "Vecs loading requires 4-byte IEEE floats and integers");

        static_assert(std::endian::native == std::endian::little,
                      "Direct vecs loading requires a little-endian machine");

        // Start at the end to measure the file before trusting dimensions
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("Cannot open vecs file: " + file_path);
        }

        std::streamoff remaining_bytes = file.tellg();
        if (remaining_bytes < 0 || !file.seekg(0, std::ios::beg)) {
            throw std::runtime_error("Cannot measure vecs file: " + file_path);
        }

        std::vector<VectorRecord<T>> records;
        std::int32_t expected_dimension = 0;
        constexpr std::streamsize header_bytes = sizeof(std::int32_t);

        while (remaining_bytes > 0) {
            std::int32_t dimension = 0;
            if (remaining_bytes < header_bytes ||
                !file.read(reinterpret_cast<char *>(&dimension),
                           header_bytes)) {
                throw std::runtime_error("Incomplete vecs dimension: " +
                                         file_path);
            }
            remaining_bytes -= header_bytes;

            if (dimension <= 0) {
                throw std::runtime_error("Vecs dimension must be positive: " +
                                         file_path);
            }
            if (expected_dimension != 0 && dimension != expected_dimension) {
                throw std::runtime_error("Inconsistent vecs dimensions: " +
                                         file_path);
            }
            expected_dimension = dimension;

            // Check the payload before allocation
            const auto payload_bytes =
                static_cast<std::uint64_t>(dimension) * sizeof(T);

            if (payload_bytes > static_cast<std::uint64_t>(remaining_bytes)) {
                throw std::runtime_error("Incomplete vecs payload: " +
                                         file_path);
            }

            if (payload_bytes >
                static_cast<std::uint64_t>(
                    std::numeric_limits<std::streamsize>::max())) {
                throw std::runtime_error("Vecs payload is too large to read: " +
                                         file_path);
            }

            if (records.size() >
                static_cast<std::size_t>(std::numeric_limits<int>::max())) {
                throw std::runtime_error("Too many vecs records for int IDs: " +
                                         file_path);
            }

            // emplace_back adds an empty record and returns a reference to it
            const auto id = static_cast<int>(records.size());
            auto &record = records.emplace_back();
            record.id = id;
            record.vector.resize(static_cast<std::size_t>(dimension));
            if (!file.read(reinterpret_cast<char *>(record.vector.data()),
                           static_cast<std::streamsize>(payload_bytes))) {
                throw std::runtime_error("Cannot read vecs payload: " +
                                         file_path);
            }
            remaining_bytes -= static_cast<std::streamoff>(payload_bytes);
        }

        return records;
    }
};

} // namespace vectordb
