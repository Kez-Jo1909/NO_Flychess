#include "../include/client.h"

namespace flychess_client {
    FlychessClient::FlychessClient(QObject *parent) : QObject(parent) {

    }

    FlychessClient::~FlychessClient() {
        ws_.stop();
    }

    void FlychessClient::connectToServer(const std::string &url) {
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
                QString qmsg = QString::fromStdString(msg->str);
                QMetaObject::invokeMethod(this, [this, qmsg]() {
                    emit messageReceived(qmsg);
                }, Qt::QueuedConnection);
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
        std::cout << "客户端主动断开连接" << std::endl;
    }

    void FlychessClient::disconnectFromServer() {
        ws_.stop();
    }


// GPT写的示例，先扔在这
//     // 回调线程
// ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
//     if (msg->type == ix::WebSocketMessageType::Message) {
//         // 先在回调线程解析 JSON 或做逻辑处理
//         std::string reply = handleMessage(msg->str);

//         // 最后只把 UI 相关事件投递回 Qt 主线程
//         QString qmsg = QString::fromStdString(reply);
//         QMetaObject::invokeMethod(this, [this, qmsg]() {
//             emit messageReceived(qmsg);
//         }, Qt::QueuedConnection);
//     }
// });

}