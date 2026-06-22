#include "core/game.hpp"

int main()
{
    Game& game = Game::getInstance();
    game.runMainLoop();

    return 0;
}
