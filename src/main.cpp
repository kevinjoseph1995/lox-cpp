#include "virtual_machine.h"
#include <iostream>

static constexpr auto USAGE =
    R"(
usage: lox_cpp [file | - ]
)";

static void run(std::string const &source)
{
}

static void run_interactive()
{
}

static void run_from_file(char const *const file_name)
{
}

int main(int argc, char **argv)
{
    switch (argc)
    {
    case 1:
        run_interactive();
        break;
    case 2:
        run_from_file(argv[1]);
        break;
    default: {
        std::cout << USAGE << std::endl;
        return 1;
    }
    }
}
