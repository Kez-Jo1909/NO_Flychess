#include "../include/server.h"
#include "card.h"
#include "game.h"
#include "command.h"
#include "resolution_stack.h"
#include <thread>
#include <chrono>

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

    // 预加载卡牌配置（路径相对于工作目录 build/server/）
    card_config_ = flychess_game::load_card_config("../../config/card.json");
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
        return current_state == ps::ROLLING || current_state == ps::CARDING;
    case ft::BEFORE_MOVE:
        return current_state == ps::SELECTING;
    case ft::AFTER_MOVE:
        return current_state == ps::CARDING;
    case ft::YOUR_TURN:
        return current_state == ps::ROLLING
            || current_state == ps::SELECTING
            || current_state == ps::CARDING;
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

    int steps = game_->RollDice();
    auto player = game_room_->getPlayerByWebId(std::stoi(client_id));
    player_to_move_ = static_cast<int>(player.color);

    if (steps == 5) {
        int available_cards_count = static_cast<int>(card_config_.size());
        int card_id = game_utils::get_random(0, available_cards_count - 1);
        std::cout << "抽到卡牌: " << card_id << std::endl;

        nlohmann::json card_msg;
        card_msg["type"] = "get_card";
        card_msg["card_id"] = card_id;
        this->sendToClient(client_id, card_msg.dump());
    }

    // 始终广播骰子结果
    sendDiceNum(steps, client_id);

    if (steps != 6 && game_->GetStartedChessCount(static_cast<int>(player.color)) == 0) {
        std::cout << "No avialable chess piece" << std::endl;
        nlohmann::json no_avialable_msg;
        no_avialable_msg["type"] = "no_avialable_piece";
        no_avialable_msg["dice_num"] = steps;
        no_avialable_msg["color"] = static_cast<int>(player.color);
        this->sendToClient(client_id, no_avialable_msg.dump());

        // 延迟推进回合（客户端在收到 no_avialable_piece 后可自行等待）
        advanceToNextPlayer(static_cast<int>(player.color));
    } else {
        game_->changePlayerState(player.color, flychess_game::PlayerState::SELECTING);
        game_state_ = flychess_game::PlayerState::SELECTING;
    }
}

void FlychessServer::handleFinishTextWaiting(const nlohmann::json& /*msg*/,
                                              const std::string& client_id) {
    auto player = game_room_->getPlayerByWebId(std::stoi(client_id));
    game_->changePlayerState(player.color, flychess_game::PlayerState::CARDING);
    game_state_ = flychess_game::PlayerState::CARDING;
    this->CardState(client_id);
}

void FlychessServer::handleUserInfo(const nlohmann::json& msg,
                                     const std::string& client_id) {
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
    int card_to_use_id = msg.at("card_id");
    int target_player_color = msg.value("target_id", -1);
    int target_piece_id = msg.value("piece_id", -1);

    auto player = game_room_->getPlayerByWebId(std::stoi(client_id));
    int caster_color = static_cast<int>(player.color);

    // 1. 查找卡牌配置
    nlohmann::json card_json = flychess_game::find_card_by_id(card_config_, card_to_use_id);
    if (card_json.is_null()) {
        std::cerr << "[UseCard] 卡牌不存在: id=" << card_to_use_id << std::endl;
        return;
    }

    // 2. 时机校验
    int function_time = card_json.value("function_time", 0);
    if (!validateCardTiming(function_time, game_state_)) {
        std::cerr << "[UseCard] 卡牌使用时机不正确: " << card_json["name"]
                  << " function_time=" << function_time
                  << " game_state=" << static_cast<int>(game_state_) << std::endl;
        nlohmann::json reject_msg;
        reject_msg["type"] = "card_rejected";
        reject_msg["reason"] = "时机不正确";
        sendToClient(client_id, reject_msg.dump());
        return;
    }

    // 3. 目标校验
    int target_selection = card_json.value("target_selection", 0);
    if (target_selection >= 1 && target_player_color < 0) {
        std::cerr << "[UseCard] 卡牌需要目标但未提供" << std::endl;
        return;
    }

    // 4. DSL → CardEffect → ResolutionStack
    std::cout << "[UseCard] " << card_json["name"]
              << " caster=" << caster_color
              << " target=" << target_player_color
              << " piece=" << target_piece_id << std::endl;

    auto effect = flychess_game::parse_card_effect(
        card_json, caster_color, target_player_color, target_piece_id);

    resolution_stack_.pushEffect(effect);
    resolution_stack_.resolveAll(*game_);

    // 5. 广播结果
    BroadCastPieceInfo(game_room_->getPlayerCount(), game_room_->getChessPerPlayer());

    // 6. 若玩家使用卡牌设置了骰子（如卡牌 6），自动推进到 SELECTING 阶段
    //    ROLLING（掷骰前用）或 CARDING（移动后用）均可触发
    if (game_state_ == flychess_game::PlayerState::ROLLING
        || game_state_ == flychess_game::PlayerState::CARDING) {
        int dice_val = game_->GetDice();
        if (dice_val >= 1 && dice_val <= 6) {
            // 骰子已被卡牌设置，广播骰子结果并进入选棋子阶段
            sendDiceNum(dice_val, client_id);

            if (dice_val != 6 && game_->GetStartedChessCount(caster_color) == 0) {
                // 没有可用的棋子
                nlohmann::json no_avialable_msg;
                no_avialable_msg["type"] = "no_avialable_piece";
                no_avialable_msg["dice_num"] = dice_val;
                no_avialable_msg["color"] = caster_color;
                this->sendToClient(client_id, no_avialable_msg.dump());
                advanceToNextPlayer(caster_color);
            } else {
                game_->changePlayerState(static_cast<game_utils::Color>(caster_color),
                                          flychess_game::PlayerState::SELECTING);
                game_state_ = flychess_game::PlayerState::SELECTING;
            }
        }
    }

    // 7. 检查可用棋子（SELECTING 状态下）
    if (game_state_ == flychess_game::PlayerState::SELECTING) {
        int steps = game_->GetDice();
        if (steps != 6 && game_->GetStartedChessCount(caster_color) == 0) {
            nlohmann::json no_avialable_msg;
            no_avialable_msg["type"] = "no_avialable_piece";
            no_avialable_msg["dice_num"] = steps;
            no_avialable_msg["color"] = caster_color;
            this->sendToClient(client_id, no_avialable_msg.dump());
        }
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
    int id = msg.at("id");
    int color = msg.at("color");

    player_to_move_ = -1;

    int steps_to_move = this->game_->GetDice();
    int move_ret = this->game_->MoveChessPiece(color, id, steps_to_move);
    if (move_ret == 0) {
        sendDiceNum(steps_to_move, client_id);
        return;
    }

    BroadCastPieceInfo(game_room_->getPlayerCount(), game_room_->getChessPerPlayer());
    int ret = this->game_->FlyChessPiece(color, id);
    BroadCastPieceInfo(game_room_->getPlayerCount(), game_room_->getChessPerPlayer());

    int finished_piece_count = this->game_->GetFinishedChessCount(color);
    if (finished_piece_count == this->game_room_->getChessPerPlayer()) {
        this->game_->changePlayerState(static_cast<game_utils::Color>(color),
                                        flychess_game::PlayerState::FINISHED);
        game_state_ = flychess_game::PlayerState::FINISHED;
        finished_players.push_back(color);

        if (finished_players.size() == game_room_->getPlayerCount()) {
            this->BroadCastAllFinished();
            // 清理游戏状态，允许玩家返回大厅
            delete game_;
            game_ = nullptr;
            game_started_ = false;
            finished_players.clear();
            return;
        }
        BroadCastSomeoneFinished(color);
    } else {
        int last_steps = this->game_->GetDice();
        if (last_steps == 6) {
            game_->changePlayerState(static_cast<game_utils::Color>(color),
                                      flychess_game::PlayerState::ROLLING);
            game_state_ = flychess_game::PlayerState::ROLLING;
            BroadCastToRollDice(color);
            return;
        }
        game_->changePlayerState(static_cast<game_utils::Color>(color),
                                  flychess_game::PlayerState::CARDING);
        game_state_ = flychess_game::PlayerState::CARDING;
        this->CardState(client_id);
    }
}

void FlychessServer::handleFinishUseCard(const nlohmann::json& msg,
                                          const std::string& /*client_id*/) {
    int color = msg.at("color");
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

    int next_color = color;
    while (true) {
        next_color = (next_color + 1) % game_room_->getMaxPlayerCount();
        if (game_->getPlayerState(next_color) != flychess_game::PlayerState::FINISHED) {
            game_->changePlayerState(static_cast<game_utils::Color>(next_color),
                                      flychess_game::PlayerState::ROLLING);
            game_state_ = flychess_game::PlayerState::ROLLING;
            BroadCastToRollDice(next_color);
            return;
        }
    }
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
    resolution_stack_.clear();
    game_state_ = flychess_game::PlayerState::UNDEFINED;
    player_to_move_ = -1;
    std::cout << "[BackToLobby] 游戏已清理，等待新游戏" << std::endl;
}

// ============================================================
// 回合推进（提取公共逻辑）
// ============================================================

void FlychessServer::advanceToNextPlayer(int current_color) {
    // 当前玩家设为 OTHERS
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
            game_->changePlayerState(static_cast<game_utils::Color>(next_color),
                                      flychess_game::PlayerState::ROLLING);
            game_state_ = flychess_game::PlayerState::ROLLING;
            BroadCastToRollDice(next_color);
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
    to_use_card_msg["phase"] = "after_move";  // CARDING 阶段 = 移动后
    this->sendToClient(client_id, to_use_card_msg.dump());
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
    for (int i = 0; i < game_room_->getPlayerCount(); i++) {
        auto p_info = game_room_->getPlayer(i);
        if (static_cast<int>(p_info.color) == color_to_roll) {
            nlohmann::json roll_dice_msg;
            roll_dice_msg["type"] = "to_roll_dice";
            this->sendToClient(std::to_string(p_info.websocket_id), roll_dice_msg.dump());

            // 同时发送出牌阶段，允许 BEFORE_ROLL 卡牌在掷骰前使用
            nlohmann::json card_msg;
            card_msg["type"] = "to_use_card";
            card_msg["phase"] = "before_roll";  // 标记为掷骰前阶段
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

    // 重新加载卡牌配置（改 card.json 后下次开局即生效）
    card_config_ = flychess_game::load_card_config("../../config/card.json");

    game_ = new flychess_game::FlychessGame();
    const int cp_count = game_room_->getChessPerPlayer();
    game_->setChessCount(cp_count);

    for (int i = 0; i < player_count; i++) {
        const auto& player_info = game_room_->getPlayer(i);
        game_->AddNewPlayer(player_info.color, cp_count);
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

    this->finished_players.clear();
    resolution_stack_.clear();
    game_started_ = true;
    game_->InitGame();

    int color_to_roll = game_->GetPlayerToRollDice();
    if (color_to_roll != -1 && color_to_roll <= 3) {
        BroadCastToRollDice(color_to_roll);
    } else {
        std::cerr << "颜色值无效。" << std::endl;
    }
}

}  // namespace flychess_server
