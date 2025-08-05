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
    int id = -1; // ID
    game_utils::Color color = game_utils::Color::UNDEFINED; // 
    int position = -1; // 
    int if_pre_goal = 0;
};

class ChessPiece{
public:
    ChessPiece() = default;

    ChessPiece(int id, game_utils::Color color, int position = -1, int if_pre_goal = 0) {
        piece_info.id = id;
        piece_info.color = color;
        piece_info.position = position;
        piece_info.if_pre_goal = if_pre_goal;
        // std::cout << "ChessPiece created with ID: " << id << ", Color: " << static_cast<int>(color) << ", Position: " << position << std::endl;
    }

    inline void SetId(int new_id) {
        piece_info.id = new_id;
    }

    inline void SetPosition(int new_position) {
        piece_info.position = new_position;
    }

    inline void SetPreGoal() {
        piece_info.if_pre_goal = 1;
    }

    inline const int ifPreGoal() {
        return piece_info.if_pre_goal;
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

    inline void MoveBackHome() {
        piece_info.position = -1;
    }

    inline void SimpleMove(int steps) {
        piece_info.position += steps;
        piece_info.position = piece_info.position <= 52 ? piece_info.position : piece_info.position - 52; // 1-52
        std::cout<< "Chess piece " << piece_info.id << " moved to position: " << piece_info.position << std::endl;
    }

    inline void MoveFromStart(int steps, int start_position) {
        // 
        int new_position = start_position + steps - 1;
        piece_info.position = new_position;
    }

    void preGoalMove(int steps, int pre_goal_position);
private:
    ChessPieceInfo piece_info;
    bool forward = true; // 
};



class Player {
public:
    Player() = default;
    // TODO : ai

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

    int GetFinishedChessPieceCount() const {
        int count = 0;
        for (const auto& chess_piece : chess_pieces) {
            if (chess_piece.GetChessPieceInfo().position == -2) {
                count++;
            }
        }
        return count;
    }

    inline void KillChessPiece() {
        kill_chess_count++;
    }

    inline void GetKilledChessPiece() {
        killed_chess_count++;
    }

    inline const int GetIfPreGoal(int chess_id) {
        return chess_pieces[chess_id].ifPreGoal();
    }

    /**
     * @name MoveChessPiece
     * @brief 
     * @param chess_id ID
     * @param steps 
     * @return 10-1
     */
    int MoveChessPiece(int chess_id, int steps);

    /**
     * @name FlyChessPiece
     * @brief 
     * @param chess_id ID
     * @return 10-1
     */
    int FlyChessPiece(int chess_id);

    void SendChessPieceBackHome(int chess_id);
private:
    game_utils::Color player_color = game_utils::Color::UNDEFINED;
    PlayerState player_state = PlayerState::UNDEFINED;
    std::vector<ChessPiece> chess_pieces; // 
    int pregoal_position = 52; // 
    int start_position = 0; // 
    int kill_chess_count = 0; // 
    int killed_chess_count = 0; // 
    int index_fly = 2;
    std::pair<int, int> bridge_start_end_position = {18, 30}; // 
};

}

#endif