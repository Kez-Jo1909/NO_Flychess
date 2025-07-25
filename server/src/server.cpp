#include "../include/server.h"

namespace flychess_server {

    FlycehssServer::FlycehssServer(int port) : port_(port), server_(std::make_unique<ix::WebSocketServer>(port)) {
        // 初始连接回调
        server_->setOnConnectionCallback(
            [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
            std::shared_ptr<ix::ConnectionState> connectionState)
            {
                std::cout << "New connection received." << std::endl;
                if(connectionState) {
                    std::cout << "Client IP: " << connectionState->getRemoteIp() << std::endl;
                }
                
                // 设置 onMessage 回调
                if (auto webSocket = weakWebSocket.lock()) {
                    setupMessageCallback(webSocket);
                }
            }
        );  
    }

    void FlycehssServer::setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket) {
        webSocket->setOnMessageCallback(
            [this](const ix::WebSocketMessagePtr& msg) {
                handleMessage(msg);
            }
        );
    }

    void FlycehssServer::handleMessage(const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            const std::string& msg_text = msg->str;



            if (!msg_text.empty() && (msg_text[0] == '{' || msg_text[0] == '[')) {
                // json消息
                try {
                    auto j = nlohmann::json::parse(msg->str);

                    std::string type = j.at("type");

                    if (type == "rolldice") {
                        std::cout<< "rolldice request from player " << j["playerId"] << std::endl;
                    }
                    else {
                        std::cout<< "未知消息类型" << std::endl;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "[JSON Parse Error] " << e.what() << std::endl;
                }
            }  else {
                std::cout<<"Received non-json message: " << msg->str << std::endl;
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
