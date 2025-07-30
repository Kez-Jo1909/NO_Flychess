#ifndef CLIENT_H
#define CLIENT_H
#pragma once
#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>


namespace flychess_client {

class FlychessClient {
public:
    FlychessClient(const std::string& url);
    ~FlychessClient();

    void connect();
    void disconnect();
    void sendMessage(const std::string& message);
private:
    void handleMessage(const std::string& msg);
private:
    ix::WebSocket websocket_;
    std::string url_;
    std::function<void(const std::string&)> onMessageCallback_;
};

}


#endif