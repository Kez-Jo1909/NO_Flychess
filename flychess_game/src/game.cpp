#include "../include/game.h"

namespace flychess_game {
    void FlychessGame::AddNewPlayer(game_utils::Color color, int chess_piece_count) {
        players.push_back(Player(color, chess_piece_count));
    }
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