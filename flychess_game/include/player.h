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

class ChessPiece{
public:
private:
    int id = -1; // 棋子ID
    game_utils::Color color = game_utils::Color::UNDEFINED; // 棋子颜色
    int position = 0; // 棋子位置
    bool isHome = true; // 是否在家
    bool isFinished = false; // 是否完成游戏
};

class Player {
public:
    Player() = default;
    // TODO : 构造函数分为ai和玩家两种

    inline void SetColor(int color){
        if (color >= 0 && color <= 3) {
            player_color = static_cast<game_utils::Color>(color);
        } else {
            player_color = game_utils::Color::UNDEFINED;
            std::cerr << "Invalid color value. Setting to UNDEFINED." << std::endl;
        }
    }

    inline game_utils::Color GetColor() const {
        return player_color;
    }
private:
    game_utils::Color player_color = game_utils::Color::UNDEFINED;
    PlayerState player_state = PlayerState::UNDEFINED;
};

}

#endif