#include "../include/player.h"

namespace flychess_game {
    Player::Player(game_utils::Color color, int chess_piece_count){
        if (color == game_utils::Color::UNDEFINED || chess_piece_count <= 0) {
            std::cerr << "Invalid player color or chess piece count." << std::endl;
            return;
        }

        player_color = color;
        player_state = PlayerState::WAITING;

        if (chess_pieces.size() > 0) {
            chess_pieces.clear();
        }

        for(int i = 0; i < chess_piece_count; i++){
            chess_pieces.push_back(ChessPiece(i, color, -1));
        }

        std::cout<< "Player created with color: " << static_cast<int>(color) << " and " << chess_piece_count << " chess pieces." << std::endl;
    }

    int Player::GameTurn(){
        if(this->player_state == PlayerState::FINISHED){
            return -1; // 玩家游戏已结束，直接跳过
        }
        this->player_state = PlayerState::PLAYING;
        return 1;
    }
}