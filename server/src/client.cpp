#include "../include/client.h"

namespace flychess_client {

FlychessClient::FlychessClient(const std::string& url)
    : url_(url) {
    websocket_.setUrl(url_);
}

FlychessClient::~FlychessClient() {
    disconnect();
}

void FlychessClient::connect() {
    websocket_.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                handleMessage(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "连接成功: " << url_ << std::endl;
            } else if (msg->type == ix::WebSocketMessageType::Error) {
                std::cerr << "连接错误: " << msg->errorInfo.reason << std::endl;
            }
        }
    );

    websocket_.start();
}

void FlychessClient::handleMessage(const std::string& msg) {
    std::cout << "收到消息: " << msg << std::endl;
}

void FlychessClient::disconnect() {
    websocket_.stop();
}

void FlychessClient::sendMessage(const std::string& message) {
    websocket_.sendText(message);
}

}