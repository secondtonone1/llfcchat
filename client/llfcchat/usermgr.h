#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <singleton.h>
#include "userdata.h"
#include <vector>
class UserMgr:public QObject,public Singleton<UserMgr>,
        public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~ UserMgr();
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    void SetToken(QString token);
    int GetUid();
    QString GetName();
    QString GetNick();
    QString GetIcon();
    QString GetDesc();
     std::shared_ptr<UserInfo> GetUserInfo();
    void AppendApplyList(QJsonArray array);
    void AppendFriendList(QJsonArray array);
    std::vector<std::shared_ptr<ApplyInfo>> GetApplyList();
    void AddApplyList(std::shared_ptr<ApplyInfo> app);
    bool AlreadyApply(int uid);
    std::vector<std::shared_ptr<FriendInfo>> GetChatListPerPage();
    bool IsLoadChatFin();
    void UpdateChatLoadedCount();
    std::vector<std::shared_ptr<FriendInfo>> GetConListPerPage();
    void UpdateContactLoadedCount();
    bool IsLoadConFin();
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void AddFriend(std::shared_ptr<AuthInfo> auth_info);
    std::shared_ptr<FriendInfo> GetFriendById(int uid);
    void AppendFriendChatMsg(int friend_id,std::vector<std::shared_ptr<TextChatData>>);
    int GetLastChatThreadId();
    void SetLastChatThreadId(int id);
    void AddChatThreadData(std::shared_ptr<ChatThreadData> chat_thread_data, int other_uid);
    int GetThreadIdByUid(int uid);
private:
    UserMgr();
    std::shared_ptr<UserInfo> _user_info;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;
    QString _token;
    int _chat_loaded;
    int _contact_loaded;
    //建立会话id到数据的映射关系
    QMap<int, std::shared_ptr<ChatThreadData>> _chat_map;
    int _last_chat_thread_id;
    //缓存其他用户uid和聊天的thread_id的映射关系。
    QMap<int, int> _uid_to_thread_id;
public slots:
    void SlotAddFriendRsp(std::shared_ptr<AuthRsp> rsp);
    void SlotAddFriendAuth(std::shared_ptr<AuthInfo> auth);
};

#endif // USERMGR_H
