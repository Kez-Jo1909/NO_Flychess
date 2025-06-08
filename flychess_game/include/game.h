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

    void DeleteGame(){
        if (!isGameOver) {
            std::cout << "Game isn't over. No need to delete." << std::endl;
            return;
        }

        for (Player* player : players) {
            delete player; // 释放玩家对象
        }
        players.clear(); // 清空玩家列表

        isGameOver = true; // 设置游戏结束状态
        isGameInit = false; // 重置游戏初始化状态
        std::cout << "Game resources cleaned up." << std::endl;
    }

    // 以下主要获取状态量的函数
    inline int GetPlayerCount() const {
        return players.size();
    }

    inline bool IsGameInit() const {
        return isGameInit;
    }

    inline bool IsGameOver() const {
        return isGameOver;
    }

    inline void SetGameOver() {
        isGameOver = 1;
    }
private:
    bool isGameOver = 0;
    bool isGameInit = 0;
    std::vector<Player*> players; // 玩家列表
};

void GameProcess();

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

/**
 * @name FrontendTest
 * @brief 前端测试函数接口
 */
void FrontendTest();

#ifdef __cplusplus
}
#endif


#endif // GAME_H