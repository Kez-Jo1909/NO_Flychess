#include "../include/server.h"
#include <thread>
#include <chrono>
#include <ixwebsocket/IXNetSystem.h>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    // Windows 控制台 UTF-8 编码
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    // Windows 上 ixwebsocket 需要先初始化网络系统（WSAStartup）
    ix::initNetSystem();

    std::cout << "=== NO_Flychess Server ===" << std::endl;
    std::cout << "启动中..." << std::endl;

    flychess_server::FlychessServer server(8080);

    if (!server.start()) {
        std::cerr << "服务端启动失败！按 Enter 退出..." << std::endl;
        std::cin.get();
        ix::uninitNetSystem();
        return 1;
    }

    std::cout << "服务端运行中，按 Ctrl+C 退出" << std::endl;

    // 主线程保持运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ix::uninitNetSystem();
    return 0;
}
