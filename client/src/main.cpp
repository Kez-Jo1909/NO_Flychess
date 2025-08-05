#include <QApplication>
#include "MainWindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    QApplication app(argc, argv);

    flychess_client::MainWindow window;
    window.show();

    return app.exec();
}
