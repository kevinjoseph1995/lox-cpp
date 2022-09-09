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
    [[nodiscard]] bool IsFromFile()
    {
        return !m_filename.empty();
    }
    [[nodiscard]] bool IsFromConsole()
    {
        return !IsFromFile();
    }
    void CLear()
    {
        m_filename.clear();
        m_source.clear();
    }

    std::string const& GetSource() const
    {
        return m_source;
    }

    [[nodiscard]] const char* GetFilename() { return m_filename.c_str(); }

private:
    std::string m_filename;
    std::string m_source;
};
#endif // LOX_CPP_SOURCE_H
