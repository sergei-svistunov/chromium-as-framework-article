#include <iostream>

#include "base/command_line.h"

int main(int argc, char **argv) {
    base::CommandLine::Init(argc, argv);

    auto *cmd_line = base::CommandLine::ForCurrentProcess();

    for (const auto &sw : cmd_line->GetSwitches()) {
        std::cout << "Switch " << sw.first << ": " << sw.second << std::endl;
    }

    for (const auto &arg: cmd_line->GetArgs()) {
        std::cout << "Arg " << arg << std::endl;
    }

    return 0;
}
