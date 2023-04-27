#include "virtual_machine.h"

#include <iostream>
#include <unistd.h>

#include <fmt/core.h>

static constexpr auto USAGE =
    R"(
usage: lox_cpp [LOX_SOURCE_FILE]
)";

static int Run(VirtualMachine& vm, Source& source)
{
    auto result = vm.Interpret(source);
    if (!result) {
        auto const& error = result.error();
        fmt::print(stderr, "{}\n", error.error_message);
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

int main(int argc, char** argv)
{
    switch (argc) {
    case 2:
        return RunFromFile(argv[1]);
    default: {
        fmt::print("{}", USAGE);
        return 1;
    }
    }
}