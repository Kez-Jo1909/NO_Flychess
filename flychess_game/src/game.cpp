#include "../include/game.h"

namespace flychess_game {

}


extern "C"{

    EMSCRIPTEN_KEEPALIVE
    int rollDice() {
        return game_utils::get_random(1, 6);
    }

    EMSCRIPTEN_KEEPALIVE
    void FrontendTest(){

    }

}