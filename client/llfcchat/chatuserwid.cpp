#include "chatuserwid.h"
#include "ui_chatuserwid.h"
#include "usermgr.h"

ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
    ui->red_point->raise();
    ShowRedPoint(false);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

QSize ChatUserWid::sizeHint() const
{
    return QSize(250, 70); // 返回自定义的尺寸
}

void ChatUserWid::SetChatData(std::shared_ptr<ChatThreadData> chat_data) {
    _chat_data = chat_data;
    auto other_id = _chat_data->GetOtherId();
    auto other_info = UserMgr::GetInstance()->GetFriendById(other_id);
    // 加载图片
    QPixmap pixmap(other_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(other_info->_name);
    
    ui->user_chat_lb->setText(chat_data->GetLastMsg());

}

std::shared_ptr<ChatThreadData> ChatUserWid::GetChatData()
{
    return _chat_data;
}

void ChatUserWid::ShowRedPoint(bool bshow)
{
    if(bshow){
        ui->red_point->show();
    }else{
        ui->red_point->hide();
    }
}



void ChatUserWid::updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs) {

    int last_msg_id = 0;
    QString last_msg = "";
    for (auto& msg : msgs) {
        last_msg = msg->GetContent();
        last_msg_id = msg->GetMsgId();
        _chat_data->AddMsg(msg);
    }
    
    _chat_data->SetLastMsgId(last_msg_id);
    ui->user_chat_lb->setText(last_msg);
}


