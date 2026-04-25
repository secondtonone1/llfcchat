---
title: 断点续传
date: 2025-07-29 10:15:50
tags: [C++聊天项目]
categories: [C++聊天项目]  
---

## 独立网络线程



### 独立前

![image-20250803111220495](https://cdn.llfc.club/img/image-20250803111220495.png)

### 独立后





![image-20250803111620510](https://cdn.llfc.club/img/image-20250803111620510.png)

### 槽函数连接方式

![image-20250803114247485](https://cdn.llfc.club/img/image-20250803114247485.png)





1. 直接连接

   `connect(发送者, 信号， [](信号参数){})`;

​	   这种槽函数在发送者所在线程触发。

2. 增加接收者

   `connect(发送者,信号,接收者,槽函数)`

   如果发送者和接收者在同一个线程，则槽函数调用的线程就是发送者所在的线程。

3. 发送者和接收者不在一个线程，connect默认采用队列连接方式

​      `connect(发送者,信号,接收者,槽函数)`

​       槽函数在接收者所在的线程触发。好处就是解耦合。



### 元对象系统

1. 信号和槽
2. 反射
3. 动态增加函数和属性

当我们信号和槽连接方式采用队列连接，那么信号的参数会被封装为元对象，投递到队列中。

要想支持元对象有两种方式

1. 继承于QObject，并且类内填写Q_OBJECT宏
2. 声明并且注册元对象类



为了支持高并发情况下断点续传，考虑将目前项目中`TcpMgr`中网络模块独立到独立线程

**封装`TcpThread`类**

利用`RAII`思想封装线程启动和回收

``` cpp
class TcpThread:public std::enable_shared_from_this<TcpThread> {
public:
    TcpThread();
    ~TcpThread();
private:
    QThread* _tcp_thread;
};
```

具体实现

``` cpp
TcpThread::TcpThread()
{
    _tcp_thread = new QThread();
    TcpMgr::GetInstance()->moveToThread(_tcp_thread);
    QObject::connect(_tcp_thread, &QThread::finished, _tcp_thread, &QObject::deleteLater);

    _tcp_thread->start();
}

TcpThread::~TcpThread()
{
    _tcp_thread->quit();
}

```

主函数启动时记得提前启动线程，将`TcpMgr`转移到独立线程中

``` cpp
    //启动tcp线程
    TcpThread tcpthread;
    MainWindow w;
    w.show();
    return a.exec();
```



测试发现，登录卡住，检测是信号`sig_connect_tcp`发送了，槽函数`slot_tcp_connect`没触发。

``` cpp
 //连接tcp连接请求的信号和槽函数
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);
```



为了测试

先在`TcpMgr`中添加测试槽函数

``` cpp
    void slot_test() {
        qDebug() << "receve thread is " << QThread::currentThread();
        qDebug() << "slot test......";
    }
```

在`LoginDialog`中连接信号

``` cpp
connect(this, &LoginDialog::sig_test, TcpMgr::GetInstance().get(), &TcpMgr::slot_test);
```

在发送`sig_connect_tcp`处发送`sig_test`

``` cpp
void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            enableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();

        //发送信号通知tcpMgr发送长链接
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();

        _uid = si.Uid;
        _token = si.Token;
        qDebug()<< "email is " << email << " uid is " << si.Uid <<" host is "
                << si.Host << " Port is " << si.Port << " Token is " << si.Token;
        emit sig_connect_tcp(si);
        emit sig_test();
    });
}
```

测试，是可以看到能触发slot_test函数得，而且线程id显示是子线程中触发得槽函数。

那么`sig_connect_tcp`信号发出，没有触发槽函数，就是因为信号得参数类型不支持元对象系统。

为了支持元对象系统，我们需要在信号的参数`ServerInfo`类实现默认构造，同时声明为元对象类型

``` cpp
struct ServerInfo{
public:
    ServerInfo() = default;
    ServerInfo(const ServerInfo& other):Host(other.Host),Port(other.Port),Token(other.Token),Uid(other.Uid){}
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

Q_DECLARE_METATYPE(ServerInfo)
```

在`TcpMgr`中注册这个元对象类型

``` cpp
qRegisterMetaType<ServerInfo>("ServerInfo");
```

再次测试就通过登录了，但是在发送后续得消息时，又遇到了自定义类型作为参数得情况，我们需要和上面一样，依次声明元对象类型并且注册。

如下列举一个，还有很多，不再详细列举

``` cpp
class SearchInfo {
public:
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon);
    SearchInfo() = default;
	int _uid;
	QString _name;
	QString _nick;
	QString _desc;
	int _sex;
    QString _icon;
};

Q_DECLARE_METATYPE(SearchInfo)
Q_DECLARE_METATYPE(std::shared_ptr<SearchInfo>)

```

`TcpMgr`封装注册元对象函数

``` cpp
void TcpMgr::registerMetaType() {
    // 注册所有自定义类型
    qRegisterMetaType<ServerInfo>("ServerInfo");
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
```

在构造函数中调用

``` cpp
TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0)
{
    registerMetaType();
    //...
}
```

再次测试就通过了

这里给大家讲讲为什么单线程情况下，信号可以携带自定义类型作为参数，不用设定元对象就可以传输，而跨线程不可以。

在 Qt 的信号/槽机制中，信号参数的传递方式取决于连接（`connect`）的类型，而连接类型又由发信号对象和接收槽对象所在的线程决定：

1. **同线程（Direct Connection）**

   - 如果信号和槽都在同一个线程里，默认使用 **Direct Connection**。
   - Direct Connection 本质上就是一个普通的 C++ 函数调用，参数直接按值或按引用传递，编译时就已经知道了类型，不需要任何额外的元类型信息。
   - 因此，即使你没有把 `SearchInfo` 注册为 `QMetaType`，编译器也能直接生成函数调用代码，信号里就可以直接传递 `SearchInfo`。

2. **跨线程（Queued Connection）**

   - 如果信号发送者和接收者不在同一个线程，Qt 会自动把连接转成 **Queued Connection**。

   - Queued Connection 的实现是：当信号发出时，Qt 会把信号参数打包成一个事件（`QEvent`），然后把事件放到目标线程的事件队列里；目标线程的事件循环（`QCoreApplication::processEvents()`）再把这个事件取出来，调用槽函数。

   - 这里的“打包”与“解包”就需要运行时才能确定参数类型，以及如何拷贝或序列化这个类型——这正是 Qt 元对象系统（`QMetaType`）要干的事情。

   - 如果没有把 `SearchInfo` 声明成一个元类型，Qt 就不知道如何在内部把它从一个线程“打包”到事件里，又如何在另一线程里还原。

   - 因此，**跨线程传递自定义类型**，必须在类型定义后加上：

     ```cpp
     Q_DECLARE_METATYPE(SearchInfo)
     ```

     并在运行时注册（通常在 `main()` 里调用一次）：

     ```cpp
     qRegisterMetaType<SearchInfo>("SearchInfo");
     ```

------

**小结**

- **同线程**：Direct Connection，编译时直接调用，**不需要** `Q_DECLARE_METATYPE`。
- **跨线程**：Queued Connection，需要运行时打包/解包参数，**必须**用 `Q_DECLARE_METATYPE`（以及 `qRegisterMetaType`）来注册你的自定义类型。



### 添加发送队列



### `UserMgr`线程安全

为了保证多线程情况下访问数据的安全性，对`UserMgr`类的操作加锁

``` cpp
 std::mutex _mtx;
```

在获取数据和设置数据的地方都进行加锁, 比如

``` cpp
std::shared_ptr<UserInfo> UserMgr::GetUserInfo()
{
    std::lock_guard<std::mutex> lock(_mtx);
    return _user_info;
}
```

还有很多不再赘述



## 设置发送队列

默认情况下`qt`的`socket`都是非阻塞的。

所以调用`socket.write(数据)`可能会直接返回`-1`

返回`-1`表示网络出错，一般都是`EWOULD_BLOCK/EAGAIN`造成的。表示发送缓冲区已经满了，无法继续发送。

而我们之前的逻辑，无论在哪个线程，想要发送数据，统一发送信号

``` cpp
 void sig_send_data(ReqId reqId, QByteArray data);
```

会触发`TcpMgr`的槽函数

``` cpp
void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
{
    uint16_t id = reqId;

    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(dataBytes.length());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    // 添加字符串数据
    block.append(dataBytes);

   
    qint64 written = _socket.write(block);
    qDebug() << "tcp mgr send byte data is" << _current_block
        << ", write() returned" << written;
}
```

上述函数在网络情况良好的时候不会产生问题，但是如果网络发送情况频繁的时候，就容易出现written为-1的情况。

也就是发送缓冲区满了，导致发送失败。

对于这种情况，我们可以模仿我们的服务器写法，添加一个发送队列，然后将要发送的数据投递到发送队列

``` cpp
    //发送队列
    QQueue<QByteArray> _send_queue;
    //正在发送的包
    QByteArray  _current_block;
    //当前已发送的字节数
    qint64        _bytes_sent;
    //是否正在发送
    bool _pending;
```

修改发送逻辑

``` cpp
void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
{
    uint16_t id = reqId;

    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(dataBytes.length());

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
    qDebug() << "tcp mgr send byte data is" << _current_block
        << ", write() returned" << written;
}
```

我们需要监听发送返回的数据，`QT`也提供了类似于`asio`的异步回调功能，只是在发送完成后返回一个信号`void bytesWritten(qint64 bytes);`

我们连接这个信号

``` cpp
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
```

`_pending`控制发送还是放入队列。



## 断点续传思路

### 思路图



![image-20250810113255940](https://cdn.llfc.club/img/image-20250810113255940.png)



### 修改上传逻辑

原来的传输逻辑，采用的是循环上传，就是将一个文件拆分成多个报文段，循环上传，而不等待服务器每次回复

``` cpp
void MainWindow::on_uploadBtn_clicked()
{
    ui->uploadBtn->setEnabled(false);
    // 打开文件
       QFile file(_file_name);
       if (!file.open(QIODevice::ReadOnly)) {
           qWarning() << "Could not open file:" << file.errorString();
           return;
       }

       // 保存当前文件指针位置
       qint64 originalPos = file.pos();
       QCryptographicHash hash(QCryptographicHash::Md5);
       if (!hash.addData(&file)) {
           qWarning() << "Failed to read data from file:" << _file_name;
           return ;
       }

       _file_md5 = hash.result().toHex(); // 返回十六进制字符串

    // 读取文件内容并发送
    QByteArray buffer;
    int seq = 0;

    QFileInfo fileInfo(_file_name); // 创建 QFileInfo 对象

    QString fileName = fileInfo.fileName(); // 获取文件名
    qDebug() << "文件名是：" << fileName; // 输出文件名
    int total_size = fileInfo.size();
    int last_seq = 0;
    if(total_size % MAX_FILE_LEN){
        last_seq = (total_size/MAX_FILE_LEN)+1;
    }else{
        last_seq = total_size/MAX_FILE_LEN;
    }

    // 恢复文件指针到原来的位置
    file.seek(originalPos);

    while (!file.atEnd()) {
        //每次读取2048字节发送
        buffer = file.read(MAX_FILE_LEN);
        QJsonObject jsonObj;
        // 将文件内容转换为 Base64 编码（可选）
        QString base64Data = buffer.toBase64();
        //qDebug() << "send data is " << base64Data;
        ++seq;
        jsonObj["md5"] = _file_md5;
        jsonObj["name"] = fileName;
        jsonObj["seq"] = seq;
        jsonObj["trans_size"] = buffer.size() + (seq-1)*MAX_FILE_LEN;
        jsonObj["total_size"] = total_size;

        if(buffer.size() + (seq-1)*MAX_FILE_LEN == total_size){
            jsonObj["last"] = 1;
        }else{
            jsonObj["last"] = 0;
        }

        jsonObj["data"]= base64Data;
        jsonObj["last_seq"] = last_seq;
        QJsonDocument doc(jsonObj);
        auto send_data = doc.toJson();
        TcpClient::Inst().sendMsg(ID_UPLOAD_FILE_REQ, send_data);
        //startDelay(500);
     }

    //关闭文件
    file.close();

}
```

现在需要改为分段上传，每次上传后，等待服务器返回响应后再上传下一个

``` cpp
void MainWindow::on_uploadBtn_clicked()
{
    ui->uploadBtn->setEnabled(false);
    ui->pauseBtn->setEnabled(true);
    // 打开文件
       QFile file(_file_name);
       if (!file.open(QIODevice::ReadOnly)) {
           qWarning() << "Could not open file:" << file.errorString();
           return;
       }

       // 保存当前文件指针位置
       qint64 originalPos = file.pos();
       QCryptographicHash hash(QCryptographicHash::Md5);
       if (!hash.addData(&file)) {
           qWarning() << "Failed to read data from file:" << _file_name;
           return ;
       }

       _file_md5 = hash.result().toHex(); // 返回十六进制字符串

    // 读取文件内容并发送
    QByteArray buffer;
    int seq = 0;

    // 创建 QFileInfo 对象
    auto fileInfo = std::make_shared<QFileInfo>(_file_name);

    QString fileName = fileInfo->fileName(); // 获取文件名
    qDebug() << "文件名是：" << fileName; // 输出文件名
    int total_size = fileInfo->size();
    int last_seq = 0;
    if(total_size % MAX_FILE_LEN){
        last_seq = (total_size/MAX_FILE_LEN)+1;
    }else{
        last_seq = total_size/MAX_FILE_LEN;
    }

    // 恢复文件指针到原来的位置
    file.seek(originalPos);

    //改为读取第一块并发送
    //每次读取2048字节发送
    buffer = file.read(MAX_FILE_LEN);
    QJsonObject jsonObj;
    // 将文件内容转换为 Base64 编码（可选）
    QString base64Data = buffer.toBase64();
    //qDebug() << "send data is " << base64Data;
    ++seq;
    jsonObj["md5"] = _file_md5;
    jsonObj["name"] = fileName;
    jsonObj["seq"] = seq;
    jsonObj["trans_size"] = buffer.size() + (seq-1)*MAX_FILE_LEN;
    jsonObj["total_size"] = total_size;

    if(buffer.size() + (seq-1)*MAX_FILE_LEN == total_size){
        jsonObj["last"] = 1;
    }else{
        jsonObj["last"] = 0;
    }

    jsonObj["data"]= base64Data;
    jsonObj["last_seq"] = last_seq;
    QJsonDocument doc(jsonObj);
    auto send_data = doc.toJson();
    TcpClient::Inst().sendMsg(ID_UPLOAD_FILE_REQ, send_data);
    LogicMgr::Inst()->AddMD5File(_file_md5, fileInfo);
    //关闭文件
    file.close();

}
```

### 收到响应后续传

当客户端收到服务器的回包后，解析后传递给`LogicMgr`, `LogicMgr`中需要将后续的报文段发送给服务器。我们封装如下逻辑

``` cpp
void LogicWorker::InitHandlers()
{
    //注册上传消息
    _handlers[ID_UPLOAD_FILE_RSP] = [this](QJsonObject obj){
        auto err = obj["error"].toInt();
        if(err != RSP_SUCCESS){
            qDebug() << "upload msg rsp err is " << err;
            return;
        }

        auto name = obj["name"].toString();
        auto total_size = obj["total_size"].toInt();
        auto trans_size = obj["trans_size"].toInt();
        auto md5 = obj["md5"].toString();
        auto seq = obj["seq"].toInt();

        qDebug() << "recv : " << name << " file trans_size is " << trans_size;
        emit sig_trans_size(trans_size);

        //判断trans_size是否和total_size相等
        if(total_size == trans_size){
            return;
        }

        auto file_info = LogicMgr::Inst()->GetFileInfo(md5);
        if(!file_info){
            return;
        }
        //再次组织数据发送
        QFile file(file_info->filePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open file:" << file.errorString();
            return;
        }

        //文件偏移到已经发送的位置，继续读取发送
        file.seek(trans_size);

        if(LogicMgr::Inst()->Pause()){
            return ;
        }
        QByteArray buffer;
        seq++;
        //每次读取2048字节发送
        buffer = file.read(MAX_FILE_LEN);
        QJsonObject jsonObj;
        // 将文件内容转换为 Base64 编码（可选）
        QString base64Data = buffer.toBase64();

        jsonObj["md5"] = md5;
        jsonObj["name"] = file_info->fileName();
        jsonObj["seq"] = seq;
        jsonObj["trans_size"] = buffer.size() + (seq-1)*MAX_FILE_LEN;
        jsonObj["total_size"] = total_size;

        if(buffer.size() + (seq-1)*MAX_FILE_LEN == total_size){
            jsonObj["last"] = 1;
        }else{
            jsonObj["last"] = 0;
        }

        jsonObj["data"]= base64Data;
        jsonObj["last_seq"] = obj["last_seq"].toInt();
        QJsonDocument doc(jsonObj);
        auto send_data = doc.toJson();
        TcpClient::Inst().sendMsg(ID_UPLOAD_FILE_REQ, send_data);

        file.close();
    };

}
```

其中`sig_trans_size`为信号，通知主界面显示进度

### 暂停和续传

客户端需增加暂停和续传按钮，支持传说过程中暂停，点击后再继续上传等功能

``` cpp
void MainWindow::slot_pause_continue()
{
    //续传状态或者初始状态，按下暂停按钮
    if(_cur_state == INIT || _cur_state == CONTINUE){
        //设置当前状态为暂停状态
        _b_pause = true;
        ui->pauseBtn->setText("继续");
        _cur_state = PAUSE;
        LogicMgr::Inst()->SetPause(true);
        return;
    }

    //判断当前为暂停状态，则点击后开启续传
    if(_cur_state == PAUSE){
        _b_pause = false;
        ui->pauseBtn->setText("暂停");
        _cur_state = CONTINUE   ;
        LogicMgr::Inst()->SetPause(false);
        //发送请求获取文件信息，继续上传
        auto file_info = LogicMgr::Inst()->GetFileInfo(_file_md5);

        QJsonObject jsonObj;

        jsonObj["md5"] = _file_md5;

        QJsonDocument doc(jsonObj);
        auto send_data = doc.toJson();
        TcpClient::Inst().sendMsg(ID_SYNC_FILE_REQ, send_data);

        return;
    }
}

```

这里继续上传需要请求一下服务器，同步之前的上传进度。

我们添加了新的协议`ID_SYNC_FILE_REQ`, 服务器收到后将状态和进度返回，客户端响应

``` cpp
  _handlers[ID_SYNC_FILE_RSP] = [this](QJsonObject obj){
        auto err = obj["error"].toInt();
        if(err != RSP_SUCCESS){
            qDebug() << " msg rsp err is " << err;
            return;
        }

        auto md5 = obj["md5"].toString();
        auto seq = obj["seq"].toInt();
        auto total_size = obj["total_size"].toInt();

        auto file_info = LogicMgr::Inst()->GetFileInfo(md5);
        if(!file_info){
            qDebug() << "not found file" ;
            return;
        }

        //再次组织数据发送
        QFile file(file_info->filePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open file:" << file.errorString();
            return;
        }

        auto trans_size = obj["trans_size"].toInt();

        //文件偏移到已经发送的位置，继续读取发送
        file.seek(trans_size);

        if(LogicMgr::Inst()->Pause()){
            return ;
        }
        QByteArray buffer;
        seq++;
        //每次读取2048字节发送
        buffer = file.read(MAX_FILE_LEN);
        QJsonObject jsonObj;
        // 将文件内容转换为 Base64 编码（可选）
        QString base64Data = buffer.toBase64();

        jsonObj["md5"] = md5;
        jsonObj["name"] = file_info->fileName();
        jsonObj["seq"] = seq;
        jsonObj["trans_size"] = buffer.size() + (seq-1)*MAX_FILE_LEN;
        jsonObj["total_size"] = total_size;

        if(buffer.size() + (seq-1)*MAX_FILE_LEN == total_size){
            jsonObj["last"] = 1;
        }else{
            jsonObj["last"] = 0;
        }

        jsonObj["data"]= base64Data;
        jsonObj["last_seq"] = obj["last_seq"].toInt();
        QJsonDocument doc(jsonObj);
        auto send_data = doc.toJson();
        TcpClient::Inst().sendMsg(ID_UPLOAD_FILE_REQ, send_data);

        file.close();
    };
```

客户端根据返回的进度，按照偏移量读取指定文件，并且继续上报。

如果健壮一点，可以判断服务器返回的错误信息，根据错误，提示主界面做出交互显示等。这里不再赘述。

到此客户端设计完成。

### 单线程服务器改造

单线程服务器改造不大，只需要增加同步文件进度信息的处理逻辑，以及优化之前的上传处理逻辑即可

``` cpp
  _fun_callbacks[ID_UPLOAD_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto data = root["data"].asString();
			//std::cout << "recv file data is  " << data << std::endl;

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_FILE_RSP);
				});

			// 解码
			std::string decoded = base64_decode(data);

			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / name).string();
			std::cout << "file_path_str is " << file_path_str << std::endl;

			if (seq != 1) {
				auto iter = _map_md5_files.find(md5);
				if (iter == _map_md5_files.end()) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					return;
				}
			}


			std::ofstream outfile;
			//第一个包
			if (seq == 1) {
				// 打开文件，如果存在则清空，不存在则创建
				outfile.open(file_path_str, std::ios::binary | std::ios::trunc);
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				std::lock_guard<std::mutex> lock(_file_mtx);
				_map_md5_files[md5] = file_info;
			}
			else {
				// 保存为文件
				outfile.open(file_path_str, std::ios::binary | std::ios::app);
				std::lock_guard<std::mutex> lock(_file_mtx);
				auto file_info = _map_md5_files[md5];
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
			}
			
			if (!outfile) {
				std::cerr << "无法打开文件进行写入。" << std::endl;
				return ;
			}

			outfile.write(decoded.data(), decoded.size());
			if (!outfile) {
				std::cerr << "写入文件失败。" << std::endl;
				return ;
			}

			outfile.close();
			std::cout << "文件已成功保存为: " << name <<  std::endl;

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = total_size;
			rtvalue["seq"] = seq;
			rtvalue["name"] = name;
			rtvalue["trans_size"] = trans_size;
			rtvalue["md5"] = md5;
	};


	_fun_callbacks[ID_SYNC_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {

			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_SYNC_FILE_RSP);
				});

			auto md5 = root["md5"].asString();

			auto iter = _map_md5_files.find(md5);
			if (iter == _map_md5_files.end()) {
				rtvalue["error"] = ErrorCodes::FileNotExists;
				return;
			}

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = iter->second->_total_size;
			rtvalue["seq"] = iter->second->_seq;
			rtvalue["name"] = iter->second->_name;
			rtvalue["trans_size"] = iter->second->_trans_size;
			rtvalue["md5"] = md5;

	};
```

### 多线程服务器

多线程服务器改造和单线程类似

只不过将处理逻辑放入`LogicWorker`中

``` cpp
void LogicWorker::RegisterCallBacks()
{
	_fun_callbacks[ID_TEST_MSG_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto data = root["data"].asString();
			std::cout << "recv test data is  " << data << std::endl;

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_TEST_MSG_RSP);
				});

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["data"] = data;
	};

	_fun_callbacks[ID_UPLOAD_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / name).string();
			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_FILE_RSP);
				});

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			//第一个包
			if (seq == 1) {
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				LogicSystem::GetInstance()->AddMD5File(md5, file_info);
			}
			else {
				auto file_info = LogicSystem::GetInstance()->GetFileInfo(md5);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
			}


			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, name, seq, total_size,
					trans_size, last, file_data),
				index
			);

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = total_size;
			rtvalue["seq"] = seq;
			rtvalue["name"] = name;
			rtvalue["trans_size"] = trans_size;
			rtvalue["last"] = last;
			rtvalue["md5"] = md5;
	};



	_fun_callbacks[ID_SYNC_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {

			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_SYNC_FILE_RSP);
				});

			auto md5 = root["md5"].asString();

			auto file = LogicSystem::GetInstance()->GetFileInfo(md5);
			if (file == nullptr) {
				rtvalue["error"] = ErrorCodes::FileNotExists;
				return;
			}

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = file->_total_size;
			rtvalue["seq"] = file->_seq;
			rtvalue["name"] = file->_name;
			rtvalue["trans_size"] = file->_trans_size;
			rtvalue["md5"] = md5;

	};
}
```

将进度信息存储在`LogicSystem`中，后续可参考填写入`redis`，方便后续分布式扩展，注意如果填写了多个资源服务器，还有写入服务器信息，这个不再赘述和进阶，我们只用一个资源服务器做演示，后续读者可自己进阶分布式设计。

``` cpp
void LogicSystem::AddMD5File(std::string md5, std::shared_ptr<FileInfo> fileinfo) {
	std::lock_guard<std::mutex> lock(_file_mtx);
	_map_md5_files[md5] = fileinfo;
}

std::shared_ptr<FileInfo> LogicSystem::GetFileInfo(std::string md5) {
	std::lock_guard<std::mutex> lock(_file_mtx);
	auto iter = _map_md5_files.find(md5);
	if (iter == _map_md5_files.end()) {
		return nullptr;
	}

	return iter->second;
}
```

## 集成资源服务器

### 新架构形式

集成资源服务器后的架构为

![image-20250905105415106](https://cdn.llfc.club/img/image-20250905105415106.png)

将上述多线程服务器，整合到项目目录，同时设置资源属性表，复用之前的就可以了。

注意资源服务器配置要稍作修改

``` cpp
[GateServer]
Port = 8080
[VarifyServer]
Host = 127.0.0.1
Port = 50051
[StatusServer]
Host = 127.0.0.1
Port = 50052
[SelfServer]
Name = reserver
Host = 0.0.0.0
Port  = 9090
RPCPort = 51055
[Mysql]
Host = 81.68.86.146
Port = 3308
User = root
Passwd = 123456.
Schema = llfc
[Redis]
Host = 81.68.86.146
Port = 6380
Passwd = 123456

[Static]
Path = static
[Output]
Path = bin

```



## 客户端新增资源网络类

### 构造函数解析

因为客户端需要长连接资源服务器，采用TCP方式上传文件，所以需要封装一个单例的`FileTcpMgr`类，用于上传资源。

``` cpp
class FileTcpMgr : public QObject, public Singleton<FileTcpMgr>,
        public std::enable_shared_from_this<FileTcpMgr>
{
    Q_OBJECT
public:
    friend class Singleton<FileTcpMgr>;
    ~FileTcpMgr();
    void SendData(ReqId reqId, QByteArray data);
    void CloseConnection();
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
     void sig_send_data(ReqId reqId, QByteArray data);
     void sig_con_success(bool bsuccess);
     void sig_connection_closed();
public slots:
    void slot_send_data(ReqId reqId, QByteArray data);
    void slot_tcp_connect(std::shared_ptr<ServerInfo> si);
};
```

构造函数具体实现

``` cpp
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
```

简单描述下上述构造函数做的事情：

1. 成功连接服务器后，会触发`QTcpSocket::connected`信号，从而回调`lambda`表达式，发送`sig_con_success`信号
2. 接收服务器传输的数据，会触发`QTcpSocket::readyRead`信号，从而回调`lambda`表达式，在这里处理头部信息和包体信息。进行`TLV`协议解析后回调`handleMsg`。
3. 捕获`QTcpSocket::SocketError`信号，当出错后回调`lambda`表达式发送信号通知主界面错误。
4. 捕获连接断开信号`QTcpSocket::disconnected`，回调`lambda`表达式，通知主界面连接断开。
5. 连接发送信号`sig_send_data`，因为`socket`在独立线程，不能直接调用发送，所以改为异步发送，触发槽函数`slot_send_data`
6. 因为异步发送，可能存在未发送完全的情况，所以我们用`QTcpSocket::bytesWritten`来检测发送了多少字节，通过`lambda`表达式回调处理，继续发送数据。
7. 因为`socket`被独立为单独线程，所以关闭也不能直接调用`close(socket)`， 需要统一在槽函数中处理。

注册元对象系统的逻辑不再赘述。

### 连接槽函数

我们实现槽函数`slot_tcp_connect`用来创建客户端到资源服务器的连接

``` cpp
void FileTcpMgr::slot_tcp_connect(std::shared_ptr<ServerInfo> si)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    _host = si->_res_host;
    _port = static_cast<uint16_t>(si->_res_port.toUInt());
    _socket.connectToHost(_host, _port);
}
```



### 注册处理流程

注册上传头像回调逻辑

``` cpp
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
            return;
        }

        auto file_info = UserMgr::GetInstance()->GetFileInfoByMD5(md5);
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
        sendObj["name"] = file_info->fileName();
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

}
```



### 独立文件线程

对于上传我们独立到文件上报线程中

``` cpp
class FileTcpThread: public std::enable_shared_from_this<FileTcpThread>{
public:
    FileTcpThread();
    ~FileTcpThread();
private:
    QThread * _file_tcp_thread;

};
```

具体实现

``` cpp
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

```

主函数调用

``` cpp
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "global.h"
#include "tcpmgr.h"
#include "filetcpmgr.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/style/stylesheet.qss");

    if( qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }else{
         qDebug("Open failed");
     }


    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    // 拼接文件名
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path +
                             QDir::separator() + fileName);

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;

    //启动tcp线程
    TcpThread tcpthread;
    //启动资源网络线程
    FileTcpThread file_tcp_thread;
    MainWindow w;
    w.show();
    return a.exec();
}

```

原来的登录流程稍作修改，连接好`ChatServer`后，连接`ResourceServer`， 最后再让用户登录。

## 服务器逻辑

服务器新增文件上报逻辑处理， 在`LogicWorker::RegisterCallBacks`中添加

``` cpp
_fun_callbacks[ID_UPLOAD_HEAD_ICON_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			auto uid = root["uid"].asInt();
			auto token = root["token"].asString();
			auto last_seq = root["last_seq"].asInt();
			//转化为字符串
			auto uid_str = std::to_string(uid);

			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / uid_str / name).string();
			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
				});

			//第一个包校验一下token是否合理
			if (seq == 1) {
				//从redis获取用户token是否正确
				std::string uid_str = std::to_string(uid);
				std::string token_key = USERTOKENPREFIX + uid_str;
				std::string token_value = "";
				bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
				if (!success) {
					rtvalue["error"] = ErrorCodes::UidInvalid;
					return;
				}

				if (token_value != token) {
					rtvalue["error"] = ErrorCodes::TokenInvalid;
					return;
				}
			}

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			//第一个包
			if (seq == 1) {
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				LogicSystem::GetInstance()->AddMD5File(md5, file_info);
			}
			else {
				auto file_info = LogicSystem::GetInstance()->GetFileInfo(md5);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
			}


			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, file_path_str, name, seq, total_size,
					trans_size, last, file_data),
				index
			);

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = total_size;
			rtvalue["seq"] = seq;
			rtvalue["name"] = name;
			rtvalue["trans_size"] = trans_size;
			rtvalue["last"] = last;
			rtvalue["md5"] = md5;
			rtvalue["uid"] = uid;
			rtvalue["last_seq"] = last_seq;
	};

```



## 源码

[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat)

效果展示：

上传前

![image-20250905145321372](https://cdn.llfc.club/img/image-20250905145321372.png)



上传后

![image-20250905145408921](https://cdn.llfc.club/img/image-20250905145408921.png)



服务器存储成功

![image-20250905145440949](https://cdn.llfc.club/img/image-20250905145440949.png)





## 续传信息持久化

增加`redis`接口

``` cpp
bool RedisMgr::SetFileInfo(const std::string& md5, std::shared_ptr<FileInfo> file_info)
{
	Json::Reader reader;
	Json::Value root;
	root["file_path_str"] = file_info->_file_path_str;
	root["name"] = file_info->_name;
	root["seq"] = file_info->_seq;
	root["total_size"] = file_info->_total_size;
	root["trans_size"] = file_info->_trans_size;
	auto file_info_str = root.toStyledString();
	auto redis_key = "file_upload_" + md5;
	bool success = SetExp(redis_key, file_info_str, 3600);
	return success;
}
```

新增超时设置

``` cpp
bool RedisMgr::SetExp(const std::string& key, const std::string& value, int expire_seconds) {
	//执行redis命令行
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "SETEX %s %d %s", key.c_str(), 
		      expire_seconds,
		value.c_str());

	if (NULL == reply) {
		std::cout << "Execute command [ SETEX " << key << " " << expire_seconds
			<< " " << value << " ] failure ! " << std::endl;
		_con_pool->returnConnection(connect);
		return false;
	}

	if (!(reply->type == REDIS_REPLY_STATUS &&
		(strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
		std::cout << "Execute command [ SETEX " << key << " " << expire_seconds
			<< " " << value << " ] failure ! " << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}

	freeReplyObject(reply);
	std::cout << "Execute command [ SETEX " << key << " " << expire_seconds
		<< " " << value << " ] success ! " << std::endl;
	_con_pool->returnConnection(connect);
	return true;
}
```

每次收到上传信息后，更新上传进度到redis中

``` cpp
_fun_callbacks[ID_UPLOAD_HEAD_ICON_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			auto uid = root["uid"].asInt();
			auto token = root["token"].asString();
			auto last_seq = root["last_seq"].asInt();
			//转化为字符串
			auto uid_str = std::to_string(uid);

			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / uid_str / name).string();
			Json::Value  rtvalue;
			auto callback = [=](const Json::Value& result) {

				// 在异步任务完成后调用
				Json::Value rtvalue = result;
				rtvalue["error"] = ErrorCodes::Success;
				rtvalue["total_size"] = total_size;
				rtvalue["seq"] = seq;
				rtvalue["name"] = name;
				rtvalue["trans_size"] = trans_size;
				rtvalue["last"] = last;
				rtvalue["md5"] = md5;
				rtvalue["uid"] = uid;
				rtvalue["last_seq"] = last_seq;
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
			};

			//第一个包校验一下token是否合理
			if (seq == 1) {
				//从redis获取用户token是否正确
				std::string uid_str = std::to_string(uid);
				std::string token_key = USERTOKENPREFIX + uid_str;
				std::string token_value = "";
				bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
				if (!success) {
					rtvalue["error"] = ErrorCodes::UidInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}

				if (token_value != token) {
					rtvalue["error"] = ErrorCodes::TokenInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			//第一个包
			if (seq == 1) {
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				//LogicSystem::GetInstance()->AddMD5File(md5, file_info);
				//改为用redis存储
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}
			else {
				//auto file_info = LogicSystem::GetInstance()->GetFileInfo(md5);
				//改为从redis中加载
				auto file_info = RedisMgr::GetInstance()->GetFileInfo(name);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}


			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, uid, file_path_str, name, seq, total_size,
					trans_size, last, file_data, callback),
				index
			);

	};

```



## 资源url更新

因为上头像传资源后，要将资源的路径存储到`mysql`数据库中，所以我们新增`MysqlMgr`,这个直接从`ChatServer`拷贝一份即可。

但是要注意，添加如下函数

``` cpp
bool MysqlMgr::UpdateUserIcon(int uid, const std::string& icon) {
	return _dao.UpdateHeadInfo(uid, icon);
}

```

`Dao`层面实现更新头像逻辑

``` cpp
bool MysqlDao::UpdateHeadInfo(int uid, const std::string& icon)
{
	auto con = pool_->getConnection();
	if (!con) {
		return false;
	}
	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	auto& conn = con->_con;
	try {
		std::string update_sql =
			"UPDATE user SET icon = ? WHERE uid = ?;";

		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(update_sql));
		pstmt->setString(1, icon);
		pstmt->setInt64(2, uid);

		int affected_rows = pstmt->executeUpdate();

		// 检查是否有行被更新（可选）
		if (affected_rows == 0) {
			std::cerr << "No user found with uid: " << uid << std::endl;
			return false;
		}

		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException in UpdateHeadInfo: " << e.what() << std::endl;
		return false;
	}
	return false;
}

```



## 封装异步回调

之前我们处理文件上传是异步方式，将要保存的文件投递给消息队列，交给独立线程后台保存。我们没有等待处理完成就直接将消息回传给客户端，这么做不是很好，所以改为异步方式，简单的方式就是通过回调函数处理，或者包装一个future等待。这里考虑保留异步结构，所以还是用回调处理

``` cpp
struct FileTask {
	FileTask(std::shared_ptr<CSession> session, int uid, std::string path, std::string name,
		int seq, int total_size, int trans_size, int last, 
		std::string file_data,
		std::function<void(const Json::Value&)> callback) :_session(session), _uid(uid),
		_seq(seq), _path(path), _name(name), _total_size(total_size),
		_trans_size(trans_size), _last(last), _file_data(file_data), _callback(callback)
	{}
	~FileTask(){}
	std::shared_ptr<CSession> _session;
	int _uid;
	int _seq ;
	std::string _path;
	std::string _name ;
	int _total_size ;
	int _trans_size ;
	int _last ;
	std::string _file_data;
	std::function<void(const Json::Value&)>  _callback;  //添加回调函数
};
```

改进后的处理

``` cpp
	_fun_callbacks[ID_UPLOAD_HEAD_ICON_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			auto uid = root["uid"].asInt();
			auto token = root["token"].asString();
			auto last_seq = root["last_seq"].asInt();
			//转化为字符串
			auto uid_str = std::to_string(uid);

			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / uid_str / name).string();
			Json::Value  rtvalue;
			auto callback = [=](const Json::Value& result) {

				// 在异步任务完成后调用
				Json::Value rtvalue = result;
				rtvalue["error"] = ErrorCodes::Success;
				rtvalue["total_size"] = total_size;
				rtvalue["seq"] = seq;
				rtvalue["name"] = name;
				rtvalue["trans_size"] = trans_size;
				rtvalue["last"] = last;
				rtvalue["md5"] = md5;
				rtvalue["uid"] = uid;
				rtvalue["last_seq"] = last_seq;
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
			};

			//第一个包校验一下token是否合理
			if (seq == 1) {
				//从redis获取用户token是否正确
				std::string uid_str = std::to_string(uid);
				std::string token_key = USERTOKENPREFIX + uid_str;
				std::string token_value = "";
				bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
				if (!success) {
					rtvalue["error"] = ErrorCodes::UidInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}

				if (token_value != token) {
					rtvalue["error"] = ErrorCodes::TokenInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			//第一个包
			if (seq == 1) {
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				//LogicSystem::GetInstance()->AddMD5File(md5, file_info);
				//改为用redis存储
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}
			else {
				//auto file_info = LogicSystem::GetInstance()->GetFileInfo(md5);
				//改为从redis中加载
				auto file_info = RedisMgr::GetInstance()->GetFileInfo(name);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}


			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, uid, file_path_str, name, seq, total_size,
					trans_size, last, file_data, callback),
				index
			);

	};
```

`callback`是我们封装的回调函数，投递给`FileTask`,  将来在后台线程处理`FileTask`时回调。

``` cpp
void FileWorker::task_callback(std::shared_ptr<FileTask> task)
{
	// 解码
	std::string decoded = base64_decode(task->_file_data);

	auto file_path_str = task->_path;
	auto last = task->_last;
	//std::cout << "file_path_str is " << file_path_str << std::endl;

	boost::filesystem::path file_path(file_path_str);
	boost::filesystem::path dir_path = file_path.parent_path();
	// 获取完整文件名（包含扩展名）
	std::string filename = file_path.filename().string();
	Json::Value result;
	// Check if directory exists, if not, create it
	if (!boost::filesystem::exists(dir_path)) {
		if (!boost::filesystem::create_directories(dir_path)) {
			std::cerr << "Failed to create directory: " << dir_path.string() << std::endl;
			result["error"] = ErrorCodes::FileNotExists;
			task->_callback(result);
			return;
		}
	}


	std::ofstream outfile;
	//第一个包
	if (task->_seq == 1) {
		// 打开文件，如果存在则清空，不存在则创建
		outfile.open(file_path_str, std::ios::binary | std::ios::trunc);
	}
	else {
		// 保存为文件
		outfile.open(file_path_str, std::ios::binary | std::ios::app);
	}


	if (!outfile) {
		std::cerr << "无法打开文件进行写入。" << std::endl;
		result["error"] = ErrorCodes::FileWritePermissionFailed;
		task->_callback(result);
		return ;
	}

	outfile.write(decoded.data(), decoded.size());
	if (!outfile) {
		std::cerr << "写入文件失败。" << std::endl;
		result["error"] = ErrorCodes::FileWritePermissionFailed;
		task->_callback(result);
		return ;
	}

	outfile.close();
	if (last) {
		std::cout << "文件已成功保存为: " << task->_name << std::endl;
		//更新头像
		MysqlMgr::GetInstance()->UpdateUserIcon(task->_uid, filename);
		//获取用户信息
		auto user_info = MysqlMgr::GetInstance()->GetUser(task->_uid);
		if (user_info == nullptr) {
			return ;
		}

		//将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = task->_uid;
		redis_root["pwd"] = user_info->pwd;
		redis_root["name"] = user_info->name;
		redis_root["email"] = user_info->email;
		redis_root["nick"] = user_info->nick;
		redis_root["desc"] = user_info->desc;
		redis_root["sex"] = user_info->sex;
		redis_root["icon"] = user_info->icon;
		std::string base_key = USER_BASE_INFO + std::to_string(task->_uid);
		RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
	}

	if (task->_callback) {
		task->_callback(result);
	}

}
```

## 客户端上传逻辑修改

``` cpp
//上传头像
void UserInfoPage::slot_up_load()
{
    // 1. 让对话框也能选 *.webp
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("选择图片"),
        QString(),
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.webp)")
    );
    if (filename.isEmpty())
        return;

    // 2. 直接用 QPixmap::load() 加载，无需手动区分格式
    QPixmap inputImage;
    if (!inputImage.load(filename)) {
        QMessageBox::critical(
            this,
            tr("错误"),
            tr("加载图片失败！请确认已部署 WebP 插件。"),
            QMessageBox::Ok
        );
        return;
    }

    QPixmap image = ImageCropperDialog::getCroppedImage(filename, 600, 400, CropperShape::CIRCLE);
    if (image.isNull())
        return;

    QPixmap scaledPixmap = image.scaled( ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
    ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    QString storageDir = QStandardPaths::writableLocation(
                             QStandardPaths::AppDataLocation);
    // 2. 在其下再建一个 avatars 子目录
    QDir dir(storageDir);
    if (!dir.exists("avatars")) {
        if (!dir.mkpath("avatars")) {
            qWarning() << "无法创建 avatars 目录：" << dir.filePath("avatars");
            QMessageBox::warning(
                this,
                tr("错误"),
                tr("无法创建存储目录，请检查权限或磁盘空间。")
            );
            return;
        }
    }
    // 3. 拼接最终的文件名 head.png
    QString file_name = generateUniqueIconName();
    QString filePath = dir.filePath("avatars" +
                          QString(QDir::separator()) + file_name);

    // 4. 保存 scaledPixmap 为 PNG（无损、最高质量）
    if (!scaledPixmap.save(filePath, "PNG")) {
        QMessageBox::warning(
            this,
            tr("保存失败"),
            tr("头像保存失败，请检查权限或磁盘空间。")
        );
    } else {
        qDebug() << "头像已保存到：" << filePath;
        // 以后读取直接用同一路径：storageDir/avatars/head.png
    }

    //实现头像上传
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "Could not open file:" << file.errorString();
        return;
    }

    //保存当前文件位置指针
    qint64 originalPos = file.pos();

    QCryptographicHash hash(QCryptographicHash::Md5);
    if (!hash.addData(&file)) {
            qWarning() << "Failed to read data from file:" << filePath;
            return ;
    }

    // 5. 转化为16进制字符串
    QString file_md5 = hash.result().toHex(); // 返回十六进制字符串

    //读取文件内容并发送
    QByteArray buffer;
    int seq = 0;

    //创建QFileInfo 对象
    auto fileInfo = std::make_shared<QFileInfo>(filePath);
    //获取文件名
    QString fileName = fileInfo->fileName();
    //文件名
    qDebug() << "文件名是: " << fileName;

    //获取文件大小
    int total_size = fileInfo->size();
    //最后一个发送序列
    int last_seq = 0;
    //获取最后一个发送序列
    if(total_size % MAX_FILE_LEN){
        last_seq = (total_size / MAX_FILE_LEN) +1;
    }else{
        last_seq = total_size / MAX_FILE_LEN;
    }

    // 恢复文件指针到原来的位置
    file.seek(originalPos);

    //每次读取MAX_FILE_LEN字节并发送
    buffer = file.read(MAX_FILE_LEN);

    QJsonObject jsonObj;
    //将文件内容转化为Base64 编码(可选)
    QString base64Data = buffer.toBase64();
    ++seq;
    jsonObj["md5"] = file_md5;
    jsonObj["name"] = file_name;
    jsonObj["seq"] = seq;
    jsonObj["trans_size"] = buffer.size() + (seq - 1) * MAX_FILE_LEN;
    jsonObj["total_size"] = total_size;
    jsonObj["token"] = UserMgr::GetInstance()->GetToken();
    jsonObj["uid"] = UserMgr::GetInstance()->GetUid();

    if (buffer.size() + (seq - 1) * MAX_FILE_LEN == total_size) {
        jsonObj["last"] = 1;
    } else {
        jsonObj["last"] = 0;
    }

    jsonObj["data"] = base64Data;
    jsonObj["last_seq"] = last_seq;
    QJsonDocument doc(jsonObj);
    auto send_data = doc.toJson();
    //将md5信息和文件信息关联存储
    UserMgr::GetInstance()->AddNameFile(file_name, fileInfo);
    //发送消息
    FileTcpMgr::GetInstance()->SendData(ID_UPLOAD_HEAD_ICON_REQ, send_data);
    file.close();
}
```



## 客户端加载头像

服务器将上传的头像信息保存为`url`更新到`mysql`中，接下来客户端登录需要加载新的头像

在`ChatDialog`的构造函数中将头像加载逻辑修改为

``` cpp
	//模拟加载自己头像
	QString head_icon = UserMgr::GetInstance()->GetIcon();
	//使用正则表达式检查是否使用默认头像
	QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
	QRegularExpressionMatch match = regex.match(head_icon);
	if (match.hasMatch()) {
		// 如果是默认头像（:/res/head_X.jpg 格式）
		QPixmap pixmap(head_icon); // 加载默认头像图片
		QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
		ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
	}
	else {
		// 如果是用户上传的头像，获取存储目录
		QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QDir avatarsDir(storageDir + "/avatars");

		// 确保目录存在
		if (avatarsDir.exists()) {
			QString avatarPath = avatarsDir.filePath(QFileInfo(head_icon).fileName()); // 获取上传头像的完整路径
			QPixmap pixmap(avatarPath); // 加载上传的头像图片
			if (!pixmap.isNull()) {
				QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				ui->side_head_lb->setPixmap(scaledPixmap);
				ui->side_head_lb->setScaledContents(true);
			}
			else {
				qWarning() << "无法加载上传的头像：" << avatarPath;
			}
		}
		else {
			qWarning() << "头像存储目录不存在：" << avatarsDir.path();
		}
	}
```

聊天页面也需要修改头像加载逻辑

``` cpp
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

            // 确保目录存在
            if (avatarsDir.exists()) {
                QString avatarPath = avatarsDir.filePath(QFileInfo(self_info->_icon).fileName()); // 获取上传头像的完整路径
                QPixmap pixmap(avatarPath); // 加载上传的头像图片
                if (!pixmap.isNull()) {
                    pChatItem->setUserIcon(pixmap);
                }
                else {
                    qWarning() << "无法加载上传的头像：" << avatarPath;
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

            // 确保目录存在
            if (avatarsDir.exists()) {
                QString avatarPath = avatarsDir.filePath(QFileInfo(friend_info->_icon).fileName()); // 获取上传头像的完整路径
                QPixmap pixmap(avatarPath); // 加载上传的头像图片
                if (!pixmap.isNull()) {
                    pChatItem->setUserIcon(pixmap);
                }
                else {
                    qWarning() << "无法加载上传的头像：" << avatarPath;
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
```

聊天列表中加载头像逻辑修改一下

``` cpp
void ChatUserWid::SetChatData(std::shared_ptr<ChatThreadData> chat_data) {
    _chat_data = chat_data;
    auto other_id = _chat_data->GetOtherId();
    auto other_info = UserMgr::GetInstance()->GetFriendById(other_id);
    // 加载图片

    QString head_icon = UserMgr::GetInstance()->GetIcon();

    // 使用正则表达式检查是否是默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(other_info->_icon);

    if (match.hasMatch()) {
        // 如果是默认头像（:/res/head_X.jpg 格式）
        QPixmap pixmap(other_info->_icon); // 加载默认头像图片
        QPixmap scaledPixmap = pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->icon_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
        ui->icon_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir(storageDir + "/avatars");

        // 确保目录存在
        if (avatarsDir.exists()) {
            QString avatarPath = avatarsDir.filePath(QFileInfo(other_info->_icon).fileName()); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->icon_lb->setPixmap(scaledPixmap);
                ui->icon_lb->setScaledContents(true);
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
            }
        }
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
        }
    }

    ui->user_name_lb->setText(other_info->_name);
    
    ui->user_chat_lb->setText(chat_data->GetLastMsg());
}
```

用户信息加载头像

``` cpp
UserInfoPage::UserInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserInfoPage)
{
    ui->setupUi(this);
    auto icon = UserMgr::GetInstance()->GetIcon();
    qDebug() << "icon is " << icon ;

    //使用正则表达式检查是否使用默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(icon);
    if (match.hasMatch()) {
        QPixmap pixmap(icon);
        QPixmap scaledPixmap = pixmap.scaled(ui->head_lb->size(), 
            Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
        ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
        ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir(storageDir + "/avatars");

        // 确保目录存在
        if (avatarsDir.exists()) {
            QString avatarPath = avatarsDir.filePath(QFileInfo(icon).fileName()); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(ui->head_lb->size(),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
                ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
                ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
            }
        }
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
        }
    }


  
    //获取nick
    auto nick = UserMgr::GetInstance()->GetNick();
    //获取name
    auto name = UserMgr::GetInstance()->GetName();
    //描述
    auto desc = UserMgr::GetInstance()->GetDesc();
    ui->nick_ed->setText(nick);
    ui->name_ed->setText(name);
    ui->desc_ed->setText(desc);
    //连接上
    connect(ui->up_btn, &QPushButton::clicked, this, &UserInfoPage::slot_up_load);
}
```



## 测试效果

![image-20250923094811744](https://cdn.llfc.club/img/image-20250923094811744.png)



## 客户端断点下载资源

### 客户端请求下载

在客户端加载本地资源发现不存在的时候，需要请求服务器，获取资源。

如果资源比较大，需要分批下载，也就是支持断点下载。

我们先拿`UserInfoPage`举例

``` cpp
UserInfoPage::UserInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserInfoPage)
{
    ui->setupUi(this);
    auto icon = UserMgr::GetInstance()->GetIcon();
    qDebug() << "icon is " << icon ;

    //使用正则表达式检查是否使用默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(icon);
    if (match.hasMatch()) {
        QPixmap pixmap(icon);
        QPixmap scaledPixmap = pixmap.scaled(ui->head_lb->size(), 
            Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
        ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
        ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir(storageDir + "/avatars");
        // 确保目录存在
        if (avatarsDir.exists()) {
            auto file_name = QFileInfo(icon).fileName();
            QString avatarPath = avatarsDir.filePath(QFileInfo(icon).fileName()); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {

                //判断是否正在下载
                bool is_loading = UserMgr::GetInstance()->IsDownLoading(file_name);
                if (is_loading) {
                    qWarning() << "正在下载: " << file_name;
                    //先加载默认的
                    QPixmap pixmap(":/res/head_1.jpg");
                    QPixmap scaledPixmap = pixmap.scaled(ui->head_lb->size(),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
                    ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
                    ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
                    return;
                }

                QPixmap scaledPixmap = pixmap.scaled(ui->head_lb->size(),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
                ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
                ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
                UserMgr::GetInstance()->AddLabelToReset(avatarPath, ui->head_lb);
                //先加载默认的
                QPixmap pixmap(":/res/head_1.jpg");
                QPixmap scaledPixmap = pixmap.scaled(ui->head_lb->size(),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
                ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
                ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

                //判断是否正在下载
                bool is_loading = UserMgr::GetInstance()->IsDownLoading(file_name);
                if (is_loading) {
                    qWarning() << "正在下载: " << file_name;
                    return;
                }
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
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
        }
    }


  
    //获取nick
    auto nick = UserMgr::GetInstance()->GetNick();
    //获取name
    auto name = UserMgr::GetInstance()->GetName();
    //描述
    auto desc = UserMgr::GetInstance()->GetDesc();
    ui->nick_ed->setText(nick);
    ui->name_ed->setText(name);
    ui->desc_ed->setText(desc);
    //连接上
    connect(ui->up_btn, &QPushButton::clicked, this, &UserInfoPage::slot_up_load);
}
```

如果本地资源不存在，则需要向服务器请求。

### 封装请求资源接口

**判断资源是否正在下载**

``` cpp

bool UserMgr::IsDownLoading(QString name) {
    std::lock_guard<std::mutex> lock(_down_load_mtx);
    auto iter = _name_to_download_info.find(name);
    if (iter == _name_to_download_info.end()) {
        return false;
    }

    return true;
}
```

如果资源加载成功，很可能处于正在下载，所以也要判断一下。

如果资源未加载成功，则需要向服务器下载资源。先讲要下载的资源和要加载资源的空间缓存起来。

``` cpp
void UserMgr::AddLabelToReset(QString path, QLabel* label)
{
    auto iter = _name_to_reset_labels.find(path);
    if (iter == _name_to_reset_labels.end()) {
        QList<QLabel*> list;
        list.append(label);
        _name_to_reset_labels.insert(path, list);
        return;
    }

    iter->append(label);
}
```

**结构如下图**

![image-20251004125019778](https://cdn.llfc.club/img/image-20251004125019778.png)

### 更新正在下载资源

``` cpp
void UserMgr::AddDownloadFile(QString name, 
    std::shared_ptr<DownloadInfo> file_info) {
    std::lock_guard<std::mutex> lock(_down_load_mtx);
    _name_to_download_info[name] = file_info;
}
```

### 发送下载请求

``` cpp
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
```



### 接收服务器回传

``` cpp
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
```

1. 判断seq是否为1，如果为1则说明新的文件，需要创建并保存
2. 如果seq不为1，则说明是续传文件，更新追加就可以了
3. 如果`is_last`字段为true，说明是最后一个包，那么移除缓存的下载信息，同时将信息发送到主界面更新图标



### 更新页面逻辑

在`ChatDialog`界面中响应这个信号

``` cpp
//重置label icon
connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_reset_label_icon, this, &ChatDialog::slot_reset_icon);
```

槽函数处理

``` cpp
void ChatDialog::slot_reset_icon(QString path) {
	UserMgr::GetInstance()->ResetLabelIcon(path);
}
```

`UserMgr`中封装重置`icon`逻辑

``` cpp
void UserMgr::ResetLabelIcon(QString path)
{
   auto iter =  _name_to_reset_labels.find(path);
   if (iter == _name_to_reset_labels.end()) {
       return;
   }

   for (auto ele_iter = iter.value().begin(); ele_iter != iter.value().end(); ele_iter++) {
       QPixmap pixmap(path); // 加载上传的头像图片
       if (!pixmap.isNull()) {
           QPixmap scaledPixmap = pixmap.scaled((*ele_iter)->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
           (*ele_iter)->setPixmap(scaledPixmap);
           (*ele_iter)->setScaledContents(true);
       }
       else {
           qWarning() << "无法加载上传的头像：" << path;
       }
   }

   _name_to_reset_labels.erase(iter);
}
```

### 测试效果

## 服务器断点传输逻辑

**增加下载worker**

``` cpp
class DownloadWorker {
public:
	DownloadWorker();
	~DownloadWorker();
	void PostTask(std::shared_ptr<DownloadTask> task);
private:
	void task_callback(std::shared_ptr<DownloadTask>);
	std::thread _work_thread;
	std::queue<std::shared_ptr<DownloadTask>> _task_que;
	std::atomic<bool> _b_stop;
	std::mutex  _mtx;
	std::condition_variable _cv;
};
```

`DownloadWorker`处理逻辑和之前的`FileWorker`类似

``` cpp
DownloadWorker::DownloadWorker() :_b_stop(false)
{
	_work_thread = std::thread([this]() {
		while (!_b_stop) {
			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]() {
				if (_b_stop) {
					return true;
				}

				if (_task_que.empty()) {
					return false;
				}

				return true;
				});

			if (_b_stop) {
				break;
			}

			auto task = _task_que.front();
			_task_que.pop();
			task_callback(task);
		}
	});
}

DownloadWorker::~DownloadWorker()
{
	_b_stop = true;
	_cv.notify_one();
	_work_thread.join();
}

void DownloadWorker::PostTask(std::shared_ptr<DownloadTask> task)
{
	{
		std::lock_guard<std::mutex> lock(_mtx);
		_task_que.push(task);
	}

	_cv.notify_one();
}

void DownloadWorker::task_callback(std::shared_ptr<DownloadTask> task)
{
	// 解码
	auto file_path_str = task->_file_path;

	//std::cout << "file_path_str is " << file_path_str << std::endl;

	boost::filesystem::path file_path(file_path_str);

	// 获取完整文件名（包含扩展名）
	std::string filename = file_path.filename().string();
	Json::Value result;
	result["error"] = ErrorCodes::Success;

	if (!boost::filesystem::exists(file_path)) {
		std::cerr << "文件不存在: " << file_path_str << std::endl;
		result["error"] = ErrorCodes::FileNotExists;
		task->_callback(result);
		return;
	}

	std::ifstream infile(file_path_str, std::ios::binary);
	if (!infile) {
		std::cerr << "无法打开文件进行读取。" << std::endl;
		result["error"] = ErrorCodes::FileReadPermissionFailed;
		task->_callback(result);
		return;
	}

	std::shared_ptr<FileInfo> file_info = nullptr;

	if (task->_seq == 1) {
		// 获取文件大小
		infile.seekg(0, std::ios::end);
		std::streamsize file_size = infile.tellg();
		infile.seekg(0, std::ios::beg);
		//如果为空，则创建FileInfo 构造数据存储
		file_info = std::make_shared<FileInfo>();
		file_info->_file_path_str = file_path_str;
		file_info->_name = filename;
		file_info->_seq = 1;

		file_info->_total_size = file_size;
		file_info->_trans_size = 0;
		// 立即保存到 Redis，覆盖旧数据，设置过期时间
		RedisMgr::GetInstance()->SetDownLoadInfo(filename, file_info);
		std::cout << "[新下载] 文件: " << filename
			<< ", 大小: " << file_size << " 字节" << std::endl;
	}
	else {
		//断点续传，从 Redis 获取历史信息
		file_info = RedisMgr::GetInstance()->GetDownloadInfo(filename);
		if (file_info == nullptr) {
			// Redis 中没有信息（可能过期了）
			std::cerr << "断点续传失败，Redis 中无下载信息: " << filename << std::endl;
			result["error"] = ErrorCodes::RedisReadErr; 
			task->_callback(result);
			infile.close();
			return;
		}

		// 验证序列号是否匹配
		if (task->_seq != file_info->_seq) {
			std::cerr << "序列号不匹配，期望: " << file_info->_seq
				<< ", 实际: " << task->_seq << std::endl;
			result["error"] = ErrorCodes::FileSeqInvalid;
			task->_callback(result);
			infile.close();
			return;
		}

		std::cout << "[续传] 文件: " << filename
			<< ", seq: " << task->_seq
			<< ", 进度: " << file_info->_trans_size
			<< "/" << file_info->_total_size << std::endl;
	}

	// 计算当前偏移量
	std::streamsize offset = ((std::streamsize)task->_seq - 1) * MAX_FILE_LEN;
	if (offset >= file_info->_total_size) {
		std::cerr << "偏移量超出文件大小。" << std::endl;
		result["error"] = ErrorCodes::FileOffsetInvalid;
		task->_callback(result);
		infile.close();
		return;
	}

	// 定位到指定偏移量
	infile.seekg(offset);

	// 读取最多2048字节
	char buffer[MAX_FILE_LEN];
	infile.read(buffer, MAX_FILE_LEN);
	//获取read实际读取多少字节
	std::streamsize bytes_read = infile.gcount();

	if (bytes_read <= 0) {
		std::cerr << "读取文件失败。" << std::endl;
		result["error"] = ErrorCodes::FileReadFailed;
		task->_callback(result);
		infile.close();
		return;
	}

	// 将读取的数据进行base64编码
	std::string data_to_encode(buffer, bytes_read);
	std::string encoded_data = base64_encode(data_to_encode);

	// 检查是否是最后一个包
	std::streamsize current_pos = offset + bytes_read;
	bool is_last = (current_pos >= file_info->_total_size);

	// 设置返回结果
	result["data"] = encoded_data;
	result["seq"] = task->_seq;
	result["total_size"] = std::to_string(file_info->_total_size);
	result["current_size"] = std::to_string(current_pos);
	result["is_last"] = is_last;

	infile.close();

	if (is_last) {
		std::cout << "文件读取完成: " << file_path_str << std::endl;
		RedisMgr::GetInstance()->DelDownLoadInfo(filename);
	}
	else {
		//更新信息
		file_info->_seq++;
		file_info->_trans_size = offset + bytes_read;
		//更新redis
		RedisMgr::GetInstance()->SetDownLoadInfo(filename, file_info);
	}

	if (task->_callback) {
		task->_callback(result);
	}
	
}

```

在`FileSystem`中创建`worker`

``` cpp
FileSystem::FileSystem()
{
	for (int i = 0; i < FILE_WORKER_COUNT; i++) {
		_file_workers.push_back(std::make_shared<FileWorker>());
	}

	for (int i = 0; i < DOWN_LOAD_WORKER_COUNT; i++) {
		_down_load_worker.push_back(std::make_shared<DownloadWorker>());
	}
}
```



### 测试效果

将头像资源从本地删除后，重新登录时或者切换页面会引发资源重新加载，向服务器请求资源后再设置到界面显示。

![image-20251004153501602](https://cdn.llfc.club/img/image-20251004153501602.png)























