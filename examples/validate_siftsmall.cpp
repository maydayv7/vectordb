#include "vectordb/vector_store_io.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

void check_float_vectors(
    const std::vector<vectordb::VectorRecord<float>> &records,
    std::size_t expected_count, const std::string &name)
{
    if (records.size() != expected_count) {
        throw std::runtime_error(
            name + ": expected " + std::to_string(expected_count) +
            " vectors, got " + std::to_string(records.size()));
    }
    for (const auto &record : records) {
        if (record.dimension() != 128) {
            throw std::runtime_error(name +
                                     ": expected 128 coordinates per vector");
        }
    }
    std::cout << name << ": " << records.size() << " vectors, 128 dimensions\n";
}

} // namespace

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: validate_siftsmall <dataset-directory>\n";
        return 1;
    }

    try {
        const std::filesystem::path directory(argv[1]);
        const auto base = vectordb::VectorStoreIO::read_vecs<float>(
            (directory / "siftsmall_base.fvecs").string());
        const auto queries = vectordb::VectorStoreIO::read_vecs<float>(
            (directory / "siftsmall_query.fvecs").string());
        const auto ground_truth =
            vectordb::VectorStoreIO::read_vecs<std::int32_t>(
                (directory / "siftsmall_groundtruth.ivecs").string());

        check_float_vectors(base, 10'000, "Base");
        check_float_vectors(queries, 100, "Queries");

        if (ground_truth.size() != queries.size()) {
            throw std::runtime_error("Expected one ground-truth row per query");
        }
        for (const auto &row : ground_truth) {
            if (row.dimension() != 100) {
                throw std::runtime_error("Expected 100 neighbor IDs per query");
            }
            std::unordered_set<std::int32_t> seen;
            for (const auto id : row.vector) {
                if (id < 0 || static_cast<std::size_t>(id) >= base.size()) {
                    throw std::runtime_error(
                        "Ground-truth ID is outside the base dataset");
                }
                if (!seen.insert(id).second) {
                    throw std::runtime_error(
                        "Repeated neighbor ID in a ground-truth row");
                }
            }
        }

        std::cout << "Ground truth: " << ground_truth.size()
                  << " rows, 100 distinct neighbor IDs per query\n"
                  << "All neighbor IDs are in [0, " << base.size() << ").\n"
                  << "SIFT-small validation passed.\n";
    } catch (const std::exception &error) {
        std::cerr << "SIFT-small validation failed: " << error.what() << '\n';
        return 1;
    }
}
