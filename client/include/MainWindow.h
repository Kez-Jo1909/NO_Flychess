#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>
#include <QUrl>
#include <QTimer>
#include <QDesktopServices>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QVariant>
#include <QPainter>
#include <QList>
#include <QComboBox>
#include "game.h"
#include "server.h"
#include "client.h"
#include "utils.h"
#include "utils.h"

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
    void onPreparePageExitButtonUserClicked();

    void UrlEditEnter();

    void onPreparePageStartButtonClicked();
    void onPreparePagePrepareButtonClicked();

    void onPlayerCountChanged(int index);
    void onChessCountChanged(int index);

    void onNewPlayerJoined(QString name, game_utils::Color color);
    void onRegisterResult(game_utils::Color color);
    void onPlayerListUpdated(const QList<QVariantList>& players);
    void onPlayerLeaveRoom(QString name, game_utils::Color color);
    void onPlayerCountUpdate(int num);
    void onChessCountUpdate(int num);
    void onPlayerCountUpdateFailed(int min_num);
    void onGameStart();
    void onGameStartFailed();
private:
    // void repositionStartMenu();

    void onConnected();
    void onDisconnected();
    void onConnectTimeout();

protected:
    void resizeEvent(QResizeEvent *event) override;

// 以下是私有成员变量
private:
    Ui::MainWindow *ui;
    std::unique_ptr<flychess_server::FlychessServer> server_;

    // 直接在类内声明客户端实例
    flychess_client::FlychessClient *client_;

    QMessageBox* connectingBox_ = nullptr;  // “正在连接”提示框
    QTimer* connectTimer_ = nullptr;        // 连接超时定时器

    bool is_server = false;
    bool prepared = false;

    game_utils::Color user_color_ = game_utils::Color::UNDEFINED; // 默认颜色
};

}// namespace flychess_client

class ChessBoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChessBoardWidget(QWidget *parent = nullptr);

protected:
    // void resizeEvent(QResizeEvent *event) override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent *event) override;

private:
};


#endif // MAINWINDOW_H
