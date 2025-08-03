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
                std::cout << "[DEBUG] msg->str=" << msg->str << " size=" << msg->wireSize << std::endl;
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

    void FlychessClient::disconnectFromServer() {
        ws_.stop();
    }

    void FlychessClient::sendUserInfo(const std::string user_name) {
        nlohmann::json msg;
        msg["type"] = "userInfo";
        msg["name"] = user_name;

        ws_.send(msg.dump());
    }

}