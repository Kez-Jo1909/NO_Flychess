#ifndef PLAYER_H
#define PLAYER_H

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

private:
    Color player_color = Color::UNDEFINED;
    PlayerState player_state = PlayerState::UNDEFINED;
};

}

#endif