#ifndef SERVER_H
#define SERVER_H

#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "game.h"
#include "utils.h"
#include "card.h"
#include "command.h"
#include "resolution_stack.h"

namespace flychess_server {

// 服务器事件回调（供外部 Observer 使用，替代旧 Qt 信号）
struct ServerCallbacks {
    std::function<void(const std::string& client_id, const std::string& ip)> on_client_connected;
    std::function<void(const std::string& client_id)> on_client_disconnected;
    std::function<void(const std::string& client_id, const std::string& msg)> on_message_received;
};

class FlychessServer {
public:
    explicit FlychessServer(int port = 8080, const std::string& host = "0.0.0.0");

    bool start();
    void stop();
    void handleMessage(const ix::WebSocketMessagePtr& msg, const std::string& client_id);

    void GameStart();

    inline void setUrlString(const std::string& url) {
        url_to_show = url;
    }

    inline void setCallbacks(ServerCallbacks cbs) {
        callbacks_ = std::move(cbs);
    }

private:
    // ============================================================
    // 消息处理器类型
    // ============================================================
    using MessageHandler = std::function<void(const nlohmann::json&, const std::string&)>;

    // ============================================================
    // 通信底层
    // ============================================================
    int port_;
    std::unique_ptr<ix::WebSocketServer> server_;
    std::unordered_map<std::string, std::shared_ptr<ix::WebSocket>> clients_;

    ServerCallbacks callbacks_;

    void setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket, const std::string& client_id);
    void sendToClient(const std::string& client_id, const std::string msg);
    void BroadCast(const std::string& msg);

    // ============================================================
    // 消息处理器注册表
    // ============================================================
    std::unordered_map<std::string, MessageHandler> handlers_;
    void initHandlers();

    void handleRollDice(const nlohmann::json& msg, const std::string& client_id);
    void handleFinishTextWaiting(const nlohmann::json& msg, const std::string& client_id);
    void handleUserInfo(const nlohmann::json& msg, const std::string& client_id);
    void handleGetPrepared(const nlohmann::json& msg, const std::string& client_id);
    void handleGetUnprepared(const nlohmann::json& msg, const std::string& client_id);
    void handleChatMessage(const nlohmann::json& msg, const std::string& client_id);
    void handleUseCard(const nlohmann::json& msg, const std::string& client_id);
    void handleUpdatePlayerCount(const nlohmann::json& msg, const std::string& client_id);
    void handleUpdateChessCount(const nlohmann::json& msg, const std::string& client_id);
    void handleChooseChessPiece(const nlohmann::json& msg, const std::string& client_id);
    void handleFinishUseCard(const nlohmann::json& msg, const std::string& client_id);

    // ============================================================
    // 游戏逻辑辅助
    // ============================================================
    void sendDiceNum(int dice_result, const std::string& client_id);
    void CardState(const std::string client_id);

    void BroadCastPlayerList();
    void BroadCastPlayerCount();
    void BroadCastChessCount();
    void BroadCastRoomInfo();
    void BroadCastPieceInfo(int player_count, int cp_count);
    void BroadCastToRollDice(int color_to_roll);
    void BroadCastSomeoneFinished(int color_finished);
    void BroadCastAllFinished();

    bool validateCardTiming(int function_time, flychess_game::PlayerState current_state);
    void advanceToNextPlayer(int current_color);

    // ============================================================
    // 游戏状态
    // ============================================================
    flychess_game::FlychessGame *game_ = nullptr;
    flychess_game::FlychessGameRoom *game_room_ = nullptr;

    std::vector<int> finished_players;

    std::string host_;
    std::string url_to_show;

    flychess_game::PlayerState game_state_ = flychess_game::PlayerState::UNDEFINED;
    int player_to_move_ = -1;

    // ============================================================
    // 卡牌系统
    // ============================================================
    flychess_game::ResolutionStack resolution_stack_;
    nlohmann::json card_config_;
    bool game_started_ = false;  // 防重复开始

    // TODO 卡牌数量上限检测
};

}  // namespace flychess_server

#endif
