#include "../include/server.h"

namespace flychess_server {

    FlycehssServer::FlycehssServer(int port) : port_(port), server_(std::make_unique<ix::WebSocketServer>(port)) {
        // 初始连接回调
        server_->setOnConnectionCallback(
            [](std::weak_ptr<ix::WebSocket> weakWebSocket,
            std::shared_ptr<ix::ConnectionState> connectionState)
            {
                std::cout << "New connection received." << std::endl;
                if(connectionState) {
                    std::cout << "Client IP: " << connectionState->getRemoteIp() << std::endl;
                }
                
                // 设置 onMessage 回调
                if (auto webSocket = weakWebSocket.lock()) {
                    webSocket->setOnMessageCallback(
                        [](const ix::WebSocketMessagePtr& msg) {
                            if (msg->type == ix::WebSocketMessageType::Message) {
                                std::cout << "Received message: " << msg->str << std::endl;
                            }
                        }
                    );
                }
            }
        );
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
