#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QList>
#include "statelabel.h"
#include "global.h"
#include "statewidget.h"
#include <memory>
#include "userdata.h"
#include <QListWidgetItem>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override ;

    void handleGlobalMousePress(QMouseEvent *event) ;
    void CloseFindDlg();
    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);
    
private:
    void AddLBGroup(StateWidget* lb);
    void addChatUserList();
    void loadMoreChatUser();
    void ClearLabelState(StateWidget* lb);
    void loadMoreConUser();
    void SetSelectChatItem(int uid = 0);
    void SetSelectChatPage(int uid = 0);
    Ui::ChatDialog *ui;
    bool _b_loading;
    QList<StateWidget*> _lb_list;
    void ShowSearch(bool bsearch = false);
    ChatUIMode _mode;
    ChatUIMode _state;
    QWidget* _last_widget;
    //todo...
    QMap<int, QListWidgetItem*> _chat_items_added;
    int _cur_chat_uid;
public slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString & str);
    void slot_focus_out();
    void slot_loading_contact_user();
    void slot_switch_apply_friend_page();
    void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);
    void slot_show_search(bool show);
    void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> ui);
    void slot_item_clicked(QListWidgetItem *item);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata);
private slots:

};



#endif // CHATDIALOG_H
