#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->pwdLineEdit->setFocus();
    connect(ui->loginPushButton, &QPushButton::clicked, this, &LoginDialog::loginPushButtonSlot);

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::loginPushButtonSlot()
{
    showWeChatWindow();
}

void LoginDialog::showWeChatWindow()
{
    QFile file("userlog.xml");
    myDoc.setContent(&file);
    QDomElement root = myDoc.documentElement();
    if (root.hasChildNodes())
    {
        QDomNodeList userList = root.childNodes();
        bool exist = false;
        for (int i = 0; i < userList.size(); ++i)
        {
            QDomNode user = userList.at(i);
            QDomNodeList record = user.childNodes();
            QString name = record.at(0).toElement().text();
            QString pwd = record.at(1).toElement().text();
            if (name == ui->userLineEdit->text())
            {
                exist = true;
                if (!(pwd == ui->pwdLineEdit->text()))
                {
                    QMessageBox::warning(this, "提示", "密码错误!请重新输入:");
                    ui->pwdLineEdit->clear();
                    ui->pwdLineEdit->setFocus();
                    return ;
                }
            }
        }
        if (!exist)
        {
            QMessageBox::warning(this, "提示", "此用户不存在!请重新输入:");
            ui->userLineEdit->clear();
            ui->pwdLineEdit->clear();
            ui->userLineEdit->setFocus();
            return ;
        }
        weChatWindow = new MainWindow(this);
        weChatWindow->setWindowTitle(ui->userLineEdit->text());
        this->hide();
        weChatWindow->show();
    }

}
