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
private:
    std::vector<Player> players; // 玩家列表
};

}


/*以下为JS接口*/

#ifdef __cplusplus
extern "C"{
#endif

int rollDice();

/**
 * @name FrontendTest
 * @brief 前端测试函数接口
 */
void FrontendTest();

#ifdef __cplusplus
}
#endif


#endif // GAME_H