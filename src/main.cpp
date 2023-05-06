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