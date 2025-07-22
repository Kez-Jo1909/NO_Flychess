#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>

int main()
{
    ix::WebSocketServer server(8080);

    server.setOnConnectionCallback(
        [](std::weak_ptr<ix::WebSocket> weakWebSocket,
           std::shared_ptr<ix::ConnectionState> connectionState) {
            std::cout << "New connection" << std::endl;

            // 必须 lock 才能变成 shared_ptr 使用
            if (auto webSocket = weakWebSocket.lock())
            {
                webSocket->setOnMessageCallback(
                    [webSocket](const ix::WebSocketMessagePtr& msg) {
                        if (msg->type == ix::WebSocketMessageType::Message)
                        {
                            std::cout << "Received: " << msg->str << std::endl;
                            webSocket->send(msg->str);  // Echo 回去
                        }
                    });
            }
        });

    auto res = server.listen();
    if (!res.first)
    {
        std::cerr << "Listen failed: " << res.second << std::endl;
        return 1;
    }

    server.start();
    std::cout << "Server started on ws://localhost:8080" << std::endl;

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));
}
