#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
private:
    bool checkUserValid();
    bool checkPwdValid();
    Ui::LoginDialog *ui;
private slots:
    void slot_forget_pwd();
    void on_login_btn_clicked();

signals:
    void switchRegister();
    void switchReset();
};

#endif // LOGINDIALOG_H
