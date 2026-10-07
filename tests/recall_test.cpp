#include "vectordb/recall.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

TEST(RecallTest, PerfectRecallDoesNotDependOnOrder)
{
    const std::vector<int> retrieved = {19, 42, 7};
    const std::vector<std::int32_t> expected = {42, 7, 19};

    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 3), 1.0);
}

TEST(RecallTest, MeasuresPartialOverlap)
{
    const std::vector<int> retrieved = {7, 42, 88};
    const std::vector<std::int32_t> expected = {42, 7, 19};

    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 3), 2.0 / 3.0);
}

TEST(RecallTest, DisjointResultsHaveZeroRecall)
{
    const std::vector<int> retrieved = {88, 90, 91};
    const std::vector<std::int32_t> expected = {42, 7, 19};

    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 3), 0.0);
}

TEST(RecallTest, MissingResultsCountAsMisses)
{
    const std::vector<int> retrieved = {42};
    const std::vector<std::int32_t> expected = {42, 7, 19};

    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 3), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(vectordb::recall_at_k({}, expected, 3), 0.0);
}

TEST(RecallTest, UsesOnlyTheFirstKIdsInEachList)
{
    const std::vector<int> retrieved = {42, 19, 7, 88};
    const std::vector<std::int32_t> expected = {42, 7, 19, 88};

    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 2), 0.5);
}

TEST(RecallTest, RepeatedResultIdsCountOnlyOnce)
{
    const std::vector<int> retrieved = {42, 42, 42};
    const std::vector<std::int32_t> expected = {42, 7, 19};

    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 3), 1.0 / 3.0);
}

TEST(RecallTest, RejectsZeroK)
{
    EXPECT_THROW(vectordb::recall_at_k({}, {}, 0), std::invalid_argument);
}

TEST(RecallTest, RejectsInsufficientGroundTruth)
{
    const std::vector<int> retrieved = {42, 7, 19};
    const std::vector<std::int32_t> expected = {42, 7};

    EXPECT_THROW(vectordb::recall_at_k(retrieved, expected, 3),
                 std::invalid_argument);
}

TEST(RecallTest, RejectsRepeatedIdsWithinTheGroundTruthPrefix)
{
    const std::vector<int> retrieved = {42, 7, 19};
    const std::vector<std::int32_t> expected = {42, 7, 42};

    EXPECT_THROW(vectordb::recall_at_k(retrieved, expected, 3),
                 std::invalid_argument);
    EXPECT_DOUBLE_EQ(vectordb::recall_at_k(retrieved, expected, 2), 1.0);
}
