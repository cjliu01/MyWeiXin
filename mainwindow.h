#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QDomDocument>
#include <QHostInfo>
#include "filesrvdlg.h"
#include "filecntdlg.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class ChatMsgType { CHATMSG, ONLINE, OFFLINE, SFILENAME, REFFILE };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initWindow(); // 窗口初始化方法
    void onLine(const QString &name, const QString &time); // 新用户上线方法
    void offLine(const QString &name, const QString &time);  // 用户离线方法
    void sendChatMsg(ChatMsgType msgType, QString rmtName=""); // 发送UDP信息

    QString getLocalHostIp() const; // 获取本端的IP地址
    QString getLocalChatMsg() const; // 获取本段的聊天信息内容

    void recvFileName(const QString &name, const QString &hostIP, const QString &rmtName, const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event); // 重写关闭窗口方法以便发送通知离线消息

private:
    Ui::MainWindow *ui;
    QString myName; // 本端当前的用户名
    QUdpSocket *myUdpSocket; // UDP套接字指针
    qint16 myUdpPort; // UDP端口号
    QDomDocument myDoc;
    QString myFileName;
    FileSrvDlg *myFsrv;
    FileCntDlg *fCnt;

    void sendPushButtonClicked(); // "发送"按钮的单击事件方法
    void recvAndProcessChatMsg(); // 接收并处理UDP用户数据报
    void searchPushButtonClicked(); // 搜索所有线上用户
    void getSFileName(const QString &fName);
    void transPushButtonClicked();

};
#endif // MAINWINDOW_H
