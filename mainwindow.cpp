#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initWindow()
{
    myUdpSocket = new QUdpSocket(this);
    myUdpPort = 23232;
    myUdpSocket->bind(myUdpPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    connect(myUdpSocket, &QUdpSocket::readyRead, this, &MainWindow::recvAndProcessChatMsg);
    myFsrv = new FileSrvDlg(this);
    connect(myFsrv, &FileSrvDlg::sendFileName, this, &MainWindow::getSFileName);
    ui->userListTableWidget->setColumnCount(1); // 关键
    connect(ui->searchPushButton, &QPushButton::clicked, this, &MainWindow::searchPushButtonClicked);
    connect(ui->sendPushButton, &QPushButton::clicked, this, &MainWindow::sendPushButtonClicked);
    connect(ui->transPushButton, &QPushButton::clicked, this, &MainWindow::transPushButtonClicked);
}

void MainWindow::searchPushButtonClicked()
{
    myName = this->windowTitle();
    ui->userLabel->setText(myName);
    sendChatMsg(ChatMsgType::ONLINE);
}

void MainWindow::sendChatMsg(ChatMsgType msgType, QString rmtName)
{
    QByteArray qba;
    QDataStream write(&qba, QIODevice::WriteOnly);
    QString localHostIP = getLocalHostIp();
    QString localHostMsg = getLocalChatMsg();
    write << msgType << myName;
    switch (msgType)
    {
        case ChatMsgType::CHATMSG: write << localHostIP << localHostMsg; break;
        case ChatMsgType::ONLINE: write << localHostIP; break;
        case ChatMsgType::OFFLINE: break;
        case ChatMsgType::SFILENAME: write << localHostIP << rmtName << myFileName; break;
        case ChatMsgType::REFFILE: write << localHostIP << rmtName; break;
    }
    myUdpSocket->writeDatagram(qba, qba.length(), QHostAddress::Broadcast, myUdpPort);
    myUdpSocket->waitForReadyRead(100); // 结束发送，等待读取
    // https://blog.csdn.net/qq_45304158/article/details/108625740 关键
}

void MainWindow::recvAndProcessChatMsg()
{
    while (myUdpSocket->hasPendingDatagrams())
    {
        QByteArray qba;
        qba.resize(myUdpSocket->pendingDatagramSize());
        myUdpSocket->readDatagram(qba.data(), qba.size());
        QDataStream read(&qba, QIODevice::ReadOnly);
        ChatMsgType msgType;
        read >> msgType;
        QString name, hostIP, chatMsg, rName, fName;
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch (msgType)
        {
        case ChatMsgType::CHATMSG:
            read >> name >> hostIP >> chatMsg;
            ui->chatTextBrowser->setTextColor(Qt::darkGreen);
            ui->chatTextBrowser->setCurrentFont(QFont("Times New Roman", 14));
            ui->chatTextBrowser->append("【" + name + "】" + currentTime);
            ui->chatTextBrowser->append(chatMsg);
            break;
        case ChatMsgType::ONLINE:
            read >> name >> hostIP;
            onLine(name, currentTime);
            break;
        case ChatMsgType::OFFLINE:
            read >> name;
            offLine(name, currentTime);
            break;
        case ChatMsgType::SFILENAME:
            read >> name >> hostIP >> rName >> fName;
            recvFileName(name, hostIP, rName, fName);
            break;
        case ChatMsgType::REFFILE:
            read >> name >> hostIP >> rName;
            if (myName == rName)
                myFsrv->cntRefused();
            break;
        }
    }
}

void MainWindow::onLine(const QString &name, const QString &time)
{
    bool notExist = ui->userListTableWidget->findItems(name, Qt::MatchExactly).isEmpty();
    if (notExist)
    {
        QTableWidgetItem *newUser = new QTableWidgetItem(name);
        ui->userListTableWidget->insertRow(0);
        ui->userListTableWidget->setItem(0, 0, newUser);
        ui->chatTextBrowser->setTextColor(Qt::gray);
        ui->chatTextBrowser->setCurrentFont(QFont("Times New Roman", 12));
        ui->chatTextBrowser->append(QString("%1 %2上线!").arg(time).arg(name));
        sendChatMsg(ChatMsgType::ONLINE);
    }
}

void MainWindow::offLine(const QString &name, const QString &time)
{
    int row = ui->userListTableWidget->findItems(name, Qt::MatchExactly).front()->row();
    ui->userListTableWidget->removeRow(row);
    ui->chatTextBrowser->setTextColor(Qt::gray);
    ui->chatTextBrowser->setCurrentFont(QFont("Times New Roman", 12));
    ui->chatTextBrowser->append(QString("%1 %2离线!").arg(time).arg(name));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    sendChatMsg(ChatMsgType::OFFLINE);
}

QString MainWindow::getLocalHostIp() const
{
//    QList<QHostAddress> addrList = QNetworkInterface::allAddresses();
//    for (auto& addr: addrList)
//    {
//        if (addr.protocol() == QAbstractSocket::IPv4Protocol)
//            return addr.toString();
//    }
//    return "";
    return "127.0.0.1";
}

QString MainWindow::getLocalChatMsg() const
{
    QString chatMsg = ui->chatTextEdit->toHtml();
    ui->chatTextEdit->clear();
    ui->chatTextEdit->setFocus();
    return chatMsg;
}

void MainWindow::sendPushButtonClicked()
{
    sendChatMsg(ChatMsgType::CHATMSG);
}

void MainWindow::getSFileName(const QString &fName)
{
    myFileName = fName;
    int row = ui->userListTableWidget->currentRow();
    QString rmtName = ui->userListTableWidget->item(row, 0)->text();
    sendChatMsg(ChatMsgType::SFILENAME, rmtName);
}

void MainWindow::transPushButtonClicked()
{
    if (ui->userListTableWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "选择好友", "请先选择文件接收方!", QMessageBox::Ok);
        return ;
    }
    myFsrv->show();
}

void MainWindow::recvFileName(const QString &name, const QString &hostIP, const QString &rmtName, const QString &fileName)
{
    if (myName == rmtName)
    {
        QMessageBox::StandardButton result = QMessageBox::information(this, "收到文件", QString("好友 %1 给您发文件: \r\n%2，是否接收?").arg(name).arg(fileName),
                                                QMessageBox::Yes, QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            QString fName = QFileDialog::getSaveFileName(this, "保存", fileName);
            if (!fName.isEmpty())
            {
                fCnt = new FileCntDlg(this);
                fCnt->getLocalPath(fName);
                fCnt->getSrvAddr(QHostAddress(hostIP));
                fCnt->show();
            }
        }
        else
            sendChatMsg(ChatMsgType::REFFILE, name);
    }
}
