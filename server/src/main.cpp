#include "../include/server.h"

int main() {
    flychess_server::FlychessServer server(8080);

    if (!server.start())
    {
        return 1;
    }

    // 主线程保持运行
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}