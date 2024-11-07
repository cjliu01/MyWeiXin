#ifndef FILESRVDLG_H
#define FILESRVDLG_H

#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class FileSrvDlg;
}

class FileSrvDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FileSrvDlg(QWidget *parent = nullptr);
    ~FileSrvDlg();

    void cntRefused(); // 被客户端拒绝后的处理办法
signals:
    void sendFileName(const QString &name);

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    Ui::FileSrvDlg *ui;
    QTcpServer *myTcpServer; // TCP服务器对象指针
    QTcpSocket *mySrvSocket; // TCP服务套接字接口指针
    qint16 mySrvPort;

    QFile *myLocalPathFile; // 文件对象指针
    QString myPathFile; // 含路径的本地待发送文件名
    QString myFileName; // 文件名(去掉路径部分)

    qint64 myTotalBytes; // 总共要发送的字节数
    qint64 mySendBytes; // 已发送的字节数
    qint64 myBytesToBeSend; // 余下字节数
    qint64 myPayLoadSize; // 有效载荷
    QByteArray myOutputBlock; // 缓存一次发送的数据
    QTimer timer;

    void sndChatMsg(); // 发送消息方法
    void refreshProgress(qint64 byteNum); // 刷新服务器进度条方法
    void openFilePushButtonClicked(); // 打开选择要传输的文件
    void sendFilePushButtonClicked(); // "发送"按钮单击事件方法
    void srvClosePushButtonClicked(); // "停止"按钮单击事件方法
};

#endif // FILESRVDLG_H
