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
#include <set>
#include <queue>

//TCP文件上传包头长度
#define FILE_UPLOAD_HEAD_LEN 6
//TCP ID长度
#define FILE_UPLOAD_ID_LEN 2
//TCP 长度字段的长度
#define FILE_UPLOAD_LEN_LEN 4
//最大文件长度
#define MAX_FILE_LEN (1024*32)
//定义最大拥塞窗口的大小
#define MAX_CWND_SIZE 5


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
    ID_IMG_CHAT_MSG_REQ = 1035,            //图片聊天消息请求
    ID_IMG_CHAT_MSG_RSP = 1036,           //图片聊天信息回复
    ID_IMG_CHAT_UPLOAD_REQ = 1037,        //上传聊天图片资源
    ID_IMG_CHAT_UPLOAD_RSP = 1038,        //上传聊天图片资源回复

    ID_NOTIFY_IMG_CHAT_MSG_REQ = 1039,   //通知用户图片聊天信息
    ID_FILE_INFO_SYNC_REQ     =  1041,    //文件信息同步请求
    ID_FILE_INFO_SYNC_RSP     =  1042,     //文件信息同步回复
    ID_IMG_CHAT_CONTINUE_UPLOAD_REQ = 1043,  //续传聊天图片资源请求
    ID_IMG_CHAT_CONTINUE_UPLOAD_RSP = 1044,  //续传聊天图片资源回复
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

enum class MsgType {
    TEXT_MSG = 0, //文本消息
    IMG_MSG = 1,  //图片消息
    VIDEO_MSG = 2, //视频消息
    FILE_MSG = 3//文件消息,
};

enum class TransferType {
    None,
    Download,  //下载
    Upload     //上传
};

enum class TransferState {
    None,           // 无传输
    Downloading,    // 下载中
    Uploading,      // 上传中
    Paused,         // 暂停
    Completed,      // 完成
    Failed          // 失败
};

struct MsgInfo{
    MsgInfo() = default;
    MsgInfo(MsgType msgtype, QString text_or_url, QPixmap pixmap, QString unique_name, qint64 total_size, QString md5)
    :_msg_type(msgtype), _text_or_url(text_or_url), _preview_pix(pixmap),_unique_name(unique_name),_total_size(total_size),
        _current_size(0),_seq(1),_md5(md5), _last_confirmed_seq(0),_rsp_size(0), _transfer_state(TransferState::None),
        _transfer_type(TransferType::None)
    {
        _max_seq = ((total_size + MAX_FILE_LEN - 1) / MAX_FILE_LEN);
    }

    MsgType _msg_type;   //消息类型, 文本，图片，视频，文件
    QString _text_or_url;//表示文件和图像的url,文本信息
    QPixmap _preview_pix;//文件和图片的缩略图
    QString _unique_name; //文件唯一名字
    qint64 _total_size; //文件总大小
    qint64 _current_size; //传输大小
    qint64 _seq;          //传输序号
    QString _md5;         //文件md5
    std::set<qint64> _rsp_seqs;      //已经接受的回传序列集合
    std::set<qint64> _flighting_seqs;  //正在发送，但是未收到服务器回复，将来用来做超时重传
    qint64 _last_confirmed_seq;      //最后确认序列
    qint64 _max_seq;                //最大序列号
    qint64 _msg_id;                 //关联的消息id
    qint64 _rsp_size;  //服务器返回实际上传或者下载的大小
    qint64 _thread_id;             // 会话id
    TransferState _transfer_state;  //上传或者下载, 暂停，传输完成
    TransferType  _transfer_type;   //文件类型, 上传或者下载
    
};
//声明为元对象类型
Q_DECLARE_METATYPE(MsgInfo)
Q_DECLARE_METATYPE(std::shared_ptr<MsgInfo>)

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
    READED = 2,  //对方已读
    UN_UPLOAD = 3 //未上传完成
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

struct DownloadInfo {
    QString _name;
    int _total_size;
    int _current_size;
    int _seq;
    QString _client_path;
};

extern QString calculateFileHash(const QString& filePath);

#endif // GLOBAL_H
