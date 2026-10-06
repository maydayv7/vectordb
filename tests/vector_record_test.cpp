#include "vectordb/vector_record.hpp"

#include <gtest/gtest.h>

#include <any>
#include <cstdint>
#include <string>

TEST(VectorRecordTest, DimensionTracksCoordinateChanges)
{
    vectordb::VectorRecord<float> record;
    EXPECT_EQ(record.dimension(), 0U);

    record.vector = {1.0F, 2.0F, 3.0F};
    EXPECT_EQ(record.dimension(), 3U);

    record.vector.pop_back();
    EXPECT_EQ(record.dimension(), 2U);
}

TEST(VectorRecordTest, GroundTruthCopiesOwnTheirDataAndMetadata)
{
    vectordb::VectorRecord<std::int32_t> original;
    original.id = 0;
    original.vector = {42, 7, 19};
    original.metadata.values["label"] = std::string{"original"};

    // Changing a copy must not change the original record
    auto copy = original;
    copy.id = 1;
    copy.vector.at(0) = 99;
    copy.metadata.values["label"] = std::string{"copy"};

    EXPECT_EQ(original.id, 0);
    EXPECT_EQ(original.vector.at(0), 42);
    EXPECT_EQ(std::any_cast<std::string>(original.metadata.values.at("label")),
              "original");
}
