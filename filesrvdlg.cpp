#include "filesrvdlg.h"
#include "ui_filesrvdlg.h"

FileSrvDlg::FileSrvDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileSrvDlg)
{
    ui->setupUi(this);
    myTcpServer = new QTcpServer(this);
    mySrvPort = 5555;
    connect(myTcpServer, &QTcpServer::newConnection, this, &FileSrvDlg::sndChatMsg);
    myTcpServer->close();
    myTotalBytes = 0;
    mySendBytes = 0;
    myBytesToBeSend = 0;
    myPayLoadSize = 64 * 1024;
    ui->sendProgressBar->reset();
    ui->openFilePushButton->setEnabled(true);
    ui->sendFilePushButton->setEnabled(false);

    connect(ui->openFilePushButton, &QPushButton::clicked, this, &FileSrvDlg::openFilePushButtonClicked);
    connect(ui->sendFilePushButton, &QPushButton::clicked, this, &FileSrvDlg::sendFilePushButtonClicked);
    connect(ui->srvPushButton, &QPushButton::clicked, this, &FileSrvDlg::srvClosePushButtonClicked);
}

FileSrvDlg::~FileSrvDlg()
{
    delete ui;
}

void FileSrvDlg::sndChatMsg()
{
    ui->sendFilePushButton->setEnabled(false);
    mySrvSocket = myTcpServer->nextPendingConnection();
    connect(mySrvSocket, &QTcpSocket::bytesWritten, this, &FileSrvDlg::refreshProgress);
    myLocalPathFile = new QFile(myPathFile, this);
    myLocalPathFile->open(QIODevice::ReadOnly);
    myTotalBytes = myLocalPathFile->size();
    QDataStream sendOut(&myOutputBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_14);
    timer.start();
    QString curFile = myPathFile.right(myPathFile.size() - myPathFile.lastIndexOf('/') - 1);
    sendOut << qint64(0) << qint64(0) << curFile;
    myTotalBytes += myOutputBlock.size();
    sendOut.device()->seek(0); // 重定位到文件头
    sendOut << myTotalBytes << qint64(myOutputBlock.size() - sizeof(qint64) * 2);
    myBytesToBeSend = myTotalBytes - mySrvSocket->write(myOutputBlock);
    myOutputBlock.resize(0); // 清空发送缓存以便下次使用
}

void FileSrvDlg::refreshProgress(qint64 byteNum)
{
    QCoreApplication::processEvents(); // 传输大文件时界面不会卡住
    mySendBytes += byteNum;
    if (myBytesToBeSend > 0)
    {
        myOutputBlock = myLocalPathFile->read(qMin(myBytesToBeSend, myPayLoadSize));
        myBytesToBeSend -= mySrvSocket->write(myOutputBlock);
        myOutputBlock.resize(0);
    }
    else
    {
        myLocalPathFile->close();
    }
    ui->sendProgressBar->setMaximum(myTotalBytes);
    ui->sendProgressBar->setValue(mySendBytes);
    ui->sfileSizeLineEdit->setText(QString("%1").arg(myTotalBytes / (1024 * 1024)) + "MB");
    ui->sendSizeLineEdit->setText(QString("%1").arg(mySendBytes / (1024 * 1024)) + "MB");
    if (mySendBytes == myTotalBytes)
    {
        myLocalPathFile->close();
        myTcpServer->close();
        QMessageBox::information(this, "完毕", "文件传输完成!");

        myTotalBytes = 0;
        mySendBytes = 0;
        myBytesToBeSend = 0;
        ui->openFilePushButton->setEnabled(true);
        ui->sendFilePushButton->setEnabled(false); // 复原状态便于下一次的传输
    }
}

void FileSrvDlg::openFilePushButtonClicked()
{
    myPathFile = QFileDialog::getOpenFileName(this);
    if (!myPathFile.isEmpty())
    {
        myFileName = myPathFile.right(myPathFile.size() - myPathFile.lastIndexOf('/') - 1);
        ui->sfileNameLineEdit->setText(QString("%1").arg(myFileName));
        ui->sendFilePushButton->setEnabled(true);
        ui->openFilePushButton->setEnabled(false);
    }
}

void FileSrvDlg::sendFilePushButtonClicked()
{
    if (!myTcpServer->listen(QHostAddress::Any, mySrvPort))
    {
        QMessageBox::warning(this, "异常", "打开TCP端口出错,请检查网络连接!");
        this->close();
        return ;
    }
    emit sendFileName(myFileName);
}

void FileSrvDlg::srvClosePushButtonClicked()
{
    if (myTcpServer->isListening())
    {
        myTcpServer->close();
        mySrvSocket->abort();
        myLocalPathFile->close();
    }
    this->close();
}

void FileSrvDlg::closeEvent(QCloseEvent *e)
{
    srvClosePushButtonClicked();
}

void FileSrvDlg::cntRefused()
{
    myTcpServer->close();

    ui->sendFilePushButton->setEnabled(false);
    ui->openFilePushButton->setEnabled(true);
    QMessageBox::warning(this, "提示", "对方拒绝接收!");
}

