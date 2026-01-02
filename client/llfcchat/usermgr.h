#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <singleton.h>
#include "userdata.h"
#include <vector>
#include <mutex>
#include <qlabel.h>
#include "global.h"

class UserMgr:public QObject,public Singleton<UserMgr>,
        public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~ UserMgr();
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    void SetToken(QString token);
    QString GetToken();
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
    std::vector<std::shared_ptr<UserInfo>> GetChatListPerPage();
    bool IsLoadChatFin();
    void UpdateChatLoadedCount();
    std::vector<std::shared_ptr<UserInfo>> GetConListPerPage();
    void UpdateContactLoadedCount();
    bool IsLoadConFin();
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void AddFriend(std::shared_ptr<AuthInfo> auth_info);
    std::shared_ptr<UserInfo> GetFriendById(int uid);
    int GetLastChatThreadId();
    void SetLastChatThreadId(int id);
    void AddChatThreadData(std::shared_ptr<ChatThreadData> chat_thread_data, int other_uid);
    int GetThreadIdByUid(int uid);
    std::shared_ptr<ChatThreadData> GetChatThreadByThreadId(int thread_id);
    std::shared_ptr<ChatThreadData> GetChatThreadByUid(int uid);

    //获取当前正在加载的聊天数据。
    std::shared_ptr<ChatThreadData> GetCurLoadData();
    std::shared_ptr<ChatThreadData> GetNextLoadData();

    //将md5和文件信息关联起来
    void AddUploadFile(QString name, std::shared_ptr<QFileInfo> file_info);
    //移除上传的文件信息
    void RmvUploadFile(QString name);
    //获取上传信息
    std::shared_ptr<QFileInfo> GetUploadInfoByName(QString name);
    bool IsDownLoading(QString name);
    void AddDownloadFile(QString name, std::shared_ptr<DownloadInfo> file_info);
    void RmvDownloadFile(QString name);
    std::shared_ptr<DownloadInfo> GetDownloadInfo(QString name);
    //添加资源路径到将要重置的Label集合
    void AddLabelToReset(QString path, QLabel* label);
    void ResetLabelIcon(QString path);
    void AddTransFile(QString name, std::shared_ptr<MsgInfo> msg_info);
    std::shared_ptr<MsgInfo> GetTransFileByName(QString name);
    void RmvTransFileByName(QString name);
    std::shared_ptr<MsgInfo> GetFreeUploadFile();
    std::shared_ptr<MsgInfo> GetFreeDownloadFile();
    void PauseTransFileByName(QString name);
    void ResumeTransFileByName(QString name);
    bool TransFileIsUploading(QString name);

private:
    UserMgr();
    std::shared_ptr<UserInfo> _user_info;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    std::vector<std::shared_ptr<UserInfo>> _friend_list;
    QMap<int, std::shared_ptr<UserInfo>> _friend_map;
    QString _token;
    int _chat_loaded;
    int _contact_loaded;
    //建立会话id到数据的映射关系
    QMap<int, std::shared_ptr<ChatThreadData>> _chat_map;
    //聊天会话id列表
    std::vector<int> _chat_thread_ids;
    //记录已经加载聊天列表的会话索引
    int _cur_load_chat_index;
    //上次会话的id
    int _last_chat_thread_id;
    //缓存其他用户uid和聊天的thread_id的映射关系。
    QMap<int, int> _uid_to_thread_id;

    std::mutex _mtx;

    //上传文件md5和文件信息关联 映射
    QMap<QString, std::shared_ptr<QFileInfo> > _name_to_upload_info;
    std::mutex _down_load_mtx;
    //名字关联下载信息
    QMap<QString, std::shared_ptr<DownloadInfo> > _name_to_download_info;
    QHash<QString, QList<QLabel*>> _path_to_reset_labels;
    //聊天传输文件映射
    QHash<QString, std::shared_ptr<MsgInfo> > _name_to_msg_info;
    //传输文件用的锁
    std::mutex _trans_mtx;

public slots:
    void SlotAddFriendRsp(std::shared_ptr<AuthRsp> rsp);
    void SlotAddFriendAuth(std::shared_ptr<AuthInfo> auth);
};

#endif // USERMGR_H
