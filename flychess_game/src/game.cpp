#include "../include/game.h"

namespace flychess_game {
    
}

extern "C"{
EMSCRIPTEN_KEEPALIVE
int GetRandom() {
    int min = 1;
    int max = 6;
    int random_number = game_utils::get_random(min, max);
    std::cout << "Random number between " << min << " and " << max << ": " << random_number << std::endl;
    return random_number;
}
}