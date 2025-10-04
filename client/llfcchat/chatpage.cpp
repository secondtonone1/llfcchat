#include "chatpage.h"
#include "ui_chatpage.h"
#include <QStyleOption>
#include <QPainter>
#include "ChatItemBase.h"
#include "TextBubble.h"
#include "PictureBubble.h"
#include "applyfrienditem.h"
#include "usermgr.h"
#include <QJsonArray>
#include <QJsonObject>
#include "tcpmgr.h"
#include <QUuid>
#include <QStandardPaths>
#include "filetcpmgr.h"

ChatPage::ChatPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatPage)
{
    ui->setupUi(this);
    //设置按钮样式
    ui->receive_btn->SetState("normal","hover","press");
    ui->send_btn->SetState("normal","hover","press");

    //设置图标样式
    ui->emo_lb->SetState("normal","hover","press","normal","hover","press");
    ui->file_lb->SetState("normal","hover","press","normal","hover","press");

}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::SetChatData(std::shared_ptr<ChatThreadData> chat_data) {
    _chat_data = chat_data;
    auto other_id = _chat_data->GetOtherId();
    if(other_id == 0) {
        //说明是群聊
        ui->title_lb->setText(_chat_data->GetGroupName());
        //todo...加载群聊信息和成员信息
        return;
    }

    //私聊
    auto friend_info = UserMgr::GetInstance()->GetFriendById(other_id);
    if (friend_info == nullptr) {
        return;
    }
    ui->title_lb->setText(friend_info->_name);
    ui->chat_data_list->removeAllItem();
    _unrsp_item_map.clear();
    for(auto & msg : chat_data->GetMsgMapRef()){
        AppendChatMsg(msg);
    }

    for (auto& msg : chat_data->GetMsgUnRspRef()) {
        AppendChatMsg(msg);
    }
}

void ChatPage::AppendChatMsg(std::shared_ptr<ChatDataBase> msg)
{
    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role;
    if (msg->GetSendUid() == self_info->_uid) {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        
        pChatItem->setUserName(self_info->_name);

        // 使用正则表达式检查是否是默认头像
        QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
        QRegularExpressionMatch match = regex.match(self_info->_icon);
        if (match.hasMatch()) {
            pChatItem->setUserIcon(QPixmap(self_info->_icon));
        }
        else {
            // 如果是用户上传的头像，获取存储目录
            QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir avatarsDir(storageDir + "/avatars");
            auto file_name = QFileInfo(self_info->_icon).fileName();
            // 确保目录存在
            if (avatarsDir.exists()) {
                QString avatarPath = avatarsDir.filePath(file_name); // 获取上传头像的完整路径
                QPixmap pixmap(avatarPath); // 加载上传的头像图片
                if (!pixmap.isNull()) {
                    pChatItem->setUserIcon(pixmap);
                }
                else {
                    qWarning() << "无法加载上传的头像：" << avatarPath;
                    auto icon_label = pChatItem->getIconLabel();
                    UserMgr::GetInstance()->AddLabelToReset(avatarPath, icon_label);
                    //先加载默认的
                    QPixmap pixmap(":/res/head_1.jpg");
                    QPixmap scaledPixmap = pixmap.scaled(icon_label->size(),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
                    icon_label->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
                    icon_label->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

                    //判断是否正在下载
                    bool is_loading = UserMgr::GetInstance()->IsDownLoading(file_name);
                    if (is_loading) {
                        qWarning() << "正在下载: " << file_name;
                    }
                    else {
                        //发送请求获取资源
                        auto download_info = std::make_shared<DownloadInfo>();
                        download_info->_name = file_name;
                        download_info->_current_size = 0;
                        download_info->_seq = 1;
                        download_info->_total_size = 0;
                        download_info->_client_path = avatarPath;
                        //添加文件到管理者
                        UserMgr::GetInstance()->AddDownloadFile(file_name, download_info);
                        //发送消息
                        FileTcpMgr::GetInstance()->SendDownloadInfo(download_info);
                    }
                }
            }
            else {
                qWarning() << "头像存储目录不存在：" << avatarsDir.path();
            }
        }
   
        QWidget* pBubble = nullptr;
        if (msg->GetMsgType() == ChatMsgType::TEXT) {
            pBubble = new TextBubble(role, msg->GetMsgContent());
        }
     
        pChatItem->setWidget(pBubble);
        auto status = msg->GetStatus();
        pChatItem->setStatus(status);
        ui->chat_data_list->appendChatItem(pChatItem);
        if (status == 0) {
            _unrsp_item_map[msg->GetUniqueId()] = pChatItem;
        }
    }
    else {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::GetInstance()->GetFriendById(msg->GetSendUid());
        if (friend_info == nullptr) {
            return;
        }
        pChatItem->setUserName(friend_info->_name);
        
        // 使用正则表达式检查是否是默认头像
        QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
        QRegularExpressionMatch match = regex.match(friend_info->_icon);
        if (match.hasMatch()) {
            pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        }
        else {
            // 如果是用户上传的头像，获取存储目录
            QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir avatarsDir(storageDir + "/avatars");
            auto file_name = QFileInfo(self_info->_icon).fileName();
            // 确保目录存在
            if (avatarsDir.exists()) {
                QString avatarPath = avatarsDir.filePath(file_name); // 获取上传头像的完整路径
                QPixmap pixmap(avatarPath); // 加载上传的头像图片
                if (!pixmap.isNull()) {
                    pChatItem->setUserIcon(pixmap);
                }
                else {
                    qWarning() << "无法加载上传的头像：" << avatarPath;
                    auto icon_label = pChatItem->getIconLabel();
                    UserMgr::GetInstance()->AddLabelToReset(avatarPath, icon_label);
                    //先加载默认的
                    QPixmap pixmap(":/res/head_1.jpg");
                    QPixmap scaledPixmap = pixmap.scaled(icon_label->size(),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
                    icon_label->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
                    icon_label->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

                    //判断是否正在下载
                    bool is_loading = UserMgr::GetInstance()->IsDownLoading(file_name);
                    if (is_loading) {
                        qWarning() << "正在下载: " << file_name;
                    }
                    else {
                        //发送请求获取资源
                        auto download_info = std::make_shared<DownloadInfo>();
                        download_info->_name = file_name;
                        download_info->_current_size = 0;
                        download_info->_seq = 1;
                        download_info->_total_size = 0;
                        download_info->_client_path = avatarPath;
                        //添加文件到管理者
                        UserMgr::GetInstance()->AddDownloadFile(file_name, download_info);
                        //发送消息
                        FileTcpMgr::GetInstance()->SendDownloadInfo(download_info);
                    }
                }
            }
            else {
                qWarning() << "头像存储目录不存在：" << avatarsDir.path();
            }
        }

        QWidget* pBubble = nullptr;
        if (msg->GetMsgType() == ChatMsgType::TEXT) {
            pBubble = new TextBubble(role, msg->GetMsgContent());
        }
        pChatItem->setWidget(pBubble);
        auto status = msg->GetStatus();
        pChatItem->setStatus(status);
        ui->chat_data_list->appendChatItem(pChatItem);
        if (status == 0) {
            _unrsp_item_map[msg->GetUniqueId()] = pChatItem;
        }
    }


}

void ChatPage::UpdateChatStatus(QString unique_id, int status)
{
    auto iter = _unrsp_item_map.find(unique_id);
    if (iter != _unrsp_item_map.end()) {
        iter.value()->setStatus(status);
        _unrsp_item_map.erase(iter);
    }
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked()
{
    if (_chat_data == nullptr) {
        qDebug() << "friend_info is empty";
        return;
    }

    auto user_info = UserMgr::GetInstance()->GetUserInfo();
    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Self;
    QString userName = user_info->_name;
    QString userIcon = user_info->_icon;

    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    int txt_size = 0;
    auto thread_id = _chat_data->GetThreadId();
    for(int i=0; i<msgList.size(); ++i)
    {
        //消息内容长度不合规就跳过
        if(msgList[i].content.length() > 1024){
            continue;
        }

        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;
        //生成唯一id
        QUuid uuid = QUuid::createUuid();
        //转为字符串
        QString uuidString = uuid.toString();
        if(type == "text")
        {   
            pBubble = new TextBubble(role, msgList[i].content);
            if(txt_size + msgList[i].content.length()> 1024){
                textObj["fromuid"] = user_info->_uid;
                textObj["touid"] = _chat_data->GetOtherId();
                textObj["thread_id"] = thread_id;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //发送并清空之前累计的文本列表
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //_bubble_map[uuidString] = pBubble;
            txt_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            auto content = QString::fromUtf8(utf8Message);
            obj["content"] = content;
            obj["unique_id"] = uuidString;
            textArray.append(obj);
            //todo... 注意，此处先按私聊处理
            auto txt_msg = std::make_shared<TextChatData>(uuidString, thread_id, ChatFormType::PRIVATE, 
                ChatMsgType::TEXT, content, user_info->_uid, 0);
            //将未回复的消息加入到未回复列表中，以便后续处理
            _chat_data->AppendUnRspMsg(uuidString,txt_msg);
        }
        else if(type == "image")
        {
             pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }
        //发送消息
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            pChatItem->setStatus(0);
            ui->chat_data_list->appendChatItem(pChatItem);
            _unrsp_item_map[uuidString] = pChatItem;
        }

    }

    qDebug() << "textArray is " << textArray ;
    //发送给服务器
    textObj["text_array"] = textArray;
    textObj["fromuid"] = user_info->_uid;
    textObj["touid"] = _chat_data->GetOtherId();
    textObj["thread_id"] = thread_id;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    //发送并清空之前累计的文本列表
    txt_size = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
}

void ChatPage::on_receive_btn_clicked()
{
    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Other;
    auto friend_info = UserMgr::GetInstance()->GetFriendById(_chat_data->GetOtherId());
    QString userName = friend_info->_name;
    QString userIcon = friend_info->_icon;

    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    for(int i=0; i<msgList.size(); ++i)
    {
        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;
        if(type == "text")
        {
            pBubble = new TextBubble(role, msgList[i].content);
        }
        else if(type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            pChatItem->setStatus(2);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
    }
}

void ChatPage::clearItems()
{
    ui->chat_data_list->removeAllItem();
}
