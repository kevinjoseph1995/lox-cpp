//
// Created by kevin on 9/9/22.
//

#ifndef LOX_CPP_SOURCE_H
#define LOX_CPP_SOURCE_H

#include <string>

class Source {
public:
    [[nodiscard]] bool ReadFromFile(std::string_view filename);
    void AppendFromConsole(std::string_view source_part);
    [[nodiscard]] bool IsFromFile() const
    {
        return !m_filename.empty();
    }
    [[nodiscard]] bool IsFromConsole() const
    {
        return !IsFromFile();
    }
    void Clear()
    {
        m_filename.clear();
        m_source.clear();
    }

    std::string const& GetSource() const
    {
        return m_source;
    }
    std::string const& GetFilename() const
    {
        return m_filename;
    }

    [[nodiscard]] const char* GetFilename() { return m_filename.c_str(); }

private:
    std::string m_filename;
    std::string m_source;
};
#endif // LOX_CPP_SOURCE_H
