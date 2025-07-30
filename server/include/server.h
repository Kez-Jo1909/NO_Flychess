#ifndef SERVER_H
#define SERVER_H

#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include "game.h"

namespace flychess_server{

class FlycehssServer {
public:
    //传入端口号，默认8080
    explicit FlycehssServer(int port = 8080);

    // 启动服务器
    bool start();

    void stop();

    void handleMessage(const ix::WebSocketMessagePtr& msg, const std::string& client_id);

private:
    int port_;
    std::unique_ptr<ix::WebSocketServer> server_;
    std::unordered_map<std::string, std::shared_ptr<ix::WebSocket>> clients_;// client_id -> WebSocket映射

    void setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket, const std::string& client_id);

    void sendDiceNum(int dice_num_, const std::string& client_id);

    void sendToClient(const std::string& client_id, const std::string msg);

    void BroadCast(const std::string& msg);
};

}

#endif