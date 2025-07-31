#include "../include/server.h"
#include "game.h"

namespace flychess_server {

    FlycehssServer::FlycehssServer(int port) : port_(port), server_(std::make_unique<ix::WebSocketServer>(port)) {
        // 初始连接回调
        server_->setOnConnectionCallback(
            [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
            std::shared_ptr<ix::ConnectionState> connectionState)
            {
                std::cout << "New connection received." << std::endl;
                if(connectionState) {
                    std::string client_id = connectionState->getId();  // 获取唯一 ID
                    std::cout << "Client ID: " << client_id << ", IP: " << connectionState->getRemoteIp() << std::endl;
                
                
                    // 设置 onMessage 回调
                    if (auto webSocket = weakWebSocket.lock()) {
                        clients_[client_id] = webSocket;  // 保存映射
                        setupMessageCallback(webSocket, client_id);
                    }
                }
            }
        );  
    }

    void FlycehssServer::stop() {
        if (server_) {
            server_->stop();     // 请求服务器停止
            // server_->wait();     // 等待后台线程安全退出（关键！）
            // server_.reset();     // 释放资源
        }
    }


    void FlycehssServer::setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket, const std::string& client_id) {
        webSocket->setOnMessageCallback(
            [this, client_id](const ix::WebSocketMessagePtr& msg) {
                handleMessage(msg, client_id);
            }
        );
    }

    void FlycehssServer::handleMessage(const ix::WebSocketMessagePtr& msg, const std::string& client_id) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "收到来自ID [" << client_id << "] 的消息: ";
            const std::string& msg_text = msg->str;
            if (!msg_text.empty() && (msg_text[0] == '{' || msg_text[0] == '[')) {
                // json消息
                try {
                    auto j = nlohmann::json::parse(msg->str);

                    std::string type = j.at("type");

                    if (type == "rolldice") {
                        std::cout<< "rolldice request from player " << j["playerId"] << std::endl;
                        int dice_result = flychess_game::rollDice();
                        // std::cout<< "backend roll dice:" << dice_reuslt << std::endl;
                        sendDiceNum(dice_result, client_id);
                    }
                    else {
                        std::cout<< "未知消息类型" << std::endl;
                    }   
                }
                catch (const std::exception& e) {
                    std::cerr << "[JSON Parse Error] " << e.what() << std::endl;
                }
            }  else {
                std::cout<<"non-json message: " << msg->str << std::endl;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "客户端 [" << client_id << "] 断开连接。" << std::endl;
            clients_.erase(client_id);
        }
    }

    void FlycehssServer::sendDiceNum(int dice_result, const std::string& client_id) {
        nlohmann::json message_json;
        message_json["type"] = "dice_result";
        message_json["dice_result"] = dice_result;

        std::string message_str = message_json.dump();
        if (server_) {
            this->sendToClient(client_id, message_str);
        } else {
            std::cerr<< "server异常" << std::endl;
        }
    }

    void FlycehssServer::sendToClient(const std::string& client_id, const std::string msg) {
        auto it = clients_.find(client_id);

        if (it != clients_.end() && it->second->getReadyState() == ix::ReadyState::Open) {
            it->second->send(msg);
            std::cout<<"消息已发送给客户端 [" << client_id << "]:" << msg << std::endl;
        } else {
            std::cerr << "Client [" << client_id << "] not found or not connected." << std::endl;
        }
    }

    // 广播
    void FlycehssServer::BroadCast(const std::string& msg) {
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

    bool FlycehssServer::start() {
        auto res = server_->listen();
        if (!res.first)
        {
            std::cerr << "Listen failed: " << res.second << std::endl;
            return false;
        }

        server_->start();
        std::cout << "WebSocket server started on port " << port_ << std::endl;
        return true;
    }

}
