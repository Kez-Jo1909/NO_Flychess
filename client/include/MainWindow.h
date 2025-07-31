#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>
#include <QUrl>
#include <QTimer>
#include <QDesktopServices>
#include "game.h"
#include "server.h"
#include "client.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace flychess_client{

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void StartButtonClicked();  // 开始按钮槽函数

    void onExitButtonClicked();// 退出按钮槽函数

    void onSettingButtonClicked();// 设置按钮槽函数

    void onSettingExitClicked();// 设置中返回槽函数

    void onSettingReButtonClicked();// 设置中重置按钮槽函数

    void onSettingSaveButtonClicked();

    void showAboutDialog();

    void onCreateGameButtonClicked();

    void onPreparePageExitButtonClicked();

    void onPreparePageStartButtonClicked();
private:
    // void repositionStartMenu();

protected:
    void resizeEvent(QResizeEvent *event) override;

// 以下是私有成员变量
private:
    Ui::MainWindow *ui;
    std::unique_ptr<flychess_server::FlycehssServer> server_;

    // 直接在类内声明客户端实例
    flychess_client::FlychessClient *client_;
};

}// namespace flychess_client
#endif // MAINWINDOW_H
