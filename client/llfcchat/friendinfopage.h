#ifndef FRIENDINFOPAGE_H
#define FRIENDINFOPAGE_H

#include <QWidget>
#include "userdata.h"

namespace Ui {
class FriendInfoPage;
}

class FriendInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit FriendInfoPage(QWidget *parent = nullptr);
    ~FriendInfoPage();
    void SetInfo(std::shared_ptr<UserInfo> ui);
private slots:
    void on_msg_chat_clicked();

private:
    Ui::FriendInfoPage *ui;
    std::shared_ptr<UserInfo> _user_info;
signals:
    void sig_jump_chat_item(std::shared_ptr<UserInfo> si);
};

#endif // FRIENDINFOPAGE_H
