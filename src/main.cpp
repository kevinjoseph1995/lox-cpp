#include "virtual_machine.h"

#include <iostream>
#include <unistd.h>

#include "fmt/core.h"

static constexpr auto USAGE =
    R"(
usage: lox_cpp [LOX_SOURCE_FILE?]
)";

static int Run(VirtualMachine& vm, Source& source)
{
    auto result = vm.Interpret(source);
    if (!result) {
        auto const& error = result.error();
        switch (error.type) {
        case ErrorType::ScanError:
            fmt::print(stderr, "ScanError: {}\n", error.error_message);
            return 1;
        case ErrorType::ParseError:
            fmt::print(stderr, "ParseError: {}\n", error.error_message);
            return 1;
        case ErrorType::RuntimeError:
            fmt::print(stderr, "RuntimeError: {}\n", error.error_message);
            return 1;
        default:
            fmt::print(stderr, "Unknown error: {}\n", error.error_message);
            return 1;
        }
    }
    return 0;
}

static int RunFromFile(std::string_view file_name)
{
    VirtualMachine vm;
    Source source;
    if (!source.ReadFromFile(file_name)) {
        return 1;
    }
    return Run(vm, source);
}

static int RunFromStandardInput()
{
    VirtualMachine vm;
    std::string line;
    Source source;
    while (true) {
        source.Clear();
        line.clear();
        if (isatty(STDIN_FILENO)) {
            fmt::print("->> ");
        }

        std::getline(std::cin, line);
        if (line == "quit" || line == "q") {
            break;
        }
        if (line == "clear") {
            source.Clear();
            fmt::print("{}[2J{esc}[2J{esc}[1;1H", static_cast<char>(27),
                fmt::arg("esc", static_cast<char>(27)));
            continue;
        }
        if (!line.empty() && line[line.length() - 1] == '\\') {
            do {
                source.AppendFromConsole(std::string_view(line.begin(), line.end() - 1));
                source.AppendFromConsole("\n");
                std::getline(std::cin, line);
            } while (!line.empty() && line[line.length() - 1] == '\\');
        }

        source.AppendFromConsole(std::string_view(line.begin(), line.end()));
        Run(vm, source);
    }
    return 0;
}

int main(int argc, char** argv)
{
    switch (argc) {
    case 1:
        return RunFromStandardInput();
    case 2:
        return RunFromFile(argv[1]);
    default: {
        fmt::print("{}", USAGE);
        return 1;
    }
    }
}