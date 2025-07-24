#ifndef SERVER_H
#define SERVER_H

#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

namespace flychess_server{

class FlycehssServer {
public:
    //传入端口号，默认8080
    explicit FlycehssServer(int port = 8080);

    // 启动服务器
    bool start();

private:
    int port_;
    std::unique_ptr<ix::WebSocketServer> server_;
};

}

#endif