#include "filecntdlg.h"
#include "ui_filecntdlg.h"

FileCntDlg::FileCntDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileCntDlg)
{
    ui->setupUi(this);
    myCntSocket = new QTcpSocket(this);
    mySrvPort = 5555;
    connect(myCntSocket, &QTcpSocket::readyRead, this, &FileCntDlg::readChatMsg);
    myFileNameSize = 0;
    myTotalBytes = 0;
    myRecvBytes = 0;
}

FileCntDlg::~FileCntDlg()
{
    delete ui;
}

void FileCntDlg::createConnectToSrv()
{
    myBlockSize = 0;
    myCntSocket->abort();
    myCntSocket->connectToHost(mySrvAddr, mySrvPort);
    timer.start();
}

void FileCntDlg::readChatMsg()
{
    QDataStream in(myCntSocket);
    in.setVersion(QDataStream::Qt_5_14);

    float usedTime = timer.elapsed();
    if (myRecvBytes <= sizeof(qint16) * 2)
    {
        if ((myCntSocket->bytesAvailable() >= sizeof(qint16) * 2) && (myFileNameSize == 0))
        {
            in >> myTotalBytes >> myFileNameSize;
            myRecvBytes += sizeof(qint64) * 2;
        }
        if ((myCntSocket->bytesAvailable() >= myFileNameSize) && (myFileNameSize != 0))
        {
            in >> myFileName;
            myRecvBytes += myFileNameSize;
            myLocalPathFile->open(QIODevice::WriteOnly);
            ui->rfileNameLineEdit->setText(myFileName);
        }
        else
            return ;
    }
    if (myRecvBytes < myTotalBytes)
    {
        myRecvBytes += myCntSocket->bytesAvailable();
        myInputBlock = myCntSocket->readAll();
        myLocalPathFile->write(myInputBlock);
        myInputBlock.resize(0);
    }
    ui->recvProgressBar->setMaximum(myTotalBytes);
    ui->recvProgressBar->setValue(myRecvBytes);
    double tranSpeed = myRecvBytes / usedTime;
    ui->rfileSizeLineEdit->setText(QString("%1").arg(myTotalBytes / (1024 * 1024)) + "MB");
    ui->recvSizeLineEdit->setText(QString("%1").arg(myRecvBytes / (1024 * 1024)) + "MB");
    ui->rateLabel->setText(QString("%1").arg(tranSpeed * 1000 / (1024 * 1024)) + "MB/秒");

    if (myRecvBytes == myTotalBytes)
    {
        myLocalPathFile->close();
        myCntSocket->disconnectFromHost();
        ui->rateLabel->setText("接收完毕!");
    }
}


void FileCntDlg::cntClosePushButtonClicked()
{
    myCntSocket->abort();
    myLocalPathFile->close();
    this->close();
}

void FileCntDlg::closeEvent(QCloseEvent *)
{
    cntClosePushButtonClicked();
}

void FileCntDlg::getLocalPath(const QString &path)
{
    myLocalPathFile = new QFile(path, this);
}

void FileCntDlg::getSrvAddr(QHostAddress addr)
{
    mySrvAddr = addr;
    createConnectToSrv();
}
