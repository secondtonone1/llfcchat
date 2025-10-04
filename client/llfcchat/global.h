#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QDir>
#include <QSettings>
/**
 * @brief repolish用来根据属性刷新qss
 */
extern std::function<void(QWidget*)> repolish;
/**
 * @brief The ReqId enum 表示请求的id
 */

extern std::function<QString(QString)> xorString;

/*
* @brief 延迟执行
*/

extern void delay_run(int msecs);

enum ReqId{
    ID_GET_VARIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_RESET_PWD = 1003, //重置密码
    ID_LOGIN_USER = 1004, //用户登录
    ID_CHAT_LOGIN = 1005, //登陆聊天服务器
    ID_CHAT_LOGIN_RSP= 1006, //登陆聊天服务器回包
    ID_SEARCH_USER_REQ = 1007, //用户搜索请求
    ID_SEARCH_USER_RSP = 1008, //搜索用户回包
    ID_ADD_FRIEND_REQ = 1009,  //添加好友申请
    ID_ADD_FRIEND_RSP = 1010, //申请添加好友回复
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,  //认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,  //认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ  = 1017,  //文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP  = 1018,  //文本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //通知用户文本聊天信息
    ID_NOTIFY_OFF_LINE_REQ = 1021, //通知用户下线
    ID_HEART_BEAT_REQ = 1023,      //心跳请求
    ID_HEARTBEAT_RSP = 1024,       //心跳回复
    ID_LOAD_CHAT_THREAD_REQ = 1025,      //加载聊天线程
    ID_LOAD_CHAT_THREAD_RSP = 1026,      //加载聊天线程回复
    ID_CREATE_PRIVATE_CHAT_REQ = 1027, //创建私聊请求
    ID_CREATE_PRIVATE_CHAT_RSP = 1028, //创建私聊回复
    ID_LOAD_CHAT_MSG_REQ = 1029,      //加载聊天消息
    ID_LOAD_CHAT_MSG_RSP = 1030,      //加载聊天消息
    ID_UPLOAD_HEAD_ICON_REQ  = 1031,      //上传头像请求
    ID_UPLOAD_HEAD_ICON_RSP  = 1032,      //上传头像回复
    ID_DOWN_LOAD_FILE_REQ = 1033,             //下载文件请求
    ID_DOWN_LOAD_FILE_RSP = 1034,           //下载文件回复
};
Q_DECLARE_METATYPE(ReqId)

enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1, //Json解析失败
    ERR_NETWORK = 2,
};

enum Modules{
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGINMOD = 2,
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};


extern QString gate_url_prefix;


struct ServerInfo{
public:
    ServerInfo() = default;
    ServerInfo(const ServerInfo& other):_chat_host(other._chat_host),_chat_port(other._chat_port),
        _token(other._token),_uid(other._uid){}
    QString _chat_host;
    QString _chat_port;
    QString _res_host;
    QString _res_port;
    QString _token;
    int _uid;
};

Q_DECLARE_METATYPE(ServerInfo)
Q_DECLARE_METATYPE(std::shared_ptr<ServerInfo>)

enum class ChatRole
{

    Self,
    Other
};

struct MsgInfo{
    QString msgFlag;//"text,image,file"
    QString content;//表示文件和图像的url,文本信息
    QPixmap pixmap;//文件和图片的缩略图
};

//聊天界面几种模式
enum ChatUIMode{
    SearchMode, //搜索模式
    ChatMode, //聊天模式
    ContactMode, //联系模式
    SettingsMode, //设置模式
};

//自定义QListWidgetItem的几种类型
enum ListItemType{
    CHAT_USER_ITEM, //聊天用户
    CONTACT_USER_ITEM, //联系人用户
    SEARCH_USER_ITEM, //搜索到的用户
    ADD_USER_TIP_ITEM, //提示添加用户
    INVALID_ITEM,  //不可点击条目
    GROUP_TIP_ITEM, //分组提示条目
    LINE_ITEM,  //分割线
    APPLY_FRIEND_ITEM, //好友申请
};

//申请好友标签输入框最低长度
const int MIN_APPLY_LABEL_ED_LEN = 40;

const QString add_prefix = "添加标签 ";

const int  tip_offset = 5;


const std::vector<QString>  strs ={"hello world !",
                             "nice to meet u",
                             "New year，new life",
                            "You have to love yourself",
                            "My love is written in the wind ever since the whole world is you"};

const std::vector<QString> heads = {
    ":/res/head_1.jpg",
    ":/res/head_2.jpg",
    ":/res/head_3.jpg",
    ":/res/head_4.jpg",
    ":/res/head_5.jpg"
};

const std::vector<QString> names = {
    "HanMeiMei",
    "Lily",
    "Ben",
    "Androw",
    "Max",
    "Summer",
    "Candy",
    "Hunter"
};

const int CHAT_COUNT_PER_PAGE = 13;

enum MsgStatus{
    UN_READ = 0,  //对方未读
    SEND_FAILED = 1,  //发送失败
    READED = 2  //对方已读
};

//聊天形式，私聊和群聊
enum class ChatFormType {
    PRIVATE = 0,
    GROUP = 1
};

//聊天消息类型，文本，图片，文件等
enum class ChatMsgType {
    TEXT = 0,
    PIC = 1,
    FILE = 2
};

extern QString generateUniqueFileName(const QString& originalName);

extern QString generateUniqueIconName();
//TCP文件上传包头长度
#define FILE_UPLOAD_HEAD_LEN 6
//TCP ID长度
#define FILE_UPLOAD_ID_LEN 2
//TCP 长度字段的长度
#define FILE_UPLOAD_LEN_LEN 4
//最大文件长度
#define MAX_FILE_LEN 2048

struct DownloadInfo {
    QString _name;
    int _total_size;
    int _current_size;
    int _seq;
    QString _client_path;
};

#endif // GLOBAL_H
