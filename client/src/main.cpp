#include <QApplication>
#include "MainWindow.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return -1;
    }
#endif
    flychess_game::register_cards();
    flychess_game::register_functions();
    
    QApplication app(argc, argv);

    flychess_client::MainWindow window;
    window.show();

    int ret = app.exec();

#ifdef _WIN32
    WSACleanup();
#endif

    return ret;
}
