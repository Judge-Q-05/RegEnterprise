#include "CLIInterface.h"
#include <iostream>

int main() {
    try {
        CLIInterface cli;
        cli.run();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}