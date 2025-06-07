#ifndef GAME_H
#define GAME_H

#include <emscripten/emscripten.h>
#include <iostream>
#include "../include/utils.h"
#include "../include/player.h"

namespace flychess_game {
class FlyChessGame {
public:
    FlyChessGame();// 默认构造函数

    FlyChessGame(int playerCount, int machineCount = 0);
private:
    bool isGameOver = 0;
    bool isGameInit = 0;
    std::vector<Player*> players; // 玩家列表
};
}


/*以下为JS接口*/

#ifdef __cplusplus
extern "C"{
#endif

int GetRandom();

/**
 * @name GameProcessLink
 * @brief 游戏主循环函数接口
 */
void GameProcessLink();

#ifdef __cplusplus
}
#endif


#endif // GAME_H