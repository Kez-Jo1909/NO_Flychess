#include "../include/player.h"

namespace flychess_game {
    void ChessPiece::preGoalMove(int steps, int pre_goal_position) {
        int new_position = piece_info.position + steps;
        if (new_position < 58){
            piece_info.position = new_position;
            return;
        }
        else if(new_position == 58){
            piece_info.position = -2;// -2设置为完成
            std::cout << "Chess piece " << piece_info.id << " has reached the goal!" << std::endl;
            return;
        }
        else {
            int steps_to_return = new_position - 58;
            piece_info.position = 58 - steps_to_return;
            return;
        }
    }



    Player::Player(game_utils::Color color, int chess_piece_count){
        if (color == game_utils::Color::UNDEFINED || chess_piece_count <= 0) {
            std::cerr << "Invalid player color or chess piece count." << std::endl;
            return;
        }

        player_color = color;
        player_state = PlayerState::WAITING;

        switch(static_cast<int>(color)){
            case 0:
                pregoal_position = 50; // 预设位置为58
                break;
            case 1:
                pregoal_position = 37; // 预设位置为58
                break;
            case 2:
                pregoal_position = 24; // 预设位置为58
                break;
            case 3:
                pregoal_position = 11; // 预设位置为58
                break;
            default:
                std::cerr << "Invalid player color." << std::endl;
                return;
        }

        if (chess_pieces.size() > 0) {
            chess_pieces.clear();
        }

        for(int i = 0; i < chess_piece_count; i++){
            chess_pieces.push_back(ChessPiece(i, color, -1));
            std::cout<<"init posisiton: "<< chess_pieces[i].GetChessPieceInfo().position << std::endl;
        }

        // std::cout<< "Player created with color: " << static_cast<int>(color) << " and " << chess_piece_count << " chess pieces." << std::endl;
    }

    int Player::GameTurn(){
        if(this->player_state == PlayerState::FINISHED){
            return -1; // 玩家游戏已结束，直接跳过
        }
        this->player_state = PlayerState::PLAYING;
        return 1;
    }

    int Player::MoveChessPiece(int chess_id, int steps){
        std::cout<< "Player " << static_cast<int>(player_color) << " is moving chess piece " << chess_id << " with steps: " << steps << std::endl;

        auto& chess_piece_to_move = chess_pieces[chess_id];
        if(steps <= 0 || steps > 6){
            std::cerr << "Invalid steps: " << steps << ". Steps must be greater than 0." << std::endl;
            return -1;
        }
        else if(chess_piece_to_move.GetChessPieceInfo().position < 0 && steps != 6){
            std::cerr << "Invalid chess piece movement: Chess piece is not on the board." << std::endl;
            return 0;
        }

        std::cout<<"ready to move chess piece "<< std::endl;
        std::cout<<"chess piece position: "<< chess_piece_to_move.GetChessPieceInfo().position << std::endl;

        if(steps == 6 && chess_piece_to_move.GetChessPieceInfo().position == -1){
            std::cout<<"move to start"<<std::endl;
            // 如果是6点，且棋子不在起始位置，则将棋子放置到起始位置
            chess_piece_to_move.MoveToStart();
            std::cout << "Chess piece " << chess_id << " moved to start position." << std::endl;
            return 1;
        }
        else if(chess_piece_to_move.GetChessPieceInfo().position >= pregoal_position){
            std::cout<<"pre goal move"<<std::endl;
            chess_piece_to_move.preGoalMove(steps, pregoal_position);
            return 1;
        }
        else{
            std::cout<<"simple move"<<std::endl;
            chess_piece_to_move.SimpleMove(steps);
            return 1;
        }
        // TODO : 检查是否在特殊格子上（BRIDGE或GOAL）
        // TODO : 检查该格是否被占用，是否可以吃掉对方棋子等逻辑
    }
}