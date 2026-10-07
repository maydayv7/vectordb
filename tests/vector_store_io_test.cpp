#include "vectordb/vector_store_io.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

constexpr std::array<unsigned char, 24> kFloatVecs{
    0x02, 0x00, 0x00, 0x00, // Dimension: 2 (1st Vector)
    0x00, 0x00, 0x80, 0x3F, // 1.0
    0x00, 0x00, 0x20, 0xC0, // -2.5
    0x02, 0x00, 0x00, 0x00, // Dimension: 2 (2nd Vector)
    0x00, 0x00, 0x50, 0x40, // 3.25
    0x00, 0x00, 0x00, 0x00, // 0.0
};

constexpr std::array<unsigned char, 32> kIntegerVecs{
    0x03, 0x00, 0x00, 0x00, // Neighbor count: 3
    0x2A, 0x00, 0x00, 0x00, // 42
    0x07, 0x00, 0x00, 0x00, // 7
    0x13, 0x00, 0x00, 0x00, // 19
    0x03, 0x00, 0x00, 0x00, // Neighbor count: 3
    0x01, 0x00, 0x00, 0x01, // 16777217 (not representable as float)
    0x00, 0x00, 0x00, 0x00, // 0
    0xFF, 0xFF, 0xFF, 0x7F, // 2147483647
};

class VectorStoreIOTest : public testing::Test {
  protected:
    void SetUp() override
    {
        auto pattern =
            (std::filesystem::temp_directory_path() / "vectordb-vecs-XXXXXX")
                .string();
        const auto *directory = ::mkdtemp(pattern.data());
        ASSERT_NE(directory, nullptr);
        directory_ = directory;
        file_path_ = directory_ / "test.vecs";
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(directory_, error);
        EXPECT_FALSE(error) << error.message();
    }

    void write_bytes(std::span<const unsigned char> bytes)
    {
        std::ofstream file(file_path_, std::ios::binary);
        ASSERT_TRUE(file.is_open());
        if (!bytes.empty()) {
            file.write(reinterpret_cast<const char *>(bytes.data()),
                       static_cast<std::streamsize>(bytes.size()));
        }
        file.close();
        ASSERT_TRUE(file);
    }

    std::filesystem::path directory_;
    std::filesystem::path file_path_;
};

TEST_F(VectorStoreIOTest, LoadsFloatCoordinatesAndAssignsSequentialIds)
{
    write_bytes(kFloatVecs);

    const auto records =
        vectordb::VectorStoreIO::read_vecs<float>(file_path_.string());

    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].id, 0);
    EXPECT_EQ(records[1].id, 1);
    ASSERT_EQ(records[0].dimension(), 2U);
    ASSERT_EQ(records[1].dimension(), 2U);
    EXPECT_FLOAT_EQ(records[0].vector[0], 1.0F);
    EXPECT_FLOAT_EQ(records[0].vector[1], -2.5F);
    EXPECT_FLOAT_EQ(records[1].vector[0], 3.25F);
    EXPECT_FLOAT_EQ(records[1].vector[1], 0.0F);
    EXPECT_TRUE(records[0].metadata.values.empty());
    EXPECT_TRUE(records[1].metadata.values.empty());
}

TEST_F(VectorStoreIOTest, LoadsNeighborIdsWithoutConvertingThroughFloat)
{
    write_bytes(kIntegerVecs);

    const auto records =
        vectordb::VectorStoreIO::read_vecs<std::int32_t>(file_path_.string());

    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].id, 0);
    EXPECT_EQ(records[1].id, 1);
    EXPECT_EQ(records[0].vector, (std::vector<std::int32_t>{42, 7, 19}));
    EXPECT_EQ(records[1].vector,
              (std::vector<std::int32_t>{16777217, 0, 2147483647}));
}

TEST_F(VectorStoreIOTest, EmptyFileReturnsNoRecords)
{
    write_bytes({});
    EXPECT_TRUE(
        vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()).empty());
}

TEST_F(VectorStoreIOTest, RejectsMissingFile)
{
    EXPECT_THROW(vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
                 std::runtime_error);
}

TEST_F(VectorStoreIOTest, RejectsPartialFirstDimension)
{
    for (std::size_t size = 1; size < 4; ++size) {
        SCOPED_TRACE(size);
        write_bytes(std::span(kFloatVecs).first(size));
        EXPECT_THROW(
            vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
            std::runtime_error);
    }
}

TEST_F(VectorStoreIOTest, RejectsTruncatedFirstPayload)
{
    for (std::size_t size = 4; size < 12; ++size) {
        SCOPED_TRACE(size);
        write_bytes(std::span(kFloatVecs).first(size));
        EXPECT_THROW(
            vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
            std::runtime_error);
    }
}

TEST_F(VectorStoreIOTest, RejectsZeroDimension)
{
    constexpr std::array<unsigned char, 4> bytes{0x00, 0x00, 0x00, 0x00};
    write_bytes(bytes);
    EXPECT_THROW(vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
                 std::runtime_error);
}

TEST_F(VectorStoreIOTest, RejectsNegativeDimension)
{
    constexpr std::array<unsigned char, 4> bytes{0xFF, 0xFF, 0xFF, 0xFF};
    write_bytes(bytes);
    EXPECT_THROW(vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
                 std::runtime_error);
}

TEST_F(VectorStoreIOTest, RejectsHugeDimensionWithoutAllocatingItsPayload)
{
    constexpr std::array<unsigned char, 4> bytes{0xFF, 0xFF, 0xFF, 0x7F};
    write_bytes(bytes);
    EXPECT_THROW(vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
                 std::runtime_error);
}

TEST_F(VectorStoreIOTest, RejectsDifferentDimensionsBetweenCompleteRecords)
{
    auto bytes = kFloatVecs;
    bytes[12] =
        1; // The second record now has one value, with a complete payload
    write_bytes(std::span(bytes).first(20));
    EXPECT_THROW(vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
                 std::runtime_error);
}

TEST_F(VectorStoreIOTest, RejectsPartialDimensionAfterACompleteRecord)
{
    for (std::size_t size = 13; size < 16; ++size) {
        SCOPED_TRACE(size);
        write_bytes(std::span(kFloatVecs).first(size));
        EXPECT_THROW(
            vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
            std::runtime_error);
    }
}

TEST_F(VectorStoreIOTest, RejectsTruncatedPayloadAfterACompleteRecord)
{
    for (std::size_t size = 16; size < kFloatVecs.size(); ++size) {
        SCOPED_TRACE(size);
        write_bytes(std::span(kFloatVecs).first(size));
        EXPECT_THROW(
            vectordb::VectorStoreIO::read_vecs<float>(file_path_.string()),
            std::runtime_error);
    }
}

} // namespace
