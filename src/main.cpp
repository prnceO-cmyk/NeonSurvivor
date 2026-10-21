
#include "Game.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Game game;

    if (!game.init()) {
        return 1;
    }

    game.run();
    game.shutdown();

    return 0;
}
