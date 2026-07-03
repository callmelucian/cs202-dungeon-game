#include "core/game.hpp"
#include <iostream>
#include <string_view>

void runUITests();

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string_view(argv[1]) == "--test") {
        runUITests();
        return 0;
    }

    
    Game& game = Game::getInstance();
    std::cerr << "Got here" << std::endl;
    game.runMainLoop();

    return 0;
}
