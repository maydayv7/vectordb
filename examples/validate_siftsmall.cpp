#include "vectordb/flat_search.hpp"
#include "vectordb/recall.hpp"
#include "vectordb/vector_store_io.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
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

void check_recall(
    const std::vector<vectordb::VectorRecord<float>> &base,
    const std::vector<vectordb::VectorRecord<float>> &queries,
    const std::vector<vectordb::VectorRecord<std::int32_t>> &ground_truth,
    std::size_t k)
{
    double total_recall = 0.0;
    std::size_t perfect_queries = 0;
    for (std::size_t i = 0; i < queries.size(); ++i) {
        const auto ids = vectordb::flat_search(queries[i].vector, base, k);
        const double recall =
            vectordb::recall_at_k(ids, ground_truth[i].vector, k);
        total_recall += recall;
        if (recall == 1.0) {
            ++perfect_queries;
        } else {
            std::cerr << "Query " << i << ": recall@" << k << " = " << recall
                      << " (expected 1.0)\n";
        }
    }

    std::cout << std::fixed << std::setprecision(6) << "Recall@" << k << ": "
              << total_recall / static_cast<double>(queries.size()) << " ("
              << perfect_queries << '/' << queries.size()
              << " queries with perfect recall)\n";
    if (perfect_queries != queries.size()) {
        throw std::runtime_error("Recall@" + std::to_string(k) +
                                 " must be 1.0 for every query");
    }
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
                  << "All neighbor IDs are in [0, " << base.size() << ").\n";

        check_recall(base, queries, ground_truth, 10);
        check_recall(base, queries, ground_truth, 100);
        std::cout << "SIFT-small validation passed.\n";
    } catch (const std::exception &error) {
        std::cerr << "SIFT-small validation failed: " << error.what() << '\n';
        return 1;
    }
}
