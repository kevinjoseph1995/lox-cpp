#include "virtual_machine.h"
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

static constexpr auto USAGE =
    R"(
usage: lox_cpp [file | - ]
)";

static int Run(VirtualMachine &vm, std::string const &source)
{
    auto result = vm.Interpret(&source);
    switch (result)
    {
    case InterpretResult::INTERPRET_OK:
        // TODO Figure out if we want to propagate error codes from Lox to here
        return 0;
    case InterpretResult::INTERPRET_COMPILE_ERROR:
    case InterpretResult::INTERPRET_RUNTIME_ERROR:
    default:
        return 1;
    }
}

static void RunInteractive()
{
    VirtualMachine vm;
    std::string line;
    std::string source;
    while (true)
    {
        source.clear();
        line.clear();
        if (isatty(STDIN_FILENO))
        {
            fmt::print("->> ");
        }

        std::getline(std::cin, line);
        if (line == "quit" || line == "q")
        {
            break;
        }
        if (line == "clear")
        {
            fmt::print("{}[2J{esc}[2J{esc}[1;1H", static_cast<char>(27), fmt::arg("esc", static_cast<char>(27)));
        }
        if (!line.empty() && line[line.length() - 1] == '\\')
        {
            do
            {
                source.append(line.begin(), line.end() - 1);
                std::getline(std::cin, line);
            } while (!line.empty() && line[line.length() - 1] == '\\');
        }

        source.append(line.begin(), line.end());
        Run(vm, source);
    }
}

static int RunFromFile(char const *const file_name)
{
    VirtualMachine vm;
    std::ifstream file;
    file.open(file_name);
    if (!file.good())
    {
        fmt::print(stderr, "Failed to read file:{}", file_name);
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return Run(vm, source);
}

int main(int argc, char **argv)
{
    switch (argc)
    {
    case 1:
        RunInteractive();
        return 0;
    case 2:
        return RunFromFile(argv[1]);
    default: {
        fmt::print("{}", USAGE);
        return 1;
    }
    }
}
