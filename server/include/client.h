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


namespace flychess_client {

class FlychessClient : public QObject {
    Q_OBJECT
public:
    explicit FlychessClient(QObject *parent = nullptr);
    ~FlychessClient();

    void connectToServer(const std::string &url);

    void close();

    void sendMessage(const std::string& message);
signals:
    void connected();
    void disconnected();
    void messageReceived(QString msg);  // 发给 Qt 的信号
private:
    ix::WebSocket ws_;
};

}


#endif