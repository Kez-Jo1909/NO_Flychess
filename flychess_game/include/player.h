#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include "./utils.h"

namespace flychess_game {

enum class PlayerState {
    UNDEFINED = -1,
    WAITING = 0,
    PLAYING = 1,
    FINISHED = 2
};

struct ChessPieceInfo {
    int id = -1; // 棋子ID
    game_utils::Color color = game_utils::Color::UNDEFINED; // 棋子颜色
    int position = -1; // 棋子位置
};

class ChessPiece{
public:
    ChessPiece() = default;

    ChessPiece(int id, game_utils::Color color, int position = -1) {
        piece_info.id = id;
        piece_info.color = color;
        piece_info.position = position;
        // std::cout << "ChessPiece created with ID: " << id << ", Color: " << static_cast<int>(color) << ", Position: " << position << std::endl;
    }

    inline void SetId(int new_id) {
        piece_info.id = new_id;
    }

    inline void SetColor(game_utils::Color new_color) {
        piece_info.color = new_color;
    }

    inline const ChessPieceInfo& GetChessPieceInfo() const {
        return piece_info;
    }

    inline void MoveToStart() {
        piece_info.position = 0;
    }

    inline void SimpleMove(int steps) {
        piece_info.position += steps;
    }

    void preGoalMove(int steps, int pre_goal_position);
private:
    ChessPieceInfo piece_info;
};



class Player {
public:
    Player() = default;
    // TODO : 构造函数分为ai和玩家两种

    Player(game_utils::Color color, int chess_piece_count = 4);

    inline void SetColor(game_utils::Color color){
        player_color = color;
    }

    inline game_utils::Color GetColor() const {
        return player_color;
    }

    inline const int GetChessPieceCount() const {
        return chess_pieces.size();
    }

    inline const ChessPieceInfo& GetChessPieceInfo(int index) const {
        return chess_pieces[index].GetChessPieceInfo();
    }

    int GameTurn();

    int GetStartedChessPieceCount() const {
        int count = 0;
        for (const auto& chess_piece : chess_pieces) {
            if (chess_piece.GetChessPieceInfo().position >= 0) {
                count++;
            }
        }
        return count;
    }


    /**
     * @name MoveChessPiece
     * @brief 移动棋子
     * @param chess_id 棋子ID
     * @param steps 移动步数
     * @return 返回值：1表示成功，0表示棋子未在棋盘上，-1表示参数错误
     */
    int MoveChessPiece(int chess_id, int steps);
private:
    game_utils::Color player_color = game_utils::Color::UNDEFINED;
    PlayerState player_state = PlayerState::UNDEFINED;
    std::vector<ChessPiece> chess_pieces; // 棋子列表
    int pregoal_position = 52; // 预设目标位置
};

}

#endif