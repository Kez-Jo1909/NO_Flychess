#include "../include/game.h"

namespace flychess_game {
    void FlychessGame::AddNewPlayer(game_utils::Color color, int chess_piece_count) {
        players.emplace_back(color, chess_piece_count);
    }

    void FlychessGame::Reset() {
        players.clear();
        std::cout << "Game state has been reset." << std::endl;
    }

    static FlychessGame static_flychess_game;

    FlychessGame& get_instance() {
        return static_flychess_game;
    }
}


extern "C"{

    EMSCRIPTEN_KEEPALIVE
    int rollDice() {
        return game_utils::get_random(1, 6);
    }

    EMSCRIPTEN_KEEPALIVE
    void GameInit(int player_count, int chess_piece_count) {
        if (player_count <= 0) {
            std::cerr << "Invalid player count." << std::endl;
            return;
        }

        flychess_game::get_instance().Reset();

        for (int i = 0; i < player_count; i++) {
            flychess_game::get_instance().AddNewPlayer(static_cast<game_utils::Color>(i % 4), chess_piece_count);
            std::cout << "Player " << i + 1 << " added with color: " << static_cast<int>(static_cast<game_utils::Color>(i % 4)) << std::endl;
        }

        // 检查玩家数量
        if (flychess_game::get_instance().GetPlayerCount() != player_count) {
            std::cerr << "Not enough players to start the game." << std::endl;
            return;
        }else{
            std::cout << "创建完成,玩家数量检查完成" << std::endl;
        }

        // 检查棋子数量
        if (flychess_game::get_instance().GetChessPieceCount() != chess_piece_count) {
            std::cerr << "Chess piece count does not match." << std::endl;
            std::cerr << "Expected: " << chess_piece_count << ", Actual: " << flychess_game::get_instance().GetChessPieceCount() << std::endl;
            return;
        }else{
            std::cout << "棋子数量检查完成" << std::endl;
        }

    }

    EMSCRIPTEN_KEEPALIVE
    int GetPlayerCount() {
        return flychess_game::get_instance().GetPlayerCount();
    }

    EMSCRIPTEN_KEEPALIVE
    int GetChessPieceCount() {
        return flychess_game::get_instance().GetChessPieceCount();
    }

}