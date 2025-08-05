#ifndef GAME_H
#define GAME_H

#include <iostream>
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/map.h"

namespace flychess_game {

struct PlayerInfo {
    int websocket_id;
    std::string player_name = "player";// TODO : 
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

    void setPrepared(int web_id);
    void setUnPrepared(int web_id);

    void DeletePlayer(int web_id);
private:
    int player_count = 4; // 
    int chess_count_per_player = 4;
    std::vector<PlayerInfo> players; // 
};


class FlychessGame {
public:
    FlychessGame() = default;

    // 
    void AddNewPlayer(game_utils::Color color, int chess_piece_count = 4);
    
    // 
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
    std::vector<Player> players; // 
};

// js
FlychessGame& get_instance();

int rollDice();

}


/*JS*/
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