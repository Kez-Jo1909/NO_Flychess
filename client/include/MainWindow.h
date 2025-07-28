#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

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

    void on_ExitButton_clicked();// 退出按钮槽函数

private:
    void repositionStartMenu();

protected:
    void resizeEvent(QResizeEvent *event) override;

// 以下是私有成员变量
private:
    Ui::MainWindow *ui;
};

}// namespace flychess_client
#endif // MAINWINDOW_H
