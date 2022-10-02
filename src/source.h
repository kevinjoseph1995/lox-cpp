//
// Created by kevin on 9/9/22.
//

#ifndef LOX_CPP_SOURCE_H
#define LOX_CPP_SOURCE_H

#include <string>

class Source {
public:
    [[nodiscard]] auto ReadFromFile(std::string_view filename) -> bool;
    auto AppendFromConsole(std::string_view source_part) -> void;
    [[nodiscard]] auto IsFromFile() const -> bool
    {
        return !m_filename.empty();
    }
    [[nodiscard]] bool IsFromConsole() const
    {
        return !IsFromFile();
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
        return m_filename;
    }

    [[nodiscard]] auto GetFilename() -> const char* { return m_filename.c_str(); }

private:
    std::string m_filename;
    std::string m_source;
};
#endif // LOX_CPP_SOURCE_H
