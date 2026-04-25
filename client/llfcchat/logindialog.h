#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"

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
    void initHead();
    void initHttpHandlers();
    void showTip(QString str,bool b_ok);
    bool checkUserValid();
    bool checkPwdValid();
    Ui::LoginDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    bool enableBtn(bool);
    QMap<TipErr, QString> _tip_errs;
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    int _uid;
    QString _token;
private slots:
    void slot_forget_pwd();
    void on_login_btn_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_con_finish(bool bsuccess);
    void slot_login_failed(int);
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);
};

#endif // LOGINDIALOG_H
