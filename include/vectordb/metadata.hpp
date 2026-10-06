#pragma once

#include <any>
#include <string>
#include <unordered_map>

namespace vectordb {

struct Metadata {
    std::unordered_map<std::string, std::any> values;
};

} // namespace vectordb
