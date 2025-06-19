#include "../include/game.h"

namespace flychess_game {
    void FlychessGame::AddNewPlayer(game_utils::Color color, int chess_piece_count) {
        players.emplace_back(color, chess_piece_count);
    }

    void FlychessGame::Reset() {
        players.clear();
        std::cout << "Game state has been reset." << std::endl;
    }

    FlychessGame& get_instance() {
        static FlychessGame static_flychess_game;
        return static_flychess_game;
    }

    Player& FlychessGame::GetPlayer(int player_id) {
        return players[player_id];
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

    EMSCRIPTEN_KEEPALIVE
    const flychess_map::GridInfo* DrawChessPiece(int player_id, int chess_id) {
        if (player_id < 0 || player_id >= flychess_game::get_instance().GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return nullptr;
        }
        auto chess_piece_to_draw = flychess_game::get_instance().GetPlayerChess(player_id, chess_id);

        if (chess_id != chess_piece_to_draw.id) {
            std::cerr << "Error: Chess ID does not match: " << chess_id << " != " << chess_piece_to_draw.id << std::endl;
            return nullptr;
        }

        if (flychess_game::get_instance().GetPlayer(player_id).GetColor() != chess_piece_to_draw.color) {
            std::cerr << "Error: Chess color does not match player color." << std::endl;
            return nullptr;
        }

        return &flychess_map::getGameMap().searchGridInfo(chess_piece_to_draw.position, static_cast<int>(chess_piece_to_draw.color), chess_id);
    }

}