#ifndef GAME_H
#define GAME_H

#include <iostream>
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/map.h"

namespace flychess_game {

struct PlayerInfo {
    int websocket_id;
    std::string player_name = "player";// TODO : 也许未来可以本地存储用户名
    game_utils::Color color;
    bool if_prepared = false;

    PlayerInfo(game_utils::Color color, std::string p_name, int web_id) : color(color), websocket_id(web_id) {
        if_prepared = false;
    }
};

class FlychessGameRoom {
public:
    FlychessGameRoom() = default;

    inline void setChessCount(int count) {
        chess_count_per_player = count;
    }

    inline bool setPlayerCount(int count) {
        if (players.size() <= count) {
            player_count = count;
            return true;
        }
        else
            return false;
    }

    void addPlayer(const PlayerInfo p_info);

    int getPlayerCount() const {
        return players.size();
    }

    int getMaxPlayerCount() const {
        return player_count;
    }

    int getChessPerPlayer() const {
        return chess_count_per_player;
    }

    PlayerInfo getPlayer(int index) {
        return players[index];
    }

    PlayerInfo getPlayerByWebId(int web_id) {
        for (const auto& player : players) {
            if (player.websocket_id == web_id) {
                return player;
            }
        }
        throw std::runtime_error("Player not found with the given websocket ID.");
    }

    bool ifAllPrepared();

    void setPrepared(int web_id);
    void setUnPrepared(int web_id);

    void DeletePlayer(int web_id);
private:
    int player_count = 4; // 玩家数量
    int chess_count_per_player = 4;
    std::vector<PlayerInfo> players; // 玩家信息列表
};


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

    inline void setChessCount(int count) {
        chess_count_per_player = count;
    }

    const int GetChessPieceCount() const {
        return chess_count_per_player;
    }

    Player& GetPlayer(int player_id);

    const ChessPieceInfo& GetPlayerChess(int player_id, int chess_id) {
        return players[player_id].GetChessPieceInfo(chess_id);
    }
    
    void changePlayerState(game_utils::Color color, PlayerState new_state);

    void IfPositionTaken(int position, int player_id, int chess_id);

    void InitGame();

    int GetPlayerToRollDice();

    int MoveChessPiece(int player_id, int chess_id, int steps);

    int GetStartedChessCount(int player_id);

    PlayerState getPlayerState(int color);

    int GetFinishedChessCount(int player_id);
    int FlyChessPiece(int player_id, int chess_id);

    int RollDice();

    int GetDice() const {
        return steps;
    }
private:
    std::vector<Player> players; // 玩家列表
    int chess_count_per_player = 4;
    int steps = -1; // 当前掷骰子的点数
};

// 原本给js前端预留的接口
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