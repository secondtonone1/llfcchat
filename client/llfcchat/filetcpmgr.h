#ifndef FILETCPMGR_H
#define FILETCPMGR_H

#include <QTcpSocket>
#include "singleton.h"
#include "global.h"
#include <functional>
#include <QObject>
#include "userdata.h"
#include <QJsonArray>
#include <memory>
#include <QThread>
#include <QQueue>


class FileTcpThread: public std::enable_shared_from_this<FileTcpThread>{
public:
    FileTcpThread();
    ~FileTcpThread();
private:
    QThread * _file_tcp_thread;

};

class FileTcpMgr : public QObject, public Singleton<FileTcpMgr>,
        public std::enable_shared_from_this<FileTcpMgr>
{
    Q_OBJECT
public:
    friend class Singleton<FileTcpMgr>;
    ~FileTcpMgr();
    void SendData(ReqId reqId, QByteArray data);
    void CloseConnection();
    void SendDownloadInfo(std::shared_ptr<DownloadInfo> download);
private:
    void initHandlers();
    explicit FileTcpMgr(QObject *parent = nullptr);

    void registerMetaType();
    void handleMsg(ReqId id, int len, QByteArray data);

    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint32 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
    //发送队列
    QQueue<QByteArray> _send_queue;
    //正在发送的包
    QByteArray  _current_block;
    //当前已发送的字节数
    qint64        _bytes_sent;
    //是否正在发送
    bool _pending;
signals:
    void sig_close();
     void sig_send_data(ReqId reqId, QByteArray data);
     void sig_con_success(bool bsuccess);
     void sig_connection_closed();
     void sig_reset_label_icon(QString path);
public slots:
    void slot_send_data(ReqId reqId, QByteArray data);
    void slot_tcp_connect(std::shared_ptr<ServerInfo> si);
    void slot_tcp_close();
};

#endif // FILETCPMGR_H
