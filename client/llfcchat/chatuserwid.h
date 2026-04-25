#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"
namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();
    QSize sizeHint() const override;
    void SetChatData(std::shared_ptr<ChatThreadData> chat_data);
    std::shared_ptr<ChatThreadData> GetChatData();
    void ShowRedPoint(bool bshow);
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);
private:
    Ui::ChatUserWid *ui;
    std::shared_ptr<ChatThreadData> _chat_data;
};

#endif // CHATUSERWID_H
