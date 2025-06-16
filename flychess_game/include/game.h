#ifndef GAME_H
#define GAME_H

#include <emscripten/emscripten.h>
#include <iostream>
#include "../include/utils.h"
#include "../include/player.h"

namespace flychess_game {
class FlychessGame {
public:
    FlychessGame() = default;

    // 添加玩家
    void AddNewPlayer(game_utils::Color color, int chess_piece_count = 4);
    // 重设游戏状态
    void Reset();

    inline const int GetPlayerCount() const {
        return players.size();
    }

    const int GetChessPieceCount() const {
        if (players.empty()) return -1;
        return players.back().GetChessPieceCount();
    }
private:
    std::vector<Player> players; // 玩家列表
};

FlychessGame& get_instance();

}


/*以下为JS接口*/

#ifdef __cplusplus
extern "C"{
#endif

int rollDice();

void GameInit(int player_count, int chess_piece_count);

int GetPlayerCount();
int GetChessPieceCount();

#ifdef __cplusplus
}
#endif


#endif // GAME_H