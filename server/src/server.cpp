    #include "../include/server.h"
    #include <QMetaObject>
    #include <QJsonDocument>
    #include <QJsonObject>
    #include "game.h"

    namespace flychess_server {

    FlychessServer::FlychessServer(int port, QObject* parent)
        : QObject(parent), port_(port), server_(std::make_unique<ix::WebSocketServer>(port)) 
    {
        // 初始连接回调
        server_->setOnConnectionCallback(
            [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
                std::shared_ptr<ix::ConnectionState> connectionState)
            {
                std::cout << "New connection received." << std::endl;
                if(connectionState) {
                    std::string client_id = connectionState->getId();
                    std::string ip = connectionState->getRemoteIp();
                    std::cout << "Client ID: " << client_id << ", IP: " << ip << std::endl;
                
                    if (auto webSocket = weakWebSocket.lock()) {
                        clients_[client_id] = webSocket;  // 保存映射
                        setupMessageCallback(webSocket, client_id);
                    }

                    // 发射 Qt 信号
                    QMetaObject::invokeMethod(this, [this, client_id, ip]() {
                        emit clientConnected(QString::fromStdString(client_id),
                                            QString::fromStdString(ip));
                    }, Qt::QueuedConnection);
                }
            }
        );
        game_room_ = new flychess_game::FlychessGameRoom();
        // game_room_ -> addPlayer(flychess_game::PlayerInfo(game_utils::Color::RED, "player", 0)); 
    }

    void FlychessServer::stop() {
        if (server_) {
            server_->stop();     
        }
    }

    void FlychessServer::setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket, const std::string& client_id) {
        webSocket->setOnMessageCallback(
            [this, client_id](const ix::WebSocketMessagePtr& msg) {
                handleMessage(msg, client_id);
            }
        );
    }

    void FlychessServer::handleMessage(const ix::WebSocketMessagePtr& msg, const std::string& client_id) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "收到来自ID [" << client_id << "] 的消息: ";
            const std::string& msg_text = msg->str;
            if (!msg_text.empty() && (msg_text[0] == '{' || msg_text[0] == '[')) {
                try {
                    auto j = nlohmann::json::parse(msg->str);
                    std::string type = j.at("type");

                    // // 通过 Qt 信号通知
                    // QMetaObject::invokeMethod(this, [this, client_id, j]() {
                    //     QJsonObject obj = QJsonDocument::fromJson(
                    //         QByteArray::fromStdString(j.dump())
                    //     ).object();
                    //     emit jsonReceived(QString::fromStdString(client_id), obj);
                    // }, Qt::QueuedConnection);

                    if (type == "rolldice") {
                        std::cout<< "rolldice request from player " << j["playerId"] << std::endl;
                        int dice_result = flychess_game::rollDice();
                        sendDiceNum(dice_result, client_id);
                    }
                    else if (type == "userInfo") {// 新加入玩家在这里
                        std::string user_name = j.at("name");
                        int color = game_room_->getPlayerCount();
                        game_room_->addPlayer(flychess_game::PlayerInfo(static_cast<game_utils::Color>(color), "player", std::stoi(client_id)));

                        nlohmann::json ret_msg;
                        ret_msg["type"] = "add_player_broadcast";
                        ret_msg["color"] = std::to_string(color);
                        ret_msg["name"] = user_name;
                        this->BroadCast(ret_msg.dump());

                        nlohmann::json register_msg;
                        register_msg["type"] = "register_ret";
                        register_msg["color"] = std::to_string(color);
                        this->sendToClient(client_id, register_msg.dump());
                        // std::cout << "[Server] 广播 add_player_broadcast: " << std::endl;

                        this->BroadCastPlayerList();
                        this->BroadCastRoomInfo();
                    }
                    else if(type == "get_prepared") {
                        this->game_room_->setPrepared(std::stoi(client_id));
                        BroadCastPlayerList();
                    }
                    else if(type == "get_unprepared") {
                        this->game_room_->setUnPrepared(std::stoi(client_id));
                        BroadCastPlayerList();
                    }
                    else if(type == "update_player_count") {
                        int num = j.at("new_p_num");
                        bool ret = this->game_room_->setPlayerCount(num);

                        if(ret) {
                            this->BroadCastPlayerCount();
                        }
                        else {
                            nlohmann::json failed_ret;
                            failed_ret["type"] = "failed_pc_update";
                            failed_ret["reason"] = game_room_->getPlayerCount();
                            this->BroadCastPlayerCount();
                            this->sendToClient(client_id, failed_ret.dump());
                        }
                    }
                    else if(type == "update_chess_count") {
                        int num = j.at("new_c_num");
                        this->game_room_->setChessCount(num);

                        this->BroadCastChessCount();
                    }
                    else {
                        std::cout<< "未知消息类型,内容:";
                        std::cout<< msg_text << std::endl;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "[JSON Parse Error] " << e.what() << std::endl;
                }
            } else {
                std::cout<<"non-json message: " << msg->str << std::endl;

                // 发射普通文本消息信号
                QMetaObject::invokeMethod(this, [this, client_id, msg_text]() {
                    emit messageReceived(QString::fromStdString(client_id),
                                        QString::fromStdString(msg_text));
                }, Qt::QueuedConnection);
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "客户端 [" << client_id << "] 断开连接。" << std::endl;
            clients_.erase(client_id);

            // 广播退出房间
            nlohmann::json leave_msg;
            leave_msg["type"] = "leave_room";
            const auto& player_to_leave_info = game_room_->getPlayerByWebId(std::stoi(client_id));
            int color_value = static_cast<int>(player_to_leave_info.color);
            leave_msg["color"] = std::to_string(color_value);
            leave_msg["name"] = player_to_leave_info.player_name;
            this->BroadCast(leave_msg.dump());

            // 删除房间内玩家信息
            game_room_->DeletePlayer(std::stoi(client_id));
            this->BroadCastPlayerList();

            QMetaObject::invokeMethod(this, [this, client_id]() {
                emit clientDisconnected(QString::fromStdString(client_id));
            }, Qt::QueuedConnection);
        }
    }

    void FlychessServer::sendDiceNum(int dice_result, const std::string& client_id) {
        nlohmann::json message_json;
        message_json["type"] = "dice_result";
        message_json["dice_result"] = dice_result;
        message_json["player_color"] = game_room_->getPlayerByWebId(std::stoi(client_id)).color;
        message_json["name"] = game_room_->getPlayerByWebId(std::stoi(client_id)).player_name;

        if (server_) {
            // this->sendToClient(client_id, message_str);
            BroadCast(message_json.dump());
        } else {
            std::cerr<< "server异常" << std::endl;
        }
    }

    void FlychessServer::sendToClient(const std::string& client_id, const std::string msg) {
        auto it = clients_.find(client_id);

        if (it != clients_.end() && it->second->getReadyState() == ix::ReadyState::Open) {
            it->second->send(msg);
            std::cout<<"消息已发送给客户端 [" << client_id << "]:" << msg << std::endl;
        } else {
            std::cerr << "Client [" << client_id << "] not found or not connected." << std::endl;
        }
    }

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

    void FlychessServer::BroadCast(const std::string& msg) {
        for (const auto& [id, socket] : clients_)
        {
            if (socket->getReadyState() == ix::ReadyState::Open) {
                socket->send(msg);
                std::cout<< "消息已广播给客户端 [" << id << "]:" << msg << std::endl;
            }
            else {
                std::cerr << "Client [" << id << "] is not connected." << std::endl;
            }
        }
    }

    void FlychessServer::BroadCastPlayerList() {
        nlohmann::json player_list_json;
        player_list_json["type"] = "update_player_list";
        int current_player_num = game_room_->getPlayerCount();
        for(int i = 0;  i < current_player_num; i++) {
            const auto& player = game_room_->getPlayer(i);
            player_list_json["players"].push_back({
                {"name", player.player_name},
                {"color", static_cast<int>(player.color)},
                {"if_prepared", player.if_prepared}
            });
        }
        this->BroadCast(player_list_json.dump());
    }


    bool FlychessServer::start() {
        auto res = server_->listen();
        if (!res.first)
        {
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

    // 直接在MainWindow里调用
    void FlychessServer::GameStart() {
        // 先判断是否全部准备
        bool all_prepared = game_room_->ifAllPrepared();
        if  (!all_prepared) {
            // BROADCAST
            nlohmann::json not_ready_msg;
            not_ready_msg["type"] = "not_ready";
            this->BroadCast(not_ready_msg.dump());
        }
        else {
            const int player_count = game_room_->getPlayerCount();
            if(player_count != game_room_->getMaxPlayerCount()) {
                nlohmann::json not_enough_msg;
                not_enough_msg["type"] = "not_enough_players";
                this->BroadCast(not_enough_msg.dump());
                return;
            }
            
            // 创建游戏实例
            game_ = new flychess_game::FlychessGame();
            const int cp_count = game_room_->getChessPerPlayer();
            game_->setChessCount(cp_count);
            // 开始添加玩家
            for(int i = 0; i<player_count; i++) {
                const auto& player_info = game_room_->getPlayer(i);
                game_->AddNewPlayer(player_info.color, cp_count);
            }

            // 检查玩家数量
            if (game_->GetPlayerCount() != player_count) {
                // TODO 发出信号
                nlohmann::json player_count_error_msg;
                player_count_error_msg["type"] = "player_count_error";
                this->BroadCast(player_count_error_msg.dump());
                // std::cerr << "玩家数量不匹配，预期: " << player_count << ", 实际: " << game_->GetPlayerCount() << std::endl;
                return;
            } else {
                // std::cout << "玩家数量检查完成" << std::endl;
                // 发送棋子初始状态
                BroadCastPieceInfo(player_count, cp_count);
            }

            // 发出开始游戏
            nlohmann::json start_game_msg;
            start_game_msg["type"] = "game_start";
            this->BroadCast(start_game_msg.dump());
        }

    }

    void FlychessServer::BroadCastPieceInfo(int player_count, int cp_count) {
        nlohmann::json piece_info_msg;
        piece_info_msg["type"] = "all_piece_info";
        for(int i = 0; i < player_count; i++) {
            for(int j = 0; j < cp_count; j++) {
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

}// flychess_server
