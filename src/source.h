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

#ifndef LOX_CPP_SOURCE_H
#define LOX_CPP_SOURCE_H

#include "error.h"
#include <string>

class Source {
public:
    [[nodiscard]] auto ReadFromFile(std::string_view filename) -> bool;
    auto Append(std::string_view source_part) -> void;
    [[nodiscard]] auto IsFromFile() const -> bool
    {
        return !m_filename.empty();
    }
    auto Clear() -> void
    {
        m_filename.clear();
        m_source.clear();
    }

    auto GetSource() const -> std::string const&
    {
        return m_source;
    }
    auto GetFilename() const -> std::string const&
    {
        LOX_ASSERT(!m_filename.empty());
        return m_filename;
    }

    [[nodiscard]] auto GetFilename() -> char const*
    {
        LOX_ASSERT(!m_filename.empty());
        return m_filename.c_str();
    }

private:
    std::string m_filename;
    std::string m_source;
};
#endif // LOX_CPP_SOURCE_H
