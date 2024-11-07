#ifndef FILECNTDLG_H
#define FILECNTDLG_H

#include <QDialog>
#include <QFile>
#include <QElapsedTimer>
#include <QTcpSocket>
#include <QHostAddress>

namespace Ui {
class FileCntDlg;
}

class FileCntDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FileCntDlg(QWidget *parent = nullptr);
    ~FileCntDlg();
    void getSrvAddr(QHostAddress addr); // 获取服务器(发送端)IP
    void getLocalPath(const QString &path); // 获取本地文件保存路径

protected:
    void closeEvent(QCloseEvent *) override;


private:
    Ui::FileCntDlg *ui;
    QTcpSocket *myCntSocket; // 客户端套接字指针
    QHostAddress mySrvAddr; // 服务器地址
    qint16 mySrvPort; // 服务器端口

    qint64 myTotalBytes; // 总共要接收的字节数
    qint64 myRecvBytes; // 已接收的字节数
    QByteArray myInputBlock; // 缓存一次收下的数据
    quint16 myBlockSize; // 缓存块大小

    QFile *myLocalPathFile; // 待接收文件对象指针
    QString myFileName; // 待收文件名
    qint64 myFileNameSize; // 文件名大小

    QElapsedTimer timer;

    void createConnectToSrv(); // 连接服务器
    void readChatMsg(); // 读取服务器发来的文件数据
    void cntClosePushButtonClicked(); // "停止"按钮的单击事件槽函数
};

#endif // FILECNTDLG_H
