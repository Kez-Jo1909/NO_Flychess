#include "../include/player.h"

namespace flychess_game {
    void ChessPiece::preGoalMove(int steps, int pre_goal_position) {
        // 由于格子是在是写的太屎了，这里得重新做映射
        // 初次进入状态
        if(!piece_info.if_pre_goal) {
            int new_position = piece_info.position + steps;// 这个值>=pre_goal_position
            new_position -= pre_goal_position; // 计算新的位置增量
            if (new_position == 0) {
                piece_info.position = pre_goal_position; // 如果新位置为0，则设置为pre_goal_position
            }
            else {
                piece_info.position = 52 + new_position; // 否则设置为52 + 新位置增量
            }

            piece_info.if_pre_goal = 1; // 设置为预目标状态
            forward = true; // 设置为向前移动
            return;
        }
        else {
            if (piece_info.position == pre_goal_position) {
                piece_info.position = 52 + steps;
            }
            else {
                if (forward) {
                    int new_position = piece_info.position + steps; // 计算新的位置
                    if (new_position == 58){
                        // 该棋子完成
                        piece_info.position = -2; // 设置为-2表示棋子已完成
                        std::cout << "Chess piece " << piece_info.id << " has finished the game." << std::endl;
                        return;
                    }
                    else if (new_position > 58) {
                        new_position -= 58; // 如果新位置超过58，则进行环绕
                        // 貌似进入pregoal之后不会再回到pre_goal_position那个位置了
                        piece_info.position = 58 - new_position; // 设置为pre_goal_position + 新位置增量
                        std::cout << "Chess piece " << piece_info.id << " circle moved to pregoal position: " << piece_info.position << std::endl;
                        forward = false; // 设置为向后移动
                        return;
                    }
                    else {
                        piece_info.position = new_position; // 否则直接设置为新位置
                        std::cout << "Chess piece " << piece_info.id << " moved to pregoal position: " << piece_info.position << std::endl;
                        return;
                    }
                }
                else {
                    int new_position = piece_info.position - steps; // 计算新的位置
                    if (new_position < 52) {
                        forward = true;
                        piece_info.position = 52 - new_position + 52;
                        return;
                    }
                    else if (new_position == 52) {
                        piece_info.position = pre_goal_position; // 如果新位置为52，则设置为pre_goal_position
                        forward = true;
                        return;
                    }
                    else {
                        piece_info.position = new_position; // 否则直接设置为新位置
                        return;
                    }
                }
            }
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
                pregoal_position = 50;
                start_position = 1;
                break;
            case 1:
                pregoal_position = 37;
                start_position = 40;
                break;
            case 2:
                pregoal_position = 24;
                start_position = 27;
                break;
            case 3:
                pregoal_position = 11;
                start_position = 14;
                break;
            default:
                std::cerr << "Invalid player color." << std::endl;
                return;
        }

        if (chess_pieces.size() > 0) {
            chess_pieces.clear();
        }

        for(int i = 0; i < chess_piece_count; i++){
            chess_pieces.push_back(ChessPiece(i, color, -1, 0));
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
        else if(chess_piece_to_move.GetChessPieceInfo().position == 0){
            // 如果在START，从起始位置开始移动
            std::cout<<"move from start"<<std::endl;
            chess_piece_to_move.MoveFromStart(steps, start_position);
            std::cout<<"Chess piece " << chess_id << " moved from start position." << std::endl;
            return 1;
        }
        else if((chess_piece_to_move.GetChessPieceInfo().position < pregoal_position && chess_piece_to_move.GetChessPieceInfo().position + steps >= pregoal_position) || chess_piece_to_move.ifPreGoal()){
            std::cout<<"pre goal move"<<std::endl;
            chess_piece_to_move.preGoalMove(steps, pregoal_position);
            std::cout<<"Chess piece " << chess_id << " moved to "<< chess_piece_to_move.GetChessPieceInfo().position << std::endl;
            return 1;
        }
        else{
            std::cout<<"simple move"<<std::endl;
            chess_piece_to_move.SimpleMove(steps);
            std::cout<<"Chess piece " << chess_id << " moved to "<< chess_piece_to_move.GetChessPieceInfo().position << std::endl;
            return 1;
        }
        // TODO : 检查是否在特殊格子上（BRIDGE或GOAL）
    }

    void Player::SendChessPieceBackHome(int chess_id) {
        chess_pieces[chess_id].MoveBackHome();
        std::cout << "Chess piece " << chess_id << " sent back home." << std::endl;
    }
}