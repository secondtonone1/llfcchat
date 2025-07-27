#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include "userdata.h"
#include <QMap>
#include "chatitembase.h"

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void SetChatData(std::shared_ptr<ChatThreadData> chat_data);
    void AppendChatMsg(std::shared_ptr<ChatDataBase> msg);
    void UpdateChatStatus(QString unique_id, int status);
protected:
    void paintEvent(QPaintEvent *event);
private slots:
    void on_send_btn_clicked();

    void on_receive_btn_clicked();

private:
    void clearItems();
    Ui::ChatPage *ui;
    std::shared_ptr<ChatThreadData> _chat_data;
    QMap<QString, QWidget*>  _bubble_map;
    QHash<QString, ChatItemBase*> _unrsp_item_map;
};

#endif // CHATPAGE_H
