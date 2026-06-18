#include "../include/server.h"
#include "card.h"
#include "game.h"
#include "command.h"
#include "resolution_stack.h"
#include <thread>
#include <chrono>
#include <algorithm>

namespace flychess_server {

// ============================================================
// 构造 / 启动 / 停止
// ============================================================

FlychessServer::FlychessServer(int port, const std::string& host)
    : port_(port), host_(host),
      server_(std::make_unique<ix::WebSocketServer>(port, host))
{
    server_->setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
               std::shared_ptr<ix::ConnectionState> connectionState)
        {
            std::cout << "New connection received." << std::endl;
            if (connectionState) {
                std::string client_id = connectionState->getId();
                std::string ip = connectionState->getRemoteIp();
                std::cout << "Client ID: " << client_id << ", IP: " << ip << std::endl;

                if (auto webSocket = weakWebSocket.lock()) {
                    clients_[client_id] = webSocket;
                    setupMessageCallback(webSocket, client_id);
                }

                // 回调通知
                if (callbacks_.on_client_connected) {
                    callbacks_.on_client_connected(client_id, ip);
                }
            }
        }
    );

    game_room_ = new flychess_game::FlychessGameRoom();
    initHandlers();

    // 预加载卡牌配置——优先从工作目录，其次从 exe 所在目录
    card_config_ = flychess_game::load_card_config("config/card.json");
    if (card_config_.empty()) {
        card_config_ = flychess_game::load_card_config("../../config/card.json");
    }
}

void FlychessServer::stop() {
    if (server_) {
        server_->stop();
    }
}

bool FlychessServer::start() {
    auto res = server_->listen();
    if (!res.first) {
        std::cerr << "Listen failed: " << res.second << std::endl;
#ifdef _WIN32
        printf("setsockopt error: %d\n", WSAGetLastError());
#else
        perror("setsockopt error");
#endif
        return false;
    }

    server_->start();
    std::cout << "WebSocket server started on port " << port_ << std::endl;
    return true;
}

// ============================================================
// 消息分发
// ============================================================

void FlychessServer::setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket,
                                           const std::string& client_id) {
    webSocket->setOnMessageCallback(
        [this, client_id](const ix::WebSocketMessagePtr& msg) {
            handleMessage(msg, client_id);
        }
    );
}

void FlychessServer::handleMessage(const ix::WebSocketMessagePtr& msg,
                                    const std::string& client_id) {
    // 断开连接
    if (msg->type == ix::WebSocketMessageType::Close) {
        std::cout << "客户端 [" << client_id << "] 断开连接。" << std::endl;
        clients_.erase(client_id);

        nlohmann::json leave_msg;
        leave_msg["type"] = "leave_room";
        const auto& player_to_leave_info = game_room_->getPlayerByWebId(std::stoi(client_id));
        int color_value = static_cast<int>(player_to_leave_info.color);
        leave_msg["color"] = std::to_string(color_value);
        leave_msg["name"] = player_to_leave_info.player_name;
        this->BroadCast(leave_msg.dump());

        game_room_->DeletePlayer(std::stoi(client_id));
        this->BroadCastPlayerList();

        if (callbacks_.on_client_disconnected) {
            callbacks_.on_client_disconnected(client_id);
        }
        return;
    }

    // 非文本消息跳过
    if (msg->type != ix::WebSocketMessageType::Message) return;

    std::cout << "收到来自ID [" << client_id << "] 的消息: ";
    const std::string& msg_text = msg->str;

    // 非 JSON 消息
    if (msg_text.empty() || (msg_text[0] != '{' && msg_text[0] != '[')) {
        std::cout << "non-json message: " << msg_text << std::endl;
        if (callbacks_.on_message_received) {
            callbacks_.on_message_received(client_id, msg_text);
        }
        return;
    }

    // JSON 分发
    try {
        auto j = nlohmann::json::parse(msg_text);
        std::string type = j.at("type");
        std::cout << "type=" << type << std::endl;

        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            it->second(j, client_id);
        } else {
            std::cout << "未知消息类型, 内容: " << msg_text << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[JSON Parse Error] " << e.what() << std::endl;
    }
}

// ============================================================
// 消息处理器注册
// ============================================================

void FlychessServer::initHandlers() {
    handlers_["rolldice"]             = [this](auto& m, auto& c) { handleRollDice(m, c); };
    handlers_["finish_text_waiting"]  = [this](auto& m, auto& c) { handleFinishTextWaiting(m, c); };
    handlers_["userInfo"]             = [this](auto& m, auto& c) { handleUserInfo(m, c); };
    handlers_["get_prepared"]         = [this](auto& m, auto& c) { handleGetPrepared(m, c); };
    handlers_["get_unprepared"]       = [this](auto& m, auto& c) { handleGetUnprepared(m, c); };
    handlers_["chat_message"]         = [this](auto& m, auto& c) { handleChatMessage(m, c); };
    handlers_["use_card"]             = [this](auto& m, auto& c) { handleUseCard(m, c); };
    handlers_["update_player_count"]  = [this](auto& m, auto& c) { handleUpdatePlayerCount(m, c); };
    handlers_["update_chess_count"]   = [this](auto& m, auto& c) { handleUpdateChessCount(m, c); };
    handlers_["choose_chess_piece"]   = [this](auto& m, auto& c) { handleChooseChessPiece(m, c); };
    handlers_["discard_card"]         = [this](auto& m, auto& c) { handleDiscardCard(m, c); };
    handlers_["finish_use_card"]      = [this](auto& m, auto& c) { handleFinishUseCard(m, c); };
    handlers_["game_start"]          = [this](auto& m, auto& c) {
        if (game_started_) { std::cout << "游戏已在进行中，忽略重复开始请求" << std::endl; return; }
        GameStart();
    };
    handlers_["back_to_lobby"]       = [this](auto& m, auto& c) { handleBackToLobby(m, c); };
}

// ============================================================
// 卡牌时机校验
// ============================================================

bool FlychessServer::validateCardTiming(int function_time,
                                         flychess_game::PlayerState current_state) {
    using ft = game_utils::CardFunctionTime;
    using ps = flychess_game::PlayerState;

    switch (static_cast<game_utils::CardFunctionTime>(function_time)) {
    case ft::ANYTIME:
        return true;
    case ft::BEFORE_ROLL:
        return currentTurnCanUseBeforeRollCard();
    case ft::BEFORE_MOVE:
        return current_state == ps::ROLLING || current_state == ps::SELECTING;
    case ft::AFTER_MOVE:
        return currentTurnCanUseAfterMoveCard();
    case ft::YOUR_TURN:
        return current_state == ps::ROLLING
            || current_state == ps::SELECTING
            || currentTurnCanUseAfterMoveCard();
    default:
        return false;
    }
}

// ============================================================
// 消息处理器实现
// ============================================================

void FlychessServer::handleRollDice(const nlohmann::json& /*msg*/,
                                     const std::string& client_id) {
    std::cout << "rolldice request from player " << client_id << std::endl;

    const int web_id = std::stoi(client_id);
    auto player = game_room_->getPlayerByWebId(web_id);
    int color = static_cast<int>(player.color);

    if (!isCurrentPlayersTurn(web_id) || !currentTurnCanRoll()) {
        std::cout << "[RollDice] 忽略非法掷骰请求, color=" << color
                  << " player_to_move_=" << player_to_move_
                  << " game_state_=" << static_cast<int>(game_state_)
                  << std::endl;
        return;
    }

    int steps = game_->RollDice();
    player_to_move_ = color;

    sendDiceNum(steps, client_id);

    if (steps == 5) {
        int available_cards_count = static_cast<int>(card_config_.size());
        int card_id = game_utils::get_random(0, available_cards_count - 1);
        std::cout << "抽到卡牌: " << card_id << std::endl;
        player_hands_[color].push_back(card_id);
        sendHandState(color);
    }

    if (steps != 6 && game_->GetStartedChessCount(color) == 0) {
        nlohmann::json no_piece_msg;
        no_piece_msg["type"] = "no_avialable_piece";
        no_piece_msg["color"] = color;
        no_piece_msg["dice_num"] = steps;
        this->sendToClient(client_id, no_piece_msg.dump());
        return;
    }

    enterSelectingPhase(color, client_id, steps);
}

void FlychessServer::handleFinishTextWaiting(const nlohmann::json& /*msg*/,
                                              const std::string& client_id) {
    auto player = game_room_->getPlayerByWebId(std::stoi(client_id));
    int color = static_cast<int>(player.color);

    if (!isCurrentPlayersTurn(std::stoi(client_id))) {
        return;
    }

    enterAfterMoveCardOrNextTurn(color, client_id);
}

void FlychessServer::handleUserInfo(const nlohmann::json& msg,
                                     const std::string& client_id) {
    // 房间满员检查
    if (game_room_->getPlayerCount() >= game_room_->getMaxPlayerCount()) {
        std::cout << "[UserInfo] 房间已满，拒绝客户端 [" << client_id << "]" << std::endl;
        nlohmann::json reject_msg;
        reject_msg["type"] = "room_full";
        reject_msg["reason"] = "房间已满，无法加入";
        this->sendToClient(client_id, reject_msg.dump());
        return;
    }

    std::string user_name = msg.at("name");
    int color = game_room_->getPlayerCount();
    game_room_->addPlayer(flychess_game::PlayerInfo(
        static_cast<game_utils::Color>(color), "player", std::stoi(client_id)));

    nlohmann::json ret_msg;
    ret_msg["type"] = "add_player_broadcast";
    ret_msg["color"] = std::to_string(color);
    ret_msg["name"] = user_name;
    this->BroadCast(ret_msg.dump());

    nlohmann::json register_msg;
    register_msg["type"] = "register_ret";
    register_msg["color"] = std::to_string(color);
    this->sendToClient(client_id, register_msg.dump());

    nlohmann::json url_msg;
    url_msg["type"] = "url";
    url_msg["url"] = this->url_to_show;
    this->sendToClient(client_id, url_msg.dump());

    this->BroadCastPlayerList();
    this->BroadCastRoomInfo();
}

void FlychessServer::handleGetPrepared(const nlohmann::json& /*msg*/,
                                        const std::string& client_id) {
    this->game_room_->setPrepared(std::stoi(client_id));
    BroadCastPlayerList();
}

void FlychessServer::handleGetUnprepared(const nlohmann::json& /*msg*/,
                                          const std::string& client_id) {
    this->game_room_->setUnPrepared(std::stoi(client_id));
    BroadCastPlayerList();
}

void FlychessServer::handleChatMessage(const nlohmann::json& msg,
                                        const std::string& client_id) {
    std::string chat_str = msg.at("message");

    nlohmann::json broadcast;
    broadcast["type"] = "chat_message_broadcast";
    broadcast["chat_msg"] = chat_str;
    broadcast["name"] = game_room_->getPlayerByWebId(std::stoi(client_id)).player_name;
    broadcast["color"] = static_cast<int>(
        game_room_->getPlayerByWebId(std::stoi(client_id)).color);
    this->BroadCast(broadcast.dump());
}

void FlychessServer::handleUseCard(const nlohmann::json& msg,
                                    const std::string& client_id) {
    const int web_id = std::stoi(client_id);
    auto player = game_room_->getPlayerByWebId(web_id);
    int caster_color = static_cast<int>(player.color);

    if (!isCurrentPlayersTurn(web_id)) {
        std::cout << "[UseCard] 忽略非当前玩家请求, color=" << caster_color
                  << " player_to_move_=" << player_to_move_ << std::endl;
        return;
    }
    if (needsDiscardChoice(caster_color)) {
        nlohmann::json reject_msg;
        reject_msg["type"] = "card_rejected";
        reject_msg["reason"] = "请先弃牌到 5 张";
        sendToClient(client_id, reject_msg.dump());
        sendDiscardPrompt(caster_color);
        return;
    }

    int card_to_use_id = msg.at("card_id");
    auto& hand = player_hands_[caster_color];
    auto hand_it = std::find(hand.begin(), hand.end(), card_to_use_id);
    if (hand_it == hand.end()) {
        nlohmann::json reject_msg;
        reject_msg["type"] = "card_rejected";
        reject_msg["reason"] = "手牌中没有这张卡";
        reject_msg["card_id"] = card_to_use_id;
        sendToClient(client_id, reject_msg.dump());
        return;
    }
    int target_player_color = msg.value("target_id", -1);
    int target_piece_id = msg.value("piece_id", -1);

    nlohmann::json card_json = flychess_game::find_card_by_id(card_config_, card_to_use_id);
    if (card_json.is_null()) {
        std::cerr << "[UseCard] 卡牌不存在: id=" << card_to_use_id << std::endl;
        return;
    }

    int function_time = card_json.value("function_time", 0);
    if (!validateCardTiming(function_time, game_state_)) {
        std::cerr << "[UseCard] 卡牌使用时机不正确: " << card_json["name"]
                  << " function_time=" << function_time
                  << " game_state=" << static_cast<int>(game_state_) << std::endl;
        nlohmann::json reject_msg;
        reject_msg["type"] = "card_rejected";
        reject_msg["reason"] = "时机不正确";
        reject_msg["card_id"] = card_to_use_id;
        sendToClient(client_id, reject_msg.dump());
        return;
    }

    const auto state_before_use = game_state_;
    int target_selection = card_json.value("target_selection", 0);
    if (target_selection >= 1 && target_player_color < 0) {
        std::cerr << "[UseCard] 卡牌需要目标但未提供" << std::endl;
        nlohmann::json reject_msg;
        reject_msg["type"] = "card_rejected";
        reject_msg["reason"] = "缺少目标";
        reject_msg["card_id"] = card_to_use_id;
        sendToClient(client_id, reject_msg.dump());
        return;
    }
    if (!isCardTargetValid(card_json, caster_color, target_player_color, target_piece_id)) {
        std::cerr << "[UseCard] 目标不合法" << std::endl;
        nlohmann::json reject_msg;
        reject_msg["type"] = "card_rejected";
        reject_msg["reason"] = "目标不合法";
        reject_msg["card_id"] = card_to_use_id;
        sendToClient(client_id, reject_msg.dump());
        return;
    }

    const bool sets_dice = cardSetsDice(card_json);

    std::cout << "[UseCard] " << card_json["name"]
              << " caster=" << caster_color
              << " target=" << target_player_color
              << " piece=" << target_piece_id << std::endl;

    auto effect = flychess_game::parse_card_effect(
        card_json, caster_color, target_player_color, target_piece_id);

    resolution_stack_.pushEffect(effect);
    resolution_stack_.resolveAll(*game_);
    BroadCastPieceInfo(game_room_->getPlayerCount(), game_room_->getChessPerPlayer());

    hand.erase(hand_it);
    sendHandState(caster_color);
    if (needsDiscardChoice(caster_color)) {
        sendDiscardPrompt(caster_color);
    }

    nlohmann::json used_msg;
    used_msg["type"] = "card_used";
    used_msg["card_id"] = card_to_use_id;
    used_msg["player_color"] = caster_color;
    this->sendToClient(client_id, used_msg.dump());

    if (!sets_dice) {
        return;
    }

    if (needsDiscardChoice(caster_color)) {
        return;
    }

    int dice_val = game_->GetDice();
    if (dice_val < 1 || dice_val > 6) {
        return;
    }

    if (state_before_use == flychess_game::PlayerState::CARDING) {
        if (dice_val == 6) {
            pendingExtraRoll_ = true;
            pendingExtraRollColor_ = caster_color;
            return;
        }

        if (game_->GetStartedChessCount(caster_color) == 0) {
            return;
        }

        clearPendingDice();
        return;
    }

    if (state_before_use == flychess_game::PlayerState::ROLLING) {
        sendDiceNum(dice_val, client_id);
        resolveTurnContinuation(caster_color, client_id, state_before_use);
    }
}

void FlychessServer::handleUpdatePlayerCount(const nlohmann::json& msg,
                                              const std::string& client_id) {
    int num = msg.at("new_p_num");
    bool ret = this->game_room_->setPlayerCount(num);

    if (ret) {
        this->BroadCastPlayerCount();
    } else {
        nlohmann::json failed_ret;
        failed_ret["type"] = "failed_pc_update";
        failed_ret["reason"] = game_room_->getPlayerCount();
        this->BroadCastPlayerCount();
        this->sendToClient(client_id, failed_ret.dump());
    }
}

void FlychessServer::handleUpdateChessCount(const nlohmann::json& msg,
                                             const std::string& /*client_id*/) {
    int num = msg.at("new_c_num");
    this->game_room_->setChessCount(num);
    this->BroadCastChessCount();
}

void FlychessServer::handleChooseChessPiece(const nlohmann::json& msg,
                                             const std::string& client_id) {
    const int web_id = std::stoi(client_id);
    auto player = game_room_->getPlayerByWebId(web_id);
    int color = static_cast<int>(player.color);

    if (!isCurrentPlayersTurn(web_id) || game_state_ != flychess_game::PlayerState::SELECTING) {
        std::cout << "[ChoosePiece] 忽略非法选子请求, color=" << color
                  << " player_to_move_=" << player_to_move_
                  << " game_state_=" << static_cast<int>(game_state_)
                  << std::endl;
        return;
    }

    int id = msg.at("id");
    handleChooseChessPieceInternal(color, id, client_id);
}

void FlychessServer::handleDiscardCard(const nlohmann::json& msg,
                                       const std::string& client_id) {
    const int web_id = std::stoi(client_id);
    auto player = game_room_->getPlayerByWebId(web_id);
    int color = static_cast<int>(player.color);

    if (!isCurrentPlayersTurn(web_id) || !needsDiscardChoice(color)) {
        nlohmann::json reject_msg;
        reject_msg["type"] = "discard_rejected";
        reject_msg["reason"] = "当前不需要弃牌";
        sendToClient(client_id, reject_msg.dump());
        return;
    }

    int card_id = msg.at("card_id");
    if (!discardCardFromHand(color, card_id)) {
        nlohmann::json reject_msg;
        reject_msg["type"] = "discard_rejected";
        reject_msg["reason"] = "无法弃掉这张卡";
        reject_msg["card_id"] = card_id;
        sendToClient(client_id, reject_msg.dump());
        return;
    }

    sendHandState(color);
    if (needsDiscardChoice(color)) {
        sendDiscardPrompt(color);
        return;
    }

    enterPendingDiscardOrNextTurn(color, client_id);
}

void FlychessServer::handleFinishUseCard(const nlohmann::json& msg,
                                          const std::string& client_id) {
    const int web_id = std::stoi(client_id);
    int color = msg.at("color");

    if (!isCurrentPlayersTurn(web_id) || !currentTurnCanUseAfterMoveCard()) {
        std::cout << "[FinishUseCard] 忽略非法结束出牌请求, color=" << color
                  << " player_to_move_=" << player_to_move_
                  << " game_state_=" << static_cast<int>(game_state_)
                  << std::endl;
        return;
    }
    if (needsDiscardChoice(color)) {
        sendDiscardPrompt(color);
        return;
    }

    if (pendingExtraRoll_ && pendingExtraRollColor_ == color) {
        pendingExtraRoll_ = false;
        pendingExtraRollColor_ = -1;
        enterRollingPhase(color);
        return;
    }

    if (this->game_->getPlayerState(color) != flychess_game::PlayerState::FINISHED) {
        game_->changePlayerState(static_cast<game_utils::Color>(color),
                                  flychess_game::PlayerState::OTHERS);
    }

    if (finished_players.size() == game_room_->getPlayerCount()) {
        delete game_;
        game_ = nullptr;
        game_started_ = false;
        finished_players.clear();
        return;
    }

    advanceToNextPlayer(color);
}

void FlychessServer::handleBackToLobby(const nlohmann::json& /*msg*/,
                                        const std::string& /*client_id*/) {
    // 玩家从结算界面返回大厅
    if (game_) {
        delete game_;
        game_ = nullptr;
    }
    game_started_ = false;
    finished_players.clear();
    player_hands_.clear();
    resolution_stack_.clear();
    game_state_ = flychess_game::PlayerState::UNDEFINED;
    player_to_move_ = -1;
    std::cout << "[BackToLobby] 游戏已清理，等待新游戏" << std::endl;
}

// ============================================================
// 回合推进（提取公共逻辑）
// ============================================================

void FlychessServer::advanceToNextPlayer(int current_color) {
    if (game_->getPlayerState(current_color) != flychess_game::PlayerState::FINISHED) {
        game_->changePlayerState(static_cast<game_utils::Color>(current_color),
                                  flychess_game::PlayerState::OTHERS);
    }

    if (finished_players.size() == game_room_->getPlayerCount()) {
        delete game_;
        game_ = nullptr;
        game_started_ = false;
        finished_players.clear();
        return;
    }
    int next_color = current_color;
    while (true) {
        next_color = (next_color + 1) % game_room_->getMaxPlayerCount();
        if (game_->getPlayerState(next_color) != flychess_game::PlayerState::FINISHED) {
            enterRollingPhase(next_color);
            return;
        }
    }
}

// ============================================================
// 通信辅助
// ============================================================

void FlychessServer::sendDiceNum(int dice_result, const std::string& client_id) {
    nlohmann::json message_json;
    message_json["type"] = "dice_result";
    message_json["dice_result"] = dice_result;
    message_json["player_color"] = game_room_->getPlayerByWebId(std::stoi(client_id)).color;
    message_json["name"] = game_room_->getPlayerByWebId(std::stoi(client_id)).player_name;

    if (server_) {
        BroadCast(message_json.dump());
    } else {
        std::cerr << "server异常" << std::endl;
    }
}

void FlychessServer::sendHandState(int color) {
    if (color < 0 || color >= game_room_->getPlayerCount()) {
        return;
    }
    const auto& player_info = game_room_->getPlayer(color);
    const auto hand_it = player_hands_.find(color);
    const auto& hand = hand_it == player_hands_.end() ? std::vector<int>{} : hand_it->second;

    nlohmann::json hand_msg;
    hand_msg["type"] = "hand_state";
    hand_msg["cards"] = nlohmann::json::array();
    for (int card_id : hand) {
        auto card_json = flychess_game::find_card_by_id(card_config_, card_id);
        if (!card_json.is_null()) {
            hand_msg["cards"].push_back(card_json);
        }
    }
    sendToClient(std::to_string(player_info.websocket_id), hand_msg.dump());
}

void FlychessServer::sendDiscardPrompt(int color) {
    if (color < 0 || color >= game_room_->getPlayerCount()) {
        return;
    }
    const auto& player_info = game_room_->getPlayer(color);
    nlohmann::json prompt_msg;
    prompt_msg["type"] = "discard_required";
    prompt_msg["max_hand_size"] = 5;
    prompt_msg["current_count"] = player_hands_[color].size();
    sendToClient(std::to_string(player_info.websocket_id), prompt_msg.dump());
}

void FlychessServer::sendToClient(const std::string& client_id, const std::string msg) {
    auto it = clients_.find(client_id);
    if (it != clients_.end() && it->second->getReadyState() == ix::ReadyState::Open) {
        it->second->send(msg);
        std::cout << "消息已发送给客户端 [" << client_id << "]:" << msg << std::endl;
    } else {
        std::cerr << "Client [" << client_id << "] not found or not connected." << std::endl;
    }
}

void FlychessServer::BroadCast(const std::string& msg) {
    for (const auto& [id, socket] : clients_) {
        if (socket->getReadyState() == ix::ReadyState::Open) {
            socket->send(msg);
        } else {
            std::cerr << "Client [" << id << "] is not connected." << std::endl;
        }
    }
}

// ============================================================
// 广播辅助
// ============================================================

void FlychessServer::BroadCastPlayerCount() {
    nlohmann::json update_pc_ret;
    update_pc_ret["type"] = "update_pc_ret";
    update_pc_ret["new_count"] = game_room_->getMaxPlayerCount();
    this->BroadCast(update_pc_ret.dump());
}

void FlychessServer::BroadCastChessCount() {
    nlohmann::json update_cc_ret;
    update_cc_ret["type"] = "update_cc_ret";
    update_cc_ret["new_count"] = game_room_->getChessPerPlayer();
    this->BroadCast(update_cc_ret.dump());
}

void FlychessServer::BroadCastRoomInfo() {
    this->BroadCastPlayerCount();
    this->BroadCastChessCount();
}

void FlychessServer::CardState(const std::string client_id) {
    nlohmann::json to_use_card_msg;
    to_use_card_msg["type"] = "to_use_card";
    to_use_card_msg["phase"] = currentTurnCanUseBeforeRollCard() ? "before_roll" : "after_move";
    this->sendToClient(client_id, to_use_card_msg.dump());
}

void FlychessServer::sendChoosePieceState(const std::string& client_id, int color, int dice, bool auto_selected) {
    nlohmann::json choose_msg;
    choose_msg["type"] = "to_choose_piece";
    choose_msg["color"] = color;
    choose_msg["dice"] = dice;
    choose_msg["auto_selected"] = auto_selected;
    this->sendToClient(client_id, choose_msg.dump());
}

void FlychessServer::enterRollingPhase(int color) {
    game_->changePlayerState(static_cast<game_utils::Color>(color),
                              flychess_game::PlayerState::ROLLING);
    game_state_ = flychess_game::PlayerState::ROLLING;
    BroadCastToRollDice(color);
}

void FlychessServer::enterSelectingPhase(int color, const std::string& client_id, int dice) {
    game_->changePlayerState(static_cast<game_utils::Color>(color),
                              flychess_game::PlayerState::SELECTING);
    game_state_ = flychess_game::PlayerState::SELECTING;

    if (tryAutoChoosePiece(color, client_id, dice)) {
        return;
    }

    sendChoosePieceState(client_id, color, dice, false);
}

void FlychessServer::enterAfterMoveCardOrNextTurn(int color, const std::string& client_id) {
    int steps = game_->GetDice();
    if (!shouldEnterCardPhaseAfterMove(color, steps)) {
        enterPendingDiscardOrNextTurn(color, client_id);
        return;
    }

    if (!hasAnyUsableCardForPhase(color, "after_move")) {
        enterPendingDiscardOrNextTurn(color, client_id);
        return;
    }

    game_->changePlayerState(static_cast<game_utils::Color>(color),
                              flychess_game::PlayerState::CARDING);
    game_state_ = flychess_game::PlayerState::CARDING;
    CardState(client_id);
}

void FlychessServer::enterPendingDiscardOrNextTurn(int color, const std::string& client_id) {
    if (needsDiscardChoice(color)) {
        game_->changePlayerState(static_cast<game_utils::Color>(color),
                                  flychess_game::PlayerState::CARDING);
        game_state_ = flychess_game::PlayerState::CARDING;
        sendDiscardPrompt(color);
        return;
    }

    if (pendingExtraRoll_ && pendingExtraRollColor_ == color) {
        pendingExtraRoll_ = false;
        pendingExtraRollColor_ = -1;
        enterRollingPhase(color);
        return;
    }

    if (this->game_->getPlayerState(color) != flychess_game::PlayerState::FINISHED) {
        game_->changePlayerState(static_cast<game_utils::Color>(color),
                                  flychess_game::PlayerState::OTHERS);
    }

    advanceToNextPlayer(color);
}

void FlychessServer::resolveTurnContinuation(int color, const std::string& client_id,
                                             flychess_game::PlayerState state_before_action) {
    const int dice = game_->GetDice();

    if (state_before_action == flychess_game::PlayerState::ROLLING) {
        if (shouldSkipSelecting(color, dice)) {
            enterAfterMoveCardOrNextTurn(color, client_id);
            return;
        }
        enterSelectingPhase(color, client_id, dice);
        return;
    }

    if (state_before_action == flychess_game::PlayerState::SELECTING) {
        if (dice == 6) {
            enterRollingPhase(color);
            return;
        }
        enterAfterMoveCardOrNextTurn(color, client_id);
    }
}

bool FlychessServer::shouldEnterCardPhaseAfterMove(int color, int steps) const {
    if (steps == 6) {
        return false;
    }
    return game_->GetStartedChessCount(color) >= 0;
}

bool FlychessServer::hasAnyUsableCardForPhase(int color, const std::string& phase) const {
    auto saved_state = game_state_;
    auto saved_turn = player_to_move_;

    const_cast<FlychessServer*>(this)->player_to_move_ = color;
    const_cast<FlychessServer*>(this)->game_state_ =
        (phase == "before_roll") ? flychess_game::PlayerState::ROLLING : flychess_game::PlayerState::CARDING;

    const auto hand_it = player_hands_.find(color);
    if (hand_it == player_hands_.end() || hand_it->second.empty()) {
        const_cast<FlychessServer*>(this)->game_state_ = saved_state;
        const_cast<FlychessServer*>(this)->player_to_move_ = saved_turn;
        return false;
    }

    for (int card_id : hand_it->second) {
        const auto card_json = flychess_game::find_card_by_id(card_config_, card_id);
        if (card_json.is_null()) {
            continue;
        }

        int function_time = card_json.value("function_time", 0);
        if (!const_cast<FlychessServer*>(this)->validateCardTiming(function_time, game_state_)) {
            continue;
        }

        int target_selection = card_json.value("target_selection", 0);
        if (target_selection <= 0) {
            const_cast<FlychessServer*>(this)->game_state_ = saved_state;
            const_cast<FlychessServer*>(this)->player_to_move_ = saved_turn;
            return true;
        }

        const int chess_count = game_room_->getChessPerPlayer();
        const int player_count = game_room_->getPlayerCount();
        for (int target_color = 0; target_color < player_count; ++target_color) {
            for (int piece_id = 0; piece_id < chess_count; ++piece_id) {
                if (isCardTargetValid(card_json, color, target_color, piece_id)) {
                    const_cast<FlychessServer*>(this)->game_state_ = saved_state;
                    const_cast<FlychessServer*>(this)->player_to_move_ = saved_turn;
                    return true;
                }
            }
        }
    }

    const_cast<FlychessServer*>(this)->game_state_ = saved_state;
    const_cast<FlychessServer*>(this)->player_to_move_ = saved_turn;
    return false;
}

bool FlychessServer::isCardTargetValid(const nlohmann::json& card_json, int caster_color,
                                       int target_player_color, int target_piece_id) const {
    const int target_selection = card_json.value("target_selection", 0);
    if (target_selection <= 0) {
        return target_player_color < 0 && target_piece_id < 0;
    }

    if (target_player_color < 0 || target_player_color >= game_room_->getPlayerCount()) {
        return false;
    }
    if (target_piece_id < 0 || target_piece_id >= game_room_->getChessPerPlayer()) {
        return false;
    }

    const auto& piece = game_->GetPlayerChess(target_player_color, target_piece_id);
    if (piece.position < 0) {
        return false;
    }

    if (target_selection == 1) {
        return target_player_color == caster_color;
    }
    if (target_selection == 2) {
        return target_player_color != caster_color;
    }
    return true;
}

std::vector<int> FlychessServer::getSelectablePieces(int color, int dice) const {
    std::vector<int> selectable;
    const int chess_count = game_room_->getChessPerPlayer();
    for (int id = 0; id < chess_count; ++id) {
        const auto& piece = game_->GetPlayerChess(color, id);
        if (piece.position == -2) {
            continue;
        }
        if (piece.position >= 0 || dice == 6) {
            selectable.push_back(id);
        }
    }
    return selectable;
}

bool FlychessServer::shouldSkipSelecting(int color, int dice) const {
    return dice != 6 && game_->GetStartedChessCount(color) == 0;
}

bool FlychessServer::tryAutoChoosePiece(int color, const std::string& client_id, int dice) {
    const auto selectable = getSelectablePieces(color, dice);
    if (selectable.empty()) {
        return false;
    }

    if (dice != 6) {
        if (game_->GetStartedChessCount(color) == 1 && selectable.size() == 1) {
            return handleChooseChessPieceInternal(color, selectable.front(), client_id);
        }
        return false;
    }

    const int unfinished = game_room_->getChessPerPlayer() - game_->GetFinishedChessCount(color);
    if (unfinished == 1 && selectable.size() == 1) {
        return handleChooseChessPieceInternal(color, selectable.front(), client_id);
    }

    return false;
}

bool FlychessServer::handleChooseChessPieceInternal(int color, int id, const std::string& client_id) {
    player_to_move_ = color;

    int steps_to_move = this->game_->GetDice();
    int move_ret = this->game_->MoveChessPiece(color, id, steps_to_move);
    if (move_ret == 0) {
        sendChoosePieceState(client_id, color, steps_to_move, false);
        return false;
    }

    BroadCastPieceInfo(game_room_->getPlayerCount(), game_room_->getChessPerPlayer());
    this->game_->FlyChessPiece(color, id);
    BroadCastPieceInfo(game_room_->getPlayerCount(), game_room_->getChessPerPlayer());

    int finished_piece_count = this->game_->GetFinishedChessCount(color);
    if (finished_piece_count == this->game_room_->getChessPerPlayer()) {
        this->game_->changePlayerState(static_cast<game_utils::Color>(color),
                                        flychess_game::PlayerState::FINISHED);
        game_state_ = flychess_game::PlayerState::FINISHED;
        if (std::find(finished_players.begin(), finished_players.end(), color) == finished_players.end()) {
            finished_players.push_back(color);
        }

        if (finished_players.size() == game_room_->getPlayerCount()) {
            this->BroadCastAllFinished();
            delete game_;
            game_ = nullptr;
            game_started_ = false;
            finished_players.clear();
            return true;
        }
        BroadCastSomeoneFinished(color);
        advanceToNextPlayer(color);
        return true;
    }

    resolveTurnContinuation(color, client_id, flychess_game::PlayerState::SELECTING);
    return true;
}

bool FlychessServer::cardSetsDice(const nlohmann::json& card_json) const {
    const auto effects_it = card_json.find("effects");
    if (effects_it == card_json.end() || !effects_it->is_array()) {
        return false;
    }
    for (const auto& effect : *effects_it) {
        if (effect.is_object() && effect.value("op", "") == "set_dice") {
            return true;
        }
    }
    return false;
}

void FlychessServer::clearPendingDice() {
    if (game_) {
        game_->setDice(-1);
    }
}

bool FlychessServer::isCurrentPlayersTurn(int client_id) const {
    if (player_to_move_ < 0) {
        return false;
    }
    try {
        return static_cast<int>(game_room_->getPlayerByWebId(client_id).color) == player_to_move_;
    } catch (...) {
        return false;
    }
}

bool FlychessServer::currentTurnCanRoll() const {
    return game_state_ == flychess_game::PlayerState::ROLLING && !pendingExtraRoll_;
}

bool FlychessServer::currentTurnCanUseBeforeRollCard() const {
    return game_state_ == flychess_game::PlayerState::ROLLING;
}

bool FlychessServer::currentTurnCanUseAfterMoveCard() const {
    return game_state_ == flychess_game::PlayerState::CARDING;
}

bool FlychessServer::needsDiscardChoice(int color) const {
    auto it = player_hands_.find(color);
    return it != player_hands_.end() && it->second.size() > 5;
}

bool FlychessServer::discardCardFromHand(int color, int card_id) {
    auto it = player_hands_.find(color);
    if (it == player_hands_.end()) {
        return false;
    }
    auto& hand = it->second;
    auto hand_it = std::find(hand.begin(), hand.end(), card_id);
    if (hand_it == hand.end()) {
        return false;
    }
    hand.erase(hand_it);
    return true;
}

void FlychessServer::BroadCastPlayerList() {
    nlohmann::json player_list_json;
    player_list_json["type"] = "update_player_list";
    int current_player_num = game_room_->getPlayerCount();
    for (int i = 0; i < current_player_num; i++) {
        const auto& player = game_room_->getPlayer(i);
        player_list_json["players"].push_back({
            {"name", player.player_name},
            {"color", static_cast<int>(player.color)},
            {"if_prepared", player.if_prepared}
        });
    }
    this->BroadCast(player_list_json.dump());
}

void FlychessServer::BroadCastAllFinished() {
    nlohmann::json msg;
    msg["type"] = "all_finished";
    for (const auto i : finished_players) {
        auto p_info = game_room_->getPlayer(i);
        msg["rank"].push_back({
            {"color", i},
            {"name", p_info.player_name}
        });
    }
    this->BroadCast(msg.dump());
}

void FlychessServer::BroadCastToRollDice(int color_to_roll) {
    player_to_move_ = color_to_roll;
    pendingExtraRoll_ = false;
    pendingExtraRollColor_ = -1;
    for (int i = 0; i < game_room_->getPlayerCount(); i++) {
        auto p_info = game_room_->getPlayer(i);
        if (static_cast<int>(p_info.color) == color_to_roll) {
            nlohmann::json roll_dice_msg;
            roll_dice_msg["type"] = "to_roll_dice";
            this->sendToClient(std::to_string(p_info.websocket_id), roll_dice_msg.dump());

            // 同时发送出牌阶段，允许 BEFORE_ROLL 卡牌在掷骰前使用
            nlohmann::json card_msg;
            card_msg["type"] = "to_use_card";
            card_msg["phase"] = "before_roll";
            this->sendToClient(std::to_string(p_info.websocket_id), card_msg.dump());

            nlohmann::json roll_dice_broadcast_msg;
            roll_dice_broadcast_msg["type"] = "to_roll_dice_broadcast";
            roll_dice_broadcast_msg["color"] = color_to_roll;
            this->BroadCast(roll_dice_broadcast_msg.dump());
        }
    }
}

void FlychessServer::BroadCastSomeoneFinished(int color_finished) {
    nlohmann::json msg;
    msg["type"] = "someone_finished";
    msg["color"] = color_finished;
    this->BroadCast(msg.dump());
}

void FlychessServer::BroadCastPieceInfo(int player_count, int cp_count) {
    nlohmann::json piece_info_msg;
    piece_info_msg["type"] = "all_piece_info";
    for (int i = 0; i < player_count; i++) {
        for (int j = 0; j < cp_count; j++) {
            auto chess_piece = game_->GetPlayerChess(i, j);
            piece_info_msg["pieces"].push_back({
                {"id", chess_piece.id},
                {"color", static_cast<int>(chess_piece.color)},
                {"position", chess_piece.position},
                {"player_id", i}
            });
        }
    }
    BroadCast(piece_info_msg.dump());
}

// ============================================================
// 游戏流程
// ============================================================

void FlychessServer::GameStart() {
    bool all_prepared = game_room_->ifAllPrepared();
    if (!all_prepared) {
        nlohmann::json not_ready_msg;
        not_ready_msg["type"] = "not_ready";
        this->BroadCast(not_ready_msg.dump());
        return;
    }

    const int player_count = game_room_->getPlayerCount();
    if (player_count != game_room_->getMaxPlayerCount()) {
        nlohmann::json not_enough_msg;
        not_enough_msg["type"] = "not_enough_players";
        this->BroadCast(not_enough_msg.dump());
        return;
    }

    card_config_ = flychess_game::load_card_config("config/card.json");
    if (card_config_.empty()) {
        card_config_ = flychess_game::load_card_config("../../config/card.json");
    }

    game_ = new flychess_game::FlychessGame();
    player_hands_.clear();
    const int cp_count = game_room_->getChessPerPlayer();
    game_->setChessCount(cp_count);

    for (int i = 0; i < player_count; i++) {
        const auto& player_info = game_room_->getPlayer(i);
        game_->AddNewPlayer(player_info.color, cp_count);
        player_hands_[i] = {};
    }

    if (game_->GetPlayerCount() != player_count) {
        nlohmann::json player_count_error_msg;
        player_count_error_msg["type"] = "player_count_error";
        this->BroadCast(player_count_error_msg.dump());
        return;
    }

    BroadCastPieceInfo(player_count, cp_count);

    nlohmann::json start_game_msg;
    start_game_msg["type"] = "game_start";
    this->BroadCast(start_game_msg.dump());

    for (int i = 0; i < player_count; ++i) {
        sendHandState(i);
    }

    this->finished_players.clear();
    resolution_stack_.clear();
    game_started_ = true;
    game_->InitGame();

    int color_to_roll = game_->GetPlayerToRollDice();
    if (color_to_roll != -1 && color_to_roll <= 3) {
        enterRollingPhase(color_to_roll);
    } else {
        std::cerr << "颜色值无效。" << std::endl;
    }
}

}  // namespace flychess_server
