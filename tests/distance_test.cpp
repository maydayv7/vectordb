#include "vectordb/distance.hpp"

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <stdexcept>
#include <vector>

TEST(SquaredL2Test, ComputesSquaredDistanceInEitherDirection)
{
    const std::vector<float> a = {1.0F, 2.0F, 3.0F};
    const std::vector<float> b = {4.0F, 6.0F, 3.0F};

    EXPECT_FLOAT_EQ(vectordb::squared_l2(a, b), 25.0F);
    EXPECT_FLOAT_EQ(vectordb::squared_l2(b, a), 25.0F);
}

TEST(SquaredL2Test, IdenticalVectorsHaveZeroDistance)
{
    const std::vector<float> coordinates = {-1.5F, 0.0F, 2.25F};

    EXPECT_FLOAT_EQ(vectordb::squared_l2(coordinates, coordinates), 0.0F);
}

TEST(SquaredL2Test, HandlesNegativeAndFractionalCoordinates)
{
    const std::vector<float> a = {-1.5F, 0.25F};
    const std::vector<float> b = {0.5F, 1.75F};

    EXPECT_FLOAT_EQ(vectordb::squared_l2(a, b), 6.25F);
}

TEST(SquaredL2Test, IncludesTheLastCoordinateIn128Dimensions)
{
    const std::vector<float> a(128, 0.0F);
    std::vector<float> b(128, 0.0F);
    b.back() = 3.0F;

    EXPECT_FLOAT_EQ(vectordb::squared_l2(a, b), 9.0F);
}

TEST(SquaredL2Test, EmptyVectorsHaveZeroDistance)
{
    EXPECT_FLOAT_EQ(vectordb::squared_l2({}, {}), 0.0F);
}

TEST(SquaredL2Test, RejectsDifferentDimensions)
{
    const std::vector<float> shorter = {1.0F};
    const std::vector<float> longer = {1.0F, 2.0F};

    EXPECT_THROW(vectordb::squared_l2(shorter, longer), std::invalid_argument);
    EXPECT_THROW(vectordb::squared_l2(longer, shorter), std::invalid_argument);
    EXPECT_THROW(vectordb::squared_l2({}, longer), std::invalid_argument);
    EXPECT_THROW(vectordb::squared_l2(longer, {}), std::invalid_argument);
}

TEST(SquaredL2Test, ReadsOnlyTheRequestedSpan)
{
    const std::array<float, 5> padded = {99.0F, 1.0F, 2.0F, 3.0F, 99.0F};
    const std::vector<float> b = {4.0F, 6.0F, 3.0F};
    const auto a = std::span(padded).subspan(1, 3);

    EXPECT_FLOAT_EQ(vectordb::squared_l2(a, b), 25.0F);
}
