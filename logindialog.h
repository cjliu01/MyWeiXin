#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include "mainwindow.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    void loginPushButtonSlot();
    void showWeChatWindow();

private:
    Ui::LoginDialog *ui;
    MainWindow *weChatWindow;
    QDomDocument myDoc;
};

#endif // LOGINDIALOG_H
