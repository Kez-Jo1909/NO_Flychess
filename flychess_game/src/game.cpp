#include "../include/game.h"

namespace flychess_game {
    void FlychessGameRoom::addPlayer(const PlayerInfo p_info) {
        players.push_back(p_info);
        std::cout << "Player added: " << p_info.player_name << ", Color: " << static_cast<int>(p_info.color) << std::endl;
    }

    void FlychessGameRoom::setPrepared(int web_id) {
        for (auto& player : players) {
            if (player.websocket_id == web_id) {
                player.if_prepared = true;
                // std::cout << "Player " << player.player_name << " is now prepared." << std::endl;
                return;
            }
        }
        std::cerr << "Player with websocket ID " << web_id << " not found." << std::endl;
    }

    void FlychessGameRoom::setUnPrepared(int web_id) {
        for (auto& player : players) {
            if (player.websocket_id == web_id) {
                player.if_prepared = false;
                // std::cout << "Player " << player.player_name << " is now prepared." << std::endl;
                return;
            }
        }
        std::cerr << "Player with websocket ID " << web_id << " not found." << std::endl;
    }

    void FlychessGameRoom::DeletePlayer(int web_id) {
        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i].websocket_id == web_id) {
                // 删除当前元素
                players.erase(players.begin() + i);

                // 调整后续玩家的颜色
                for (size_t j = i; j < players.size(); ++j) {
                    int c = static_cast<int>(players[j].color);
                    players[j].color = static_cast<game_utils::Color>(c - 1);
                }

                break; // 只删除一个，直接退出
            }
        }
    }

    void FlychessGame::AddNewPlayer(game_utils::Color color, int chess_piece_count) {
        players.emplace_back(color, chess_piece_count);
    }

    void FlychessGame::Reset() {
        players.clear();
        std::cout << "Game state has been reset." << std::endl;
    }

    FlychessGame& get_instance() {
        static FlychessGame static_flychess_game;
        return static_flychess_game;
    }

    Player& FlychessGame::GetPlayer(int player_id) {
        return players[player_id];
    }

    void FlychessGame::IfPositionTaken(int position, int player_id, int chess_id) {
        if (position <= 0 || players[player_id].GetIfPreGoal(chess_id))
            return; // 如果位置不可能遇到其他玩家棋子，直接返回
        for (int i = 0; i < players.size(); i++) {
            if (i == player_id) continue; // 跳过当前玩家

            auto& other_player = players[i];

            int chess_piece_count = other_player.GetChessPieceCount();

            for(int j = 0; j < chess_piece_count; j++) {
                auto other_chess_piece_info = other_player.GetChessPieceInfo(j);
                if (other_chess_piece_info.position == position) {
                    // 如果格子被占用，送回家
                    other_player.SendChessPieceBackHome(j);
                    other_player.GetKilledChessPiece();
                    players[player_id].KillChessPiece();
                    std::cout<< "Position " << position << " is occupied by player " << i 
                              << "'s chess piece " << j << ". Sending it back home." << std::endl;
                }
            }

        }
    }

    int rollDice() {
        return game_utils::get_random(1, 6);
    }

    bool FlychessGameRoom::ifAllPrepared() {
        for (const auto& player : players) {
            if (!player.if_prepared) {
                return false; // 只要有一个玩家未准备，返回false
            }
        }
        return true; // 所有玩家都已准备
    }

    void FlychessGame::InitGame() {
        // 初始化游戏状态
        for (auto& player : players) {
            player.setPlayerState(PlayerState::WAITING);
        }
        players[0].setPlayerState(PlayerState::ROLLING);
    }

    int FlychessGame::GetPlayerToRollDice() {
        for (const auto& player : players) {
            if (player.GetPlayerState() == PlayerState::ROLLING) {
                game_utils::Color ret_color =  player.GetColor();
                return static_cast<int>(ret_color);
            }
        }
        return -1;
    }

    void FlychessGame::changePlayerState(game_utils::Color color, PlayerState new_state) {
        int index = static_cast<int>(color);
        if (index >= 0 && index < players.size()) {
            players[index].setPlayerState(new_state);
        }
    }


    int FlychessGame::MoveChessPiece(int player_id, int chess_id, int steps) {
        if (player_id < 0 || player_id >= this->GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return -2;
        }

        auto& player = this->GetPlayer(player_id);
        int ret = player.MoveChessPiece(chess_id, steps);
        
        // 如果有问题直接返回错误码
        if(ret <= 0)
            return ret;
        else {
            // 移动成功开始检查格子是否占用
            auto chess_piece_info = player.GetChessPieceInfo(chess_id);
            auto position  = chess_piece_info.position;
            this->IfPositionTaken(position, player_id, chess_id);
        }
        return ret;
    }
}

// JS 接口部分
#ifdef __EMSCRIPTEN__

extern "C"{

    EMSCRIPTEN_KEEPALIVE
    int rollDice() {
        return game_utils::get_random(1, 6);
    }

    EMSCRIPTEN_KEEPALIVE
    void GameInit(int player_count, int chess_piece_count) {
        if (player_count <= 0) {
            std::cerr << "Invalid player count." << std::endl;
            return;
        }

        flychess_game::get_instance().Reset();

        for (int i = 0; i < player_count; i++) {
            flychess_game::get_instance().AddNewPlayer(static_cast<game_utils::Color>(i % 4), chess_piece_count);
            std::cout << "Player " << i + 1 << " added with color: " << static_cast<int>(static_cast<game_utils::Color>(i % 4)) << std::endl;
        }

        // 检查玩家数量
        if (flychess_game::get_instance().GetPlayerCount() != player_count) {
            std::cerr << "Not enough players to start the game." << std::endl;
            return;
        }else{
            std::cout << "创建完成,玩家数量检查完成" << std::endl;
        }

        // 检查棋子数量
        if (flychess_game::get_instance().GetChessPieceCount() != chess_piece_count) {
            std::cerr << "Chess piece count does not match." << std::endl;
            std::cerr << "Expected: " << chess_piece_count << ", Actual: " << flychess_game::get_instance().GetChessPieceCount() << std::endl;
            return;
        }else{
            std::cout << "棋子数量检查完成" << std::endl;
        }

    }

    EMSCRIPTEN_KEEPALIVE
    int GetPlayerCount() {
        return flychess_game::get_instance().GetPlayerCount();
    }

    EMSCRIPTEN_KEEPALIVE
    int GetChessPieceCount() {
        return flychess_game::get_instance().GetChessPieceCount();
    }

    EMSCRIPTEN_KEEPALIVE
    const flychess_map::GridInfo* DrawChessPiece(int player_id, int chess_id) {
        if (player_id < 0 || player_id >= flychess_game::get_instance().GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return nullptr;
        }
        auto chess_piece_to_draw = flychess_game::get_instance().GetPlayerChess(player_id, chess_id);

        if (chess_id != chess_piece_to_draw.id) {
            std::cerr << "Error: Chess ID does not match: " << chess_id << " != " << chess_piece_to_draw.id << std::endl;
            return nullptr;
        }

        if (flychess_game::get_instance().GetPlayer(player_id).GetColor() != chess_piece_to_draw.color) {
            std::cout<< "chess color: " << static_cast<int>(chess_piece_to_draw.color) << std::endl;
            std::cout<< "player color: " << static_cast<int>(flychess_game::get_instance().GetPlayer(player_id).GetColor()) << std::endl;
            std::cerr << "Error: Chess color does not match player color." << std::endl;
            return nullptr;
        }

        return &flychess_map::getGameMap().searchGridInfo(chess_piece_to_draw.position, static_cast<int>(chess_piece_to_draw.color), chess_id);
    }

    EMSCRIPTEN_KEEPALIVE
    int MoveChessPiece(int player_id, int chess_id, int steps) {
        if (player_id < 0 || player_id >= flychess_game::get_instance().GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return -2;
        }

        auto& player = flychess_game::get_instance().GetPlayer(player_id);
        int ret = player.MoveChessPiece(chess_id, steps);
        
        // 如果有问题直接返回错误码
        if(ret <= 0)
            return ret;
        else {
            // 移动成功开始检查格子是否占用
            auto chess_piece_info = player.GetChessPieceInfo(chess_id);
            auto position  = chess_piece_info.position;
            flychess_game::get_instance().IfPositionTaken(position, player_id, chess_id);
        }
        return ret;
    }

    EMSCRIPTEN_KEEPALIVE
    int FlyChessPiece(int player_id, int chess_id) {
        if (player_id < 0 || player_id >= flychess_game::get_instance().GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return -2;
        }

        auto& player = flychess_game::get_instance().GetPlayer(player_id);
        int ret = player.FlyChessPiece(chess_id);

        if (ret <= 0)
            return ret; // 如果没有飞行或发生错误，直接返回
        else {
            // 飞行成功开始检查格子是否占用
            auto chess_piece_info = player.GetChessPieceInfo(chess_id);
            auto position = chess_piece_info.position;
            flychess_game::get_instance().IfPositionTaken(position, player_id, chess_id);
            return ret; // 返回飞行结果
        }
    }

    EMSCRIPTEN_KEEPALIVE
    int GetStartedChessCount(int player_id) {
        if (player_id < 0 || player_id >= flychess_game::get_instance().GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return -1;
        }
        return flychess_game::get_instance().GetPlayer(player_id).GetStartedChessPieceCount();
    }

    EMSCRIPTEN_KEEPALIVE
    int GetFinishedChessCount(int player_id) {
        if (player_id < 0 || player_id >= flychess_game::get_instance().GetPlayerCount()) {
            std::cerr << "Invalid player ID: " << player_id << std::endl;
            return -1;
        }
        return flychess_game::get_instance().GetPlayer(player_id).GetFinishedChessPieceCount();
    }
}
#endif // __EMSCRIPTEN__