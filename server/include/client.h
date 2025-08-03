#ifndef CLIENT_H
#define CLIENT_H
#pragma once
#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <QObject>
#include <nlohmann/json.hpp>
#include "utils.h"


namespace flychess_client {

class FlychessClient : public QObject {
    Q_OBJECT
public:
    explicit FlychessClient(QObject *parent = nullptr);
    ~FlychessClient();

    void connectToServer(const std::string &url);

    void close();

    void sendMessage(const std::string& message);

    void disconnectFromServer();

    void sendUserInfo(const std::string user_name);
signals:
    void connected();
    void disconnected();
    void messageReceived(QString msg);  // 发给 Qt 的信号
    void registerResult(game_utils::Color color);
    void newPlayerJoined(QString name, game_utils::Color color);
    void unknownMessage(QString msg);
private:
    ix::WebSocket ws_;

    game_utils::Color user_color_;
};

}


#endif