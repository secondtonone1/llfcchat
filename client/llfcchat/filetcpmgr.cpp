#include "filetcpmgr.h"
#include "usermgr.h"
FileTcpMgr::FileTcpMgr(QObject *parent) : QObject(parent),
_host(""), _port(0), _b_recv_pending(false), _message_id(0), _message_len(0), _bytes_sent(0), _pending(false)
{
    registerMetaType();
    QObject::connect(&_socket, &QTcpSocket::connected, this, [&]() {
           qDebug() << "Connected to server!";
           emit sig_con_success(true);
       });


    QObject::connect(&_socket, &QTcpSocket::readyRead, this, [&]() {
        // 当有数据可读时，读取所有数据
        // 读取所有数据并追加到缓冲区
        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);

        forever {
             //先解析头部
            if(!_b_recv_pending){
                // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
                if (_buffer.size() < FILE_UPLOAD_HEAD_LEN) {
                    return; // 数据不够，等待更多数据
                }

                // 预读取消息ID和消息长度，但不从缓冲区中移除
                stream >> _message_id >> _message_len;

                //将buffer 中的前六个字节移除
                _buffer = _buffer.mid(FILE_UPLOAD_HEAD_LEN);

                // 输出读取的数据
                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_len;

            }

             //buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
            if(_buffer.size() < _message_len){
                 _b_recv_pending = true;
                 return;
            }

            _b_recv_pending = false;
            // 读取消息体
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "receive body msg is " << messageBody ;

            _buffer = _buffer.mid(_message_len);
            handleMsg(ReqId(_message_id),_message_len, messageBody);
        }

    });


    //5.15 之后版本
//       QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
//           Q_UNUSED(socketError)
//           qDebug() << "Error:" << _socket.errorString();
//       });

    // 处理错误（适用于Qt 5.15之前的版本）
     QObject::connect(&_socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
                         this,
                         [&](QTcpSocket::SocketError socketError) {
            qDebug() << "Error:" << _socket.errorString() ;
            //todo... 根据错误类型做不同的处理
            switch (socketError) {
                case QTcpSocket::ConnectionRefusedError:
                    qDebug() << "Connection Refused!";
                    emit sig_con_success(false);
                    break;
                case QTcpSocket::RemoteHostClosedError:
                    qDebug() << "Remote Host Closed Connection!";
                    break;
                case QTcpSocket::HostNotFoundError:
                    qDebug() << "Host Not Found!";
                    emit sig_con_success(false);
                    break;
                case QTcpSocket::SocketTimeoutError:
                    qDebug() << "Connection Timeout!";
                    emit sig_con_success(false);
                    break;
                case QTcpSocket::NetworkError:
                    //qDebug() << "Network Error!";
                    break;
                default:
                    //qDebug() << "Other Error!";
                    break;
            }
      });

     // 处理连接断开
     QObject::connect(&_socket, &QTcpSocket::disconnected, this,[&]() {
         qDebug() << "Disconnected from server.";
         emit sig_connection_closed();
     });


     //连接发送信号用来发送数据
     QObject::connect(this, &FileTcpMgr::sig_send_data, this, &FileTcpMgr::slot_send_data);

     //连接发送信号
     QObject::connect(&_socket, &QTcpSocket::bytesWritten, this, [this](qint64 bytes) {
                  //更新发送数据
                 _bytes_sent += bytes;
                 //未发送完整
                 if (_bytes_sent < _current_block.size()) {
                     //继续发送
                     auto data_to_send = _current_block.mid(_bytes_sent);
                     _socket.write(data_to_send);
                     return;
                 }

                 //发送完全，则查看队列是否为空
                 if (_send_queue.isEmpty()) {
                     //队列为空，说明已经将所有数据发送完成，将pending设置为false，这样后续要发送数据时可以继续发送
                     _current_block.clear();
                     _pending = false;
                     _bytes_sent = 0;
                     return;
                 }

                 //队列不为空，则取出队首元素
                 _current_block = _send_queue.dequeue();
                 _bytes_sent = 0;
                 _pending = true;
                 qint64 w2 = _socket.write(_current_block);
                 qDebug() << "[TcpMgr] Dequeued and write() returned" << w2;
         });

     //连接
     QObject::connect(this, &FileTcpMgr::sig_close, this, &FileTcpMgr::slot_tcp_close);
     //注册消息
     initHandlers();

}

void FileTcpMgr::registerMetaType()
{
    // 注册所有自定义类型
    qRegisterMetaType<ServerInfo>("ServerInfo");
    qRegisterMetaType<std::shared_ptr<ServerInfo>>("std::shared_ptr<ServerInfo>");
    qRegisterMetaType<SearchInfo>("SearchInfo");
    qRegisterMetaType<std::shared_ptr<SearchInfo>>("std::shared_ptr<SearchInfo>");

    qRegisterMetaType<AddFriendApply>("AddFriendApply");
    qRegisterMetaType<std::shared_ptr<AddFriendApply>>("std::shared_ptr<AddFriendApply>");

    qRegisterMetaType<ApplyInfo>("ApplyInfo");

    qRegisterMetaType<std::shared_ptr<AuthInfo>>("std::shared_ptr<AuthInfo>");

    qRegisterMetaType<AuthRsp>("AuthRsp");
    qRegisterMetaType<std::shared_ptr<AuthRsp>>("std::shared_ptr<AuthRsp>");

    qRegisterMetaType<UserInfo>("UserInfo");

    qRegisterMetaType<std::vector<std::shared_ptr<TextChatData>>>("std::vector<std::shared_ptr<TextChatData>>");

    qRegisterMetaType<std::vector<std::shared_ptr<ChatThreadInfo>>>("std::vector<std::shared_ptr<ChatThreadInfo>>");

    qRegisterMetaType<std::shared_ptr<ChatThreadData>>("std::shared_ptr<ChatThreadData>");
    qRegisterMetaType<ReqId>("ReqId");
}

void FileTcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter =  _handlers.find(id);
    if(find_iter == _handlers.end()){
         qDebug()<< "not found id ["<< id << "] to handle";
         return ;
    }

    find_iter.value()(id,len,data);
}

void FileTcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
{
    uint16_t id = reqId;

    // 计算长度（使用网络字节序转换）
    quint32 len = static_cast<quint32>(dataBytes.length());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    // 添加字符串数据
    block.append(dataBytes);

    //判断是否正在发送
    if (_pending) {
        //放入队列直接返回，因为目前有数据正在发送
        _send_queue.enqueue(block);
        return;
    }

    // 没有正在发送，把这包设为“当前块”，重置计数，并写出去
    _current_block = block;        // ← 保存当前正在发送的 block
    _bytes_sent = 0;            // ← 归零
    _pending = true;         // ← 标记正在发送

    qint64 written = _socket.write(_current_block);
   /* qDebug() << "tcp mgr send byte data is" << _current_block
        << ", write() returned" << written;*/
}

void FileTcpMgr::slot_tcp_connect(std::shared_ptr<ServerInfo> si)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    _host = si->_res_host;
    _port = static_cast<uint16_t>(si->_res_port.toUInt());
    _socket.connectToHost(_host, _port);
}


FileTcpMgr::~FileTcpMgr(){

}

void FileTcpMgr::SendData(ReqId reqId, QByteArray data)
{
    emit sig_send_data(reqId, data);
}

void FileTcpMgr::initHandlers()
{
    //todo 接收上传用户头像回复
    _handlers.insert(ID_UPLOAD_HEAD_ICON_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug()<< "handle id is "<< id ;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
           qDebug() << "Failed to create QJsonDocument.";
           return;
        }

        QJsonObject recvObj = jsonDoc.object();
        qDebug()<< "data jsonobj is " << recvObj ;

        if(!recvObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "icon upload_failed, err is Json Parse Err" << err ;
            //todo ... 提示上传失败
            //emit upload_failed();
            return;
        }

        int err = recvObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            //emit upload_failed();
            return;
        }

        auto md5 = recvObj["md5"].toString();
        auto seq = recvObj["seq"].toInt();
        auto trans_size = recvObj["trans_size"].toInt();
        auto uid = recvObj["uid"].toInt();
        auto total_size = recvObj["total_size"].toInt();
        auto name = recvObj["name"].toString();

        qDebug() << "recv : " << name  << "file trans_size is " << trans_size;
        //判断trans_size和total_size相等
        if(total_size == trans_size){
            UserMgr::GetInstance()->RmvUploadFile(name);
            return;
        }

        auto file_info = UserMgr::GetInstance()->GetUploadInfoByName(name);
        if(!file_info){
            return;
        }

        //再次组织数据发送
        QFile file(file_info->filePath());
        if(!file.open(QIODevice::ReadOnly)){
            qWarning() << "Could not open file: " << file.errorString();
            return;
        }

        //文件偏移到已经发送的位置，继续读取发送
        file.seek(trans_size);
        QByteArray buffer;
        seq ++;
        //每次读取2048字节发送
        buffer = file.read(MAX_FILE_LEN);
        QJsonObject sendObj;
        //将文件内容转换为base64编码
        QString base64Data = buffer.toBase64();
        sendObj["md5"] = md5;
        sendObj["name"] = name;
        sendObj["seq"] = seq;
        sendObj["trans_size"] = buffer.size() + (seq-1)*MAX_FILE_LEN;
        sendObj["total_size"] = total_size;

        if(buffer.size() + (seq-1)*MAX_FILE_LEN >= total_size){
            sendObj["last"] = 1;
        }else{
            sendObj["last"] = 0;
        }

        sendObj["data"] = base64Data;
        sendObj["last_seq"] = recvObj["last_seq"].toInt();
        sendObj["uid"] = uid;
        QJsonDocument doc(sendObj);
        auto send_data = doc.toJson();
        SendData(ID_UPLOAD_HEAD_ICON_REQ, send_data);

        file.close();
    });

    _handlers.insert(ID_DOWN_LOAD_FILE_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "parse create private chat json parse failed " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "get create private chat failed, error is " << err;
            return;
        }

        qDebug() << "Receive download file info rsp success";

        QString base64Data = jsonObj["data"].toString();
        QString clientPath = jsonObj["client_path"].toString();
        int seq = jsonObj["seq"].toInt();
        bool is_last = jsonObj["is_last"].toBool();
        QString total_size_str = jsonObj["total_size"].toString();
        qint64  total_size = total_size_str.toLongLong(nullptr);
        QString current_size_str = jsonObj["current_size"].toString();
        qint64  current_size = current_size_str.toLongLong(nullptr);
        QString name = jsonObj["name"].toString();

        auto file_info = UserMgr::GetInstance()->GetDownloadInfo(name);
        if (file_info == nullptr) {
            qDebug() << "file: " << name << " not found";
            return;
        }

        file_info->_current_size = current_size;
        file_info->_total_size = total_size;

        //Base64解码
        QByteArray decodedData = QByteArray::fromBase64(base64Data.toUtf8());
        QFile file(clientPath);

        // 根据 seq 决定打开模式
        QIODevice::OpenMode mode;
        if (seq == 1) {
            // 第一个包，覆盖写入
            mode = QIODevice::WriteOnly;
        }
        else {
            // 后续包，追加写入
            mode = QIODevice::WriteOnly | QIODevice::Append;
        }

        if (!file.open(mode)) {
            qDebug() << "Failed to open file for writing:" << clientPath;
            qDebug() << "Error:" << file.errorString();
            return;
        }


        qint64 bytesWritten = file.write(decodedData);
        if (bytesWritten != decodedData.size()) {
            qDebug() << "Failed to write all data. Written:" << bytesWritten
                << "Expected:" << decodedData.size();
        }

        file.close();

        qDebug() << "Successfully wrote" << bytesWritten << "bytes to file";
        qDebug() << "Progress:" << current_size << "/" << total_size
            << "(" << (current_size * 100 / total_size) << "%)";

        if (is_last) {
            qDebug() << "File download completed:" << clientPath;
            UserMgr::GetInstance()->RmvDownloadFile(name);
            //发送信号通知主界面重新加载label
            emit sig_reset_label_icon(clientPath);
        }
        else {
            //继续请求
            file_info->_seq = seq+1;
            FileTcpMgr::GetInstance()->SendDownloadInfo(file_info);
        }
    });
}


void FileTcpMgr::slot_tcp_close() {
    _socket.close();
}

void FileTcpMgr::CloseConnection(){
    emit sig_close();
}

void FileTcpMgr::SendDownloadInfo(std::shared_ptr<DownloadInfo> download) {
    QJsonObject jsonObj;
    jsonObj["name"] = download->_name;
    jsonObj["seq"] = download->_seq;
    jsonObj["trans_size"] = 0;
    jsonObj["total_size"] = 0;
    jsonObj["token"] = UserMgr::GetInstance()->GetToken();
    jsonObj["uid"] = UserMgr::GetInstance()->GetUid();
    jsonObj["client_path"] = download->_client_path;

    QJsonDocument doc(jsonObj);
    auto send_data = doc.toJson();

    SendData(ID_DOWN_LOAD_FILE_REQ, send_data);
}


FileTcpThread::FileTcpThread()
{
    _file_tcp_thread = new QThread();
    FileTcpMgr::GetInstance()->moveToThread(_file_tcp_thread);
    QObject::connect(_file_tcp_thread, &QThread::finished, _file_tcp_thread, &QObject::deleteLater);
    _file_tcp_thread->start();
}

FileTcpThread::~FileTcpThread()
{
    _file_tcp_thread->quit();
}
