//
// Created by kevin on 9/9/22.
//

#include "source.h"
#include "fmt/core.h"
#include <fstream>
auto Source::ReadFromFile(std::string_view filename) -> bool
{
    m_filename = filename;

    auto file = std::ifstream {};
    file.open(m_filename);
    if (!file.good()) {
        fmt::print(stderr, "Failed to read file:{}", m_filename);
        return false;
    }
    m_source = std::string((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    return true;
}
auto Source::Append(std::string_view source_part) -> void
{
    m_filename.clear();
    m_source.append(source_part);
}
