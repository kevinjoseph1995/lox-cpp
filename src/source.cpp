//
// Created by kevin on 9/9/22.
//

#include "source.h"
#include "fmt/core.h"
#include <fstream>
bool Source::ReadFromFile(std::string_view filename)
{
    m_filename = filename;

    std::ifstream file;
    file.open(m_filename);
    if (!file.good()) {
        fmt::print(stderr, "Failed to read file:{}", m_filename);
        return false;
    }
    m_source = std::string((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    return true;
}
void Source::AppendFromConsole(std::string_view source_part)
{
    m_filename.clear();
    m_source.append(source_part);
}
