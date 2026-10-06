#include "vectordb/vector_record.hpp"

#include <any>
#include <cstdint>
#include <iostream>
#include <string>

int main()
{
    vectordb::VectorRecord<float> record;
    record.id = 0;
    record.vector = {1.0F, 2.0F, 3.0F};
    record.metadata.values["label"] = std::string{"example"};

    vectordb::VectorRecord<std::int32_t> neighbors;
    neighbors.id = 0;
    neighbors.vector = {42, 7, 19};

    std::cout << "Vector " << record.id << " has " << record.dimension()
              << " dimensions\n";
    std::cout << "Label: "
              << std::any_cast<std::string>(record.metadata.values.at("label"))
              << '\n';
    std::cout << "Query " << neighbors.id << " has " << neighbors.dimension()
              << " known neighbors\n";
}
