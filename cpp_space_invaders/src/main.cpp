#include "space_invaders/Game.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        SpaceInvaders::Game game;
        if (game.initialize()) {
            game.run();
        } else {
            std::cerr << "Failed to initialize game" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
