#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>

namespace flychess_game {

enum class Color {
    UNDEFINED = -1,
    RED = 0,
    BLUE = 1,
    GREEN = 2,
    YELLOW = 3
};

enum class PlayerState {
    UNDEFINED = -1,
    WAITING = 0,
    PLAYING = 1,
    FINISHED = 2
};

class Player {
public:
    Player() = default;
    // TODO : 构造函数分为ai和玩家两种

    inline void SetColor(int color){
        if (color >= 0 && color <= 3) {
            player_color = static_cast<Color>(color);
        } else {
            player_color = Color::UNDEFINED;
            std::cerr << "Invalid color value. Setting to UNDEFINED." << std::endl;
        }
    }
private:
    Color player_color = Color::UNDEFINED;
    PlayerState player_state = PlayerState::UNDEFINED;
};

}

#endif