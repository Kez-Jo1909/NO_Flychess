#ifndef GAME_H
#define GAME_H

#include <emscripten/emscripten.h>
#include <iostream>
#include "../include/utils.h"
#include "../include/player.h"

namespace flychess_game {


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