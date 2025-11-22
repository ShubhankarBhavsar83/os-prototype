#include "Application.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Game Starting..." << std::endl;

    Application app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }

    app.run();
    app.shutdown();

    std::cout << "Game Shutdown Complete" << std::endl;
    return 0;
}