// MIT License

// Copyright (c) 2023 Kevin Joseph

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "source.h"

#include <fmt/core.h>
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
