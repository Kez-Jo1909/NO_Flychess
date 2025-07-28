#ifndef GAME_H
#define GAME_H

#include <iostream>
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/map.h"

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

    Player& GetPlayer(int player_id);

    const ChessPieceInfo& GetPlayerChess(int player_id, int chess_id) {
        return players[player_id].GetChessPieceInfo(chess_id);
    }

    void IfPositionTaken(int position, int player_id, int chess_id);
private:
    std::vector<Player> players; // 玩家列表
};

FlychessGame& get_instance();

int rollDice();

}


/*以下为JS接口*/
#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>
#ifdef __cplusplus
extern "C"{
#endif

int rollDice();

void GameInit(int player_count, int chess_piece_count);

int GetPlayerCount();
int GetChessPieceCount();

const flychess_map::GridInfo* DrawChessPiece(int player_id, int chess_id);

int MoveChessPiece(int player_id, int chess_id, int steps);

int FlyChessPiece(int player_id, int chess_id);

int GetStartedChessCount(int player_id);

int GetFinishedChessCount(int player_id);

#ifdef __cplusplus
}
#endif
#endif // __EMSCRIPTEN__

#endif // GAME_H