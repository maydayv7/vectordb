#include "vectordb/flat_search.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

TEST(FlatSearchTest, KeepsTheClosestIdsInDistanceOrder)
{
    const std::vector<float> query = {0.0F, 0.0F};
    const std::vector<vectordb::VectorRecord<float>> base = {
        {40, {3.0F, 4.0F}, {}}, {90, {1.0F, 0.0F}, {}}, {10, {0.0F, 2.0F}, {}},
        {30, {6.0F, 8.0F}, {}}, {70, {0.0F, 0.0F}, {}},
    };

    EXPECT_EQ(vectordb::flat_search(query, base, 2),
              (std::vector<int>{70, 90}));
}

TEST(FlatSearchTest, ReturnsTheSingleClosestRecord)
{
    const std::vector<float> query = {1.0F};
    const std::vector<vectordb::VectorRecord<float>> base = {
        {8, {5.0F}, {}}, {3, {0.0F}, {}}, {6, {3.0F}, {}}};

    EXPECT_EQ(vectordb::flat_search(query, base, 1), (std::vector<int>{3}));
}

TEST(FlatSearchTest, BreaksDistanceTiesByIdRegardlessOfInputOrder)
{
    const std::vector<float> query = {0.0F};
    std::vector<vectordb::VectorRecord<float>> base = {
        {8, {1.0F}, {}},
        {2, {-1.0F}, {}},
        {6, {1.0F}, {}},
        {4, {-1.0F}, {}},
    };

    EXPECT_EQ(vectordb::flat_search(query, base, 2), (std::vector<int>{2, 4}));
    std::reverse(base.begin(), base.end());
    EXPECT_EQ(vectordb::flat_search(query, base, 2), (std::vector<int>{2, 4}));
}

TEST(FlatSearchTest, ZeroKReturnsNoIds)
{
    const std::vector<float> query = {0.0F};
    const std::vector<vectordb::VectorRecord<float>> base = {{7, {1.0F}, {}}};

    EXPECT_TRUE(vectordb::flat_search(query, base, 0).empty());
}

TEST(FlatSearchTest, EmptyDatasetReturnsNoIds)
{
    const std::vector<float> query = {0.0F};

    EXPECT_TRUE(vectordb::flat_search(query, {}, 3).empty());
}

TEST(FlatSearchTest, KAtOrAboveDatasetSizeReturnsAllIdsInDistanceOrder)
{
    const std::vector<float> query = {0.0F};
    const std::vector<vectordb::VectorRecord<float>> base = {
        {42, {3.0F}, {}}, {7, {1.0F}, {}}, {99, {2.0F}, {}}};
    const std::vector<int> expected = {7, 99, 42};

    EXPECT_EQ(vectordb::flat_search(query, base, 3), expected);
    EXPECT_EQ(vectordb::flat_search(query, base, 8), expected);
    EXPECT_EQ(vectordb::flat_search(query, base,
                                    std::numeric_limits<std::size_t>::max()),
              expected);
}

TEST(FlatSearchTest, RejectsDimensionMismatchEvenWhenQueueIsFull)
{
    const std::vector<float> query = {0.0F, 0.0F};
    const std::vector<vectordb::VectorRecord<float>> base = {
        {1, {1.0F, 1.0F}, {}}, {2, {0.0F}, {}}};

    EXPECT_THROW(vectordb::flat_search(query, base, 1), std::invalid_argument);
}

TEST(FlatSearchTest, RejectsNonFiniteDistances)
{
    const std::vector<float> finite_query = {0.0F};
    const std::vector<vectordb::VectorRecord<float>> finite_base = {
        {7, {0.0F}, {}}};

    for (const float value : {std::numeric_limits<float>::quiet_NaN(),
                              std::numeric_limits<float>::infinity(),
                              -std::numeric_limits<float>::infinity(),
                              std::numeric_limits<float>::max()}) {
        const std::vector<float> query = {value};
        const std::vector<vectordb::VectorRecord<float>> base = {
            {7, {value}, {}}};

        EXPECT_THROW(vectordb::flat_search(finite_query, base, 1),
                     std::invalid_argument);
        EXPECT_THROW(vectordb::flat_search(query, finite_base, 1),
                     std::invalid_argument);
    }
}

TEST(FlatSearchTest, MatchesFullSortingForEveryKOnASmallGrid)
{
    const std::vector<float> query = {0.5F, -0.5F};
    std::vector<vectordb::VectorRecord<float>> base;
    int id = 100;
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            base.push_back(
                {id, {static_cast<float>(x), static_cast<float>(y)}, {}});
            id -= 2;
        }
    }

    // A sort is a simple reference for testing the queue
    std::vector<std::pair<float, int>> ranked;
    ranked.reserve(base.size());
    for (const auto &record : base) {
        ranked.emplace_back(vectordb::squared_l2(query, record.vector),
                            record.id);
    }
    std::sort(ranked.begin(), ranked.end());

    for (std::size_t k = 0; k <= base.size() + 1; ++k) {
        SCOPED_TRACE(k);
        std::vector<int> expected;
        for (std::size_t i = 0; i < std::min(k, ranked.size()); ++i) {
            expected.push_back(ranked[i].second);
        }

        EXPECT_EQ(vectordb::flat_search(query, base, k), expected);
    }
}
