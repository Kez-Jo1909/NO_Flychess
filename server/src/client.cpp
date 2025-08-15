#include "../include/client.h"

namespace flychess_client {
    FlychessClient::FlychessClient(QObject *parent) : QObject(parent) {

    }

    FlychessClient::~FlychessClient() {
        ws_.stop();
    }

    void FlychessClient::connectToServer(const std::string &url) {
        std::cout<<"调用 connectToServer, url: " << url << std::endl;
        ws_.stop();  // 先停掉旧连接，防止重复


        ws_.setUrl(url);

            // 注册消息回调
        ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
            if (msg->type == ix::WebSocketMessageType::Open)
            {
                // 在回调线程，使用 Qt 的事件系统发信号
                QMetaObject::invokeMethod(this, [this]() {
                    emit connected();
                }, Qt::QueuedConnection);
            }
            else if (msg->type == ix::WebSocketMessageType::Close)
            {
                QMetaObject::invokeMethod(this, [this]() {
                    emit disconnected();
                }, Qt::QueuedConnection);
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                // std::cout << "[DEBUG] msg->str=" << msg->str << " size=" << msg->wireSize << std::endl;
                QString qmsg = QString::fromStdString(msg->str);
                const std::string& msg_text = msg->str;
                if (!msg_text.empty() && (msg_text[0] == '{' || msg_text[0] == '[')) {
                    try {
                        auto j = nlohmann::json::parse(msg->str);
                        std::string type = j.at("type");

                        if (type == "register_ret") {
                            std::string color = j.at("color");
                            this->user_color_ = static_cast<game_utils::Color>(std::stoi(color));

                            // 发信号给 Qt 主线程
                            QMetaObject::invokeMethod(this, [this]() {
                                emit registerResult(this->user_color_);
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "add_player_broadcast") {
                            std::string new_player_name = j.at("name");
                            std::string new_player_color = j.at("color");
                            // game_utils::Color color = static_cast<game_utils::Color>(std::stoi(new_player_color));
                            // std::cout << "[DEBUG] 收到 add_player_broadcast: "
                            //     << new_player_name << " color=" << new_player_color << std::endl;
                            // 发信号给 Qt 主线程
                            QMetaObject::invokeMethod(this, [this, new_player_name, new_player_color]() {
                                emit newPlayerJoined(QString::fromStdString(new_player_name),
                                                     static_cast<game_utils::Color>(std::stoi(new_player_color)));
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "update_player_list") {
                            QList<QVariantList> players;
                            for (auto& player : j["players"]) {
                                std::string name = player.at("name");
                                int colorInt = player.at("color");
                                bool is_ready = player.at("if_prepared");
                                players.append(QVariantList{
                                    QString::fromStdString(name),
                                    colorInt,
                                    is_ready
                                });
                            }
                            // 发信号给 Qt 主线程
                            QMetaObject::invokeMethod(this, [this, players = std::move(players)]() {
                                emit playerListUpdated(players);
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "all_piece_info") {
                            QList<QVariantList> pieces;
                            for(auto& piece : j["pieces"]) {
                                int id = piece.at("id");
                                int color = piece.at("color");
                                int position = piece.at("position");
                                int player_id = piece.at("player_id");
                                pieces.append(QVariantList{
                                    id,
                                    color,
                                    position,
                                    player_id
                                });
                            }
                            // 发信号给 Qt 主线程
                            QMetaObject::invokeMethod(this, [this, pieces = std::move(pieces)]() {
                                emit allPieceInfo(pieces);
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "leave_room"){
                            std::string name = j.at("name");
                            std::string color = j.at("color");

                            QMetaObject::invokeMethod(this, [this, name, color]() {
                                emit playerLeaveRoom(QString::fromStdString(name), 
                                                    static_cast<game_utils::Color>(std::stoi(color)));
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "update_pc_ret") {
                            int new_pc = j.at("new_count");

                            QMetaObject::invokeMethod(this,[this, new_pc](){
                                emit updatePlayerCount(new_pc);
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "failed_pc_update") {
                            int current_min_num = j.at("reason");

                            QMetaObject::invokeMethod(this,[this, current_min_num](){
                                emit updatePlayerCountFailed(current_min_num);
                            }, Qt::QueuedConnection);
                        }
                        else if(type == "update_cc_ret") {
                            int new_cc = j.at("new_count");

                            QMetaObject::invokeMethod(this,[this, new_cc](){
                                emit updateChessCount(new_cc);
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "game_start"){
                            QMetaObject::invokeMethod(this,[this](){
                                emit GameStart();
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "not_ready") {
                            QMetaObject::invokeMethod(this, [this]() {
                                emit GameStartFailed();
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "not_enough_players") {
                            QMetaObject::invokeMethod(this, [this]() {
                                emit GameStartNotEnough();
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "dice_result") {
                            int result = j.at("dice_result");
                            std::string player_name = j.at("name");
                            int player_color = j.at("player_color");

                            QMetaObject::invokeMethod(this, [this, result, player_name, player_color]() {
                                emit rollDiceResult(result, QString::fromStdString(player_name), player_color);
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "to_roll_dice") {
                            QMetaObject::invokeMethod(this, [this]() {
                                emit toRollDice();
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "to_roll_dice_broadcast") {
                            int color = j.at("color");
                            QMetaObject::invokeMethod(this, [this, color]() {
                                emit OtherToRollDice(color);
                            }, Qt::QueuedConnection);
                        }
                        else if (type == "to_use_card") {
                            QMetaObject::invokeMethod(this, [this]() {
                                emit toUseCard();
                            }, Qt::QueuedConnection);
                        }
                        else {
                            std::cout<< "未知消息类型,内容:";
                            std::cout<< msg_text << std::endl;
                            // QMetaObject::invokeMethod(this, [this, msg_text]() {
                            //     emit unknownMessage(QString::fromStdString(msg_text));
                            // }, Qt::QueuedConnection);
                        }
                    }
                    catch(const std::exception& e) {
                        std::cerr << "[JSON Parse Error] " << e.what() << std::endl;
                    }
                }
            }
        });

        ws_.start();
    }

    void FlychessClient::sendMessage(const std::string &msg) {
        ws_.send(msg);
    }

    void FlychessClient::close() {
        ws_.close();
        ws_.stop();
        // std::cout << "客户端主动断开连接" << std::endl;
    }

    void FlychessClient::sendChosenChessPiece(int id, int color) {
        nlohmann::json msg;
        msg["type"] = "choose_chess_piece";
        msg["id"] = id;
        msg["color"] = color;

        ws_.send(msg.dump());
    }

    void FlychessClient::sendFinishUseCard(int color) {
        nlohmann::json msg;
        msg["type"] = "finish_use_card";
        msg["color"] = color;
        ws_.send(msg.dump());
    }

    void FlychessClient::sendPlayerCount(int num) {
        nlohmann::json p_count_msg;
        p_count_msg["type"] = "update_player_count";
        p_count_msg["new_p_num"] = num;

        this->sendMessage(p_count_msg.dump());
    }

    void FlychessClient::sendChessCount(int num) {
        nlohmann::json c_count_msg;
        c_count_msg["type"] = "update_chess_count";
        c_count_msg["new_c_num"] = num;

        this->sendMessage(c_count_msg.dump());
    }

    void FlychessClient::disconnectFromServer() {
        ws_.stop();
    }

    void FlychessClient::sendUserInfo(const std::string user_name) {
        nlohmann::json msg;
        msg["type"] = "userInfo";
        msg["name"] = user_name;

        ws_.send(msg.dump());
    }

    void FlychessClient::sendGetPrepared() {
        nlohmann::json msg;
        msg["type"] = "get_prepared";
        ws_.send(msg.dump());
    }

    void FlychessClient::sendGetUnPrepared() {
        nlohmann::json msg;
        msg["type"] = "get_unprepared";
        ws_.send(msg.dump());
    }

    void FlychessClient::sendRollDiceRequest() {
        nlohmann::json msg;
        msg["type"] = "rolldice";
        ws_.send(msg.dump());
    }

}