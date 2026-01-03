---
title: 聊天图片资源续传和进度显示
date: 2026-01-02 08:22:03
tags: [C++聊天项目]
categories: [C++聊天项目]  
---

## 增加上传可视化进度

之前我们传输图片的时候，只能通过服务器查看上传进度。客户端无法感知上传进度，所以考虑在图片传输基础上，显示上传进度。

### 封装可点击标签

``` cpp
#pragma once
#include <QLabel>
#include <QWidget>
#include <QIcon>

class ClickableLabel :
    public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    void setIconOverlay(const QIcon& icon); //设置遮罩图标
    void showIconOverlay(bool show);  //显示/隐藏遮罩图标
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void clicked();

private:
    QIcon m_overlayIcon;
    bool m_showOverlay;
    bool m_hovered;
};
```

具体实现

``` cpp
#include "ClickableLabel.h"
#include <QMouseEvent>
#include <QPainter>
// ClickableLabel.cpp
ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
    , m_showOverlay(false)
    , m_hovered(false)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}

void ClickableLabel::enterEvent(QEvent* event)
{
    m_hovered = true;
    update();
    QLabel::enterEvent(event);
}

void ClickableLabel::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QLabel::leaveEvent(event);
}

void ClickableLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);

    if (m_showOverlay && !m_overlayIcon.isNull()) {
        QPainter painter(this);

        // 绘制半透明遮罩
        if (m_hovered) {
            painter.fillRect(rect(), QColor(0, 0, 0, 100));
        }
        else {
            painter.fillRect(rect(), QColor(0, 0, 0, 60));
        }

        // 绘制图标
        int iconSize = qMin(width(), height()) / 3; // 图标大小为图片的1/3
        QRect iconRect(
            (width() - iconSize) / 2,
            (height() - iconSize) / 2,
            iconSize,
            iconSize
        );

        m_overlayIcon.paint(&painter, iconRect);
    }
}

void ClickableLabel::setIconOverlay(const QIcon& icon)
{
    m_overlayIcon = icon;
    update();
}

void ClickableLabel::showIconOverlay(bool show)
{
    m_showOverlay = show;
    update();
}
```

### 图片设置可点击和进度条

![image-20260102111204491](https://cdn.llfc.club/img/image-20260102111204491.png)

在图片中添加可点击的标签，并且添加进度条

``` cpp
#ifndef PICTUREBUBBLE_H
#define PICTUREBUBBLE_H

#include "BubbleFrame.h"
#include <QHBoxLayout>
#include <QPixmap>
#include "ClickableLabel.h"
#include <QProgressBar>
#include "global.h"
class PictureBubble : public BubbleFrame
{
    Q_OBJECT
public:


    PictureBubble(const QPixmap& picture, ChatRole role,int total, QWidget* parent = nullptr);

    void setProgress(int value);
    void showProgress(bool show);
    void setState(TransferState state);
    void resumeState();
    void setMsgInfo(std::shared_ptr<MsgInfo> msg);
    TransferState state() const { return m_state; }

signals:
    void pauseRequested(QString unique_name, TransferType transfer_type);   // 请求暂停
    void resumeRequested(QString unique_name, TransferType transfer_type);  // 请求继续
    void cancelRequested(QString unique_name, TransferType transfer_type);  // 请求取消

private slots:
    void onPictureClicked();

private:
    void updateIconOverlay();
    void adjustSize();

private:
    ClickableLabel* m_picLabel;
    QProgressBar* m_progressBar;
    TransferState m_state;

    QIcon m_pauseIcon;
    QIcon m_playIcon;
    QIcon m_downloadIcon;
    QSize m_pixmapSize;
    QVBoxLayout* m_vLayout;
    int m_total_size;
    std::shared_ptr<MsgInfo> _msg_info;
};

#endif // PICTUREBUBBLE_H

```

构造函数里添加进度条和样式。

``` cpp
PictureBubble::PictureBubble(const QPixmap &picture, ChatRole role, int total, QWidget *parent)
    :BubbleFrame(role, parent), m_state(TransferState::None), m_total_size(total)
{
    // 加载图标（使用Qt内置图标或自定义图标）
    m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    m_downloadIcon = style()->standardIcon(QStyle::SP_ArrowDown);

    // 创建容器
    QWidget* container = new QWidget();
    m_vLayout = new QVBoxLayout(container);
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(5);


    // 创建可点击的图片标签
    m_picLabel = new ClickableLabel();
    m_picLabel->setScaledContents(true);
    QPixmap pix = picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT),
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_pixmapSize = pix.size();
    m_picLabel->setPixmap(pix);
    m_picLabel->setFixedSize(pix.size());

    connect(m_picLabel, &ClickableLabel::clicked,
        this, &PictureBubble::onPictureClicked);

    // 创建进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setFixedWidth(pix.width());
    m_progressBar->setFixedHeight(10);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    setState(TransferState::None);

    // 样式美化
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: 1px solid #ccc;"
        "   border-radius: 3px;"
        "   text-align: center;"
        "   background-color: #f0f0f0;"
        "   font-size: 10px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #4CAF50, stop:1 #45a049);"
        "   border-radius: 2px;"
        "}"
    );

    m_vLayout->addWidget(m_picLabel);
    m_vLayout->addWidget(m_progressBar);
    this->setWidget(container);
    adjustSize();
}
```

设置传输状态为None,进度条初始值为0

调整界面设置，将图片拉伸包括进度条

``` cpp
void PictureBubble::adjustSize()
{
    int left_margin = this->layout()->contentsMargins().left();
    int right_margin = this->layout()->contentsMargins().right();
    int v_margin = this->layout()->contentsMargins().bottom();

    int width = m_pixmapSize.width() + left_margin + right_margin;
    int height = m_pixmapSize.height() + v_margin * 2;

    if (m_progressBar->isHidden() == false) {
        height += m_progressBar->height() + m_vLayout->spacing();
    }

    setFixedSize(width, height);
}
```

封装设置进度函数

``` cpp
void PictureBubble::setProgress(int value)
{
    float percent = (value / (m_total_size*1.0))*100;
    m_progressBar->setValue(percent);
    if (percent >= 100) {
        setState(TransferState::Completed);
    }
}
```

展示进度条

``` cpp
void PictureBubble::showProgress(bool show)
{
    m_progressBar->show();
    adjustSize();
}
```

当传输停止后，点击后会恢复，恢复传输或者下载状态，逻辑如下

``` cpp
void PictureBubble::resumeState() {
    if (_msg_info->_transfer_type == TransferType::Download) {
        _msg_info->_transfer_state = TransferState::Downloading;
        m_state = TransferState::Downloading;
        updateIconOverlay();
        return;
    }

    if (_msg_info->_transfer_type == TransferType::Upload) {
        _msg_info->_transfer_state = TransferState::Uploading;
        m_state = TransferState::Uploading;
        updateIconOverlay();
        return;
    }
}
```

设置状态逻辑如下,  状态分为None,下载,上传,暂停,传输完成,传输失败等。

``` cpp
void PictureBubble::setState(TransferState state)
{
    m_state = state;
    if (_msg_info) {
        _msg_info->_transfer_state = state;
    }

    updateIconOverlay();

    // 根据状态显示/隐藏进度条
    switch (state) {
    case TransferState::Downloading:
    case TransferState::Uploading:
    case TransferState::Paused:
        showProgress(true);
        break;
    case TransferState::Completed:
        // 完成后延迟隐藏进度条
        QTimer::singleShot(1000, this, [this]() {
            showProgress(false);
            });
        break;
    case TransferState::None:
    case TransferState::Failed:
        showProgress(false);
        break;
    }
}
```

为了更好的控制传输逻辑，将`std::shared_ptr<MsgInfo>`作为成员变量设置给`PictureBubble`

这样在点击图片后，可以发送图片关联的消息`id`和`unique_name`

点击图片触发槽函数

``` cpp
void PictureBubble::onPictureClicked()
{
    switch (m_state) {
    case TransferState::Downloading:
    case TransferState::Uploading:
        // 暂停
        setState(TransferState::Paused);
        emit pauseRequested(_msg_info->_unique_name, _msg_info->_transfer_type);
        break;

    case TransferState::Paused:
        // 继续
        resumeState(); // 
        emit resumeRequested(_msg_info->_unique_name, _msg_info->_transfer_type);
        break;

    case TransferState::Failed:
        // 重试
        emit resumeRequested(_msg_info->_unique_name, _msg_info->_transfer_type);
        break;

    default:
        // 其他状态可以实现查看大图等功能
        break;
    }
}

```

根据状态发送不同的信号，如果是下载或者暂停状态，则发送暂停信号。

如果是暂停状态，则发送继续下载或者继续上传状态。

``` cpp
void PictureBubble::updateIconOverlay()
{
    switch (m_state) {
    case TransferState::Downloading:
    case TransferState::Uploading:
        m_picLabel->setIconOverlay(m_pauseIcon);
        m_picLabel->showIconOverlay(true);
        break;

    case TransferState::Paused:
        m_picLabel->setIconOverlay(m_playIcon);
        m_picLabel->showIconOverlay(true);
        break;

    case TransferState::Failed:
        m_picLabel->setIconOverlay(m_downloadIcon); // 或重试图标
        m_picLabel->showIconOverlay(true);
        break;

    default:
        m_picLabel->showIconOverlay(false);
        break;
    }
}
```

### 链接状态信号

在`ChatPage`的构造函数中添加信号和槽函数链接

``` cpp
//链接暂停信号
connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::pauseRequested,
            this, &ChatPage::on_clicked_paused);
//链接恢复信号
connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::resumeRequested,
            this, &ChatPage::on_clicked_resume);
```

槽函数中设置文件为暂停状态

``` cpp
void ChatPage::on_clicked_paused(QString unique_name, TransferType transfer_type)
{
    UserMgr::GetInstance()->PauseTransFileByName(unique_name);
}
```

槽函数中设置为续传或者下载状态，并且调用`FileMgr`续传之前未传递完成的内容

``` cpp
void ChatPage::on_clicked_resume(QString unique_name, TransferType transfer_type)
{
    UserMgr::GetInstance()->ResumeTransFileByName(unique_name);
    //继续发送或者下载
    if (transfer_type == TransferType::Upload) {
        FileTcpMgr::GetInstance()->ContinueUploadFile(unique_name);
        return;
    }

    if (transfer_type == TransferType::Download) {
        return;
    }
}
```

### 续传逻辑

`FileTcpMgr`发送信号`sig_continue_upload_file`, 这么做的好处是，发送信号的线程和接收信号的线程不一致，也可以跨线程调用。

``` cpp
void FileTcpMgr::ContinueUploadFile(QString unique_name) {
    emit sig_continue_upload_file(unique_name);
}
```

在`FileTcpMgr`中链接了这个信号

``` cpp
   //链接续传信号
     QObject::connect(this, &FileTcpMgr::sig_continue_upload_file, this, &FileTcpMgr::slot_continue_upload_file);
```

槽函数`slot_continue_upload_file`

``` cpp
void FileTcpMgr::slot_continue_upload_file(QString unique_name) {
    auto msg_info = UserMgr::GetInstance()->GetTransFileByName(unique_name);
    if (msg_info == nullptr) {
        return;
    }
    //将待发送序列号更新为已经确认接收的序列号，然后基于此序列号再递增。
    msg_info->_seq = msg_info->_last_confirmed_seq;

    if ((msg_info->_seq) * MAX_FILE_LEN >= msg_info->_total_size) {
        qDebug() << "file has sent finished";
        return;
    }

    if (MAX_CWND_SIZE - _cwnd_size == 0) {
        return;
    }

    //打开
    QFile file(msg_info->_text_or_url);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file: " << file.errorString();
        return;
    }

    //文件偏移到已经发送的位置，继续读取发送
    file.seek(msg_info->_seq * MAX_FILE_LEN);

    bool b_last = false;
    //再次组织数据发送
    for (; MAX_CWND_SIZE - _cwnd_size > 0; ) {

        QByteArray buffer;
        msg_info->_seq++;
        //放入发送未回包集合
        msg_info->_flighting_seqs.insert(msg_info->_seq);
        //每次读取MAX_FILE_LEN字节发送
        buffer = file.read(MAX_FILE_LEN);
        QJsonObject sendObj;
        //将文件内容转换为base64编码
        QString base64Data = buffer.toBase64();
        sendObj["md5"] = msg_info->_md5;
        sendObj["name"] = msg_info->_unique_name;
        sendObj["seq"] = msg_info->_seq;
        msg_info->_current_size = buffer.size() + (msg_info->_seq - 1) * MAX_FILE_LEN;
        sendObj["trans_size"] = msg_info->_current_size;
        sendObj["total_size"] = msg_info->_total_size;

        b_last = false;
        if (buffer.size() + (msg_info->_seq - 1) * MAX_FILE_LEN >= msg_info->_total_size) {
            sendObj["last"] = 1;
            b_last = true;
        }
        else {
            sendObj["last"] = 0;
        }

        sendObj["data"] = base64Data;
        sendObj["last_seq"] = msg_info->_max_seq;
        sendObj["uid"] = UserMgr::GetInstance()->GetUid();
        QJsonDocument doc(sendObj);
        auto send_data = doc.toJson();
        //直接发送，其实是放入tcpmgr发送队列
        SendData(ID_IMG_CHAT_CONTINUE_UPLOAD_REQ, send_data);
        _cwnd_size++;
        //如果
        if (b_last) {
            break;
        }
    }

    file.close();
}
```

`ID_IMG_CHAT_CONTINUE_UPLOAD_REQ`为断点续传请求

``` cpp
  ID_IMG_CHAT_CONTINUE_UPLOAD_REQ = 1043,  //续传聊天图片资源请求
  ID_IMG_CHAT_CONTINUE_UPLOAD_RSP = 1044,  //续传聊天图片资源回复
```

### 响应续传回复

``` cpp
 _handlers.insert(ID_IMG_CHAT_CONTINUE_UPLOAD_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        _cwnd_size--;
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject recvObj = jsonDoc.object();
        qDebug() << "data jsonobj is " << recvObj;

        if (!recvObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "icon upload_failed, err is Json Parse Err" << err;
            //todo ... 提示上传失败
            //emit upload_failed();
            return;
        }

        int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            //emit upload_failed();
            return;
        }

        auto name = recvObj["name"].toString();
        auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            return;
        }

        auto md5 = file_info->_md5;
        auto seq = recvObj["seq"].toInt();
        //根据seq从未接收集合移动到已接收集合中
        file_info->_flighting_seqs.erase(seq);
        //将seq放入已收到集合中
        file_info->_rsp_seqs.insert(seq);
        //计算当前最后确认的序列号
        while (file_info->_rsp_seqs.count(file_info->_last_confirmed_seq + 1)) {
            ++file_info->_last_confirmed_seq;
        }



        qDebug() << "recv : " << name << "file seq is " << seq;
        //判断最大序列和最后确认序列号相等，说明收全了
        if (file_info->_last_confirmed_seq == file_info->_max_seq) {
            //更新已经传输的文件大小
            file_info->_rsp_size = file_info->_total_size;
            //通知界面显示
            emit sig_update_upload_progress(file_info);
            UserMgr::GetInstance()->RmvTransFileByName(name);
            //todo 此处添加发送其他待发送的文件
            auto free_file = UserMgr::GetInstance()->GetFreeUploadFile();
            if (free_file == nullptr) {
                return;
            }
            BatchSend(free_file);
            return;
        }

        //更新已经传输的文件大小
        file_info->_rsp_size = (file_info->_last_confirmed_seq) * MAX_FILE_LEN;

        //发送信号，更新图片上传进度
        emit sig_update_upload_progress(file_info);
        //如果传输状态不为上传，则直接返回。
        if (!UserMgr::GetInstance()->TransFileIsUploading(name)) {
            return;
        }
        BatchSend(file_info); });
```

`sig_update_upload_progress`信号为更新进度，因为在接收到服务器回包前，文件状态可能被设置为暂停，所以要判断一下文件状态是否为暂停，如果为暂停，则不发送后续内容。否则直接发送后续内容。

`sig_update_upload_progress`信号，也会在正常上传服务器，服务器回复给客户端时，客户端发送

比如上传图片信息回复逻辑里

``` cpp
_handlers.insert(ID_IMG_CHAT_UPLOAD_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        _cwnd_size--;
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject recvObj = jsonDoc.object();
        qDebug() << "data jsonobj is " << recvObj;

        if (!recvObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "icon upload_failed, err is Json Parse Err" << err;
            //todo ... 提示上传失败
            //emit upload_failed();
            return;
        }

        int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            //emit upload_failed();
            return;
        }

        auto name = recvObj["name"].toString();
        auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            return;
        }

        auto md5 = file_info->_md5;
        auto seq = recvObj["seq"].toInt();
        //根据seq从未接收集合移动到已接收集合中
        file_info->_flighting_seqs.erase(seq);
        //将seq放入已收到集合中
        file_info->_rsp_seqs.insert(seq);
        //计算当前最后确认的序列号
        while (file_info->_rsp_seqs.count(file_info->_last_confirmed_seq + 1)) {
            ++file_info->_last_confirmed_seq;
        }

     

        qDebug() << "recv : " << name << "file seq is " << seq;
        //判断最大序列和最后确认序列号相等，说明收全了
        if (file_info->_last_confirmed_seq == file_info->_max_seq) {
            //更新已经传输的文件大小
            file_info->_rsp_size = file_info->_total_size;
            //通知界面显示
            emit sig_update_upload_progress(file_info);
            UserMgr::GetInstance()->RmvTransFileByName(name);
            //todo 此处添加发送其他待发送的文件
            auto free_file = UserMgr::GetInstance()->GetFreeUploadFile();
            if (free_file == nullptr) {
                return;
            }
            BatchSend(free_file);
            return;
        }

        //更新已经传输的文件大小
        file_info->_rsp_size = (file_info->_last_confirmed_seq) * MAX_FILE_LEN;

        //发送信号，更新图片上传进度
        emit sig_update_upload_progress(file_info);
        //如果传输状态不为上传，则直接返回。
        if (!UserMgr::GetInstance()->TransFileIsUploading(name)) {
            return;
        }
        BatchSend(file_info); });

        _handlers.insert(ID_IMG_CHAT_CONTINUE_UPLOAD_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        _cwnd_size--;
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject recvObj = jsonDoc.object();
        qDebug() << "data jsonobj is " << recvObj;

        if (!recvObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "icon upload_failed, err is Json Parse Err" << err;
            //todo ... 提示上传失败
            //emit upload_failed();
            return;
        }

        int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            //emit upload_failed();
            return;
        }

        auto name = recvObj["name"].toString();
        auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            return;
        }

        auto md5 = file_info->_md5;
        auto seq = recvObj["seq"].toInt();
        //根据seq从未接收集合移动到已接收集合中
        file_info->_flighting_seqs.erase(seq);
        //将seq放入已收到集合中
        file_info->_rsp_seqs.insert(seq);
        //计算当前最后确认的序列号
        while (file_info->_rsp_seqs.count(file_info->_last_confirmed_seq + 1)) {
            ++file_info->_last_confirmed_seq;
        }



        qDebug() << "recv : " << name << "file seq is " << seq;
        //判断最大序列和最后确认序列号相等，说明收全了
        if (file_info->_last_confirmed_seq == file_info->_max_seq) {
            //更新已经传输的文件大小
            file_info->_rsp_size = file_info->_total_size;
            //通知界面显示
            emit sig_update_upload_progress(file_info);
            UserMgr::GetInstance()->RmvTransFileByName(name);
            //todo 此处添加发送其他待发送的文件
            auto free_file = UserMgr::GetInstance()->GetFreeUploadFile();
            if (free_file == nullptr) {
                return;
            }
            BatchSend(free_file);
            return;
        }

        //更新已经传输的文件大小
        file_info->_rsp_size = (file_info->_last_confirmed_seq) * MAX_FILE_LEN;

        //发送信号，更新图片上传进度
        emit sig_update_upload_progress(file_info);
        //如果传输状态不为上传，则直接返回。
        if (!UserMgr::GetInstance()->TransFileIsUploading(name)) {
            return;
        }
        BatchSend(file_info); });
}

```

同步信息回复也会发送进度上传信号

``` cpp
    _handlers.insert(ID_FILE_INFO_SYNC_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject recvObj = jsonDoc.object();
        qDebug() << "data jsonobj is " << recvObj;

        if (!recvObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "icon upload_failed, err is Json Parse Err" << err;
            //todo ... 提示上传失败,将来可能断点重传等
            //emit upload_failed();
            return;
        }

        int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            //emit upload_failed();
            return;
        }

        //为了简单起见，先处理网络正常情况  
        auto seq = recvObj["seq"].toInt();
        auto name = recvObj["name"].toString();

        auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            return;
        }

        //根据seq从未接收集合移动到已接收集合中
        file_info->_flighting_seqs.erase(seq);
        //将seq放入已收到集合中
        file_info->_rsp_seqs.insert(seq);

        //计算当前最后确认的序列号
        while (file_info->_rsp_seqs.count(file_info->_last_confirmed_seq + 1)) {
            ++file_info->_last_confirmed_seq;
        }

        qDebug() << "recv : " << name << "file seq is " << seq;
        //判断最大序列和最后确认序列号相等，说明收全了
        if (file_info->_last_confirmed_seq == file_info->_max_seq) {
            //更新已经传输的文件大小
            file_info->_rsp_size = file_info->_total_size;
            //通知界面显示
            emit sig_update_upload_progress(file_info);
            UserMgr::GetInstance()->RmvTransFileByName(name);
            //todo 此处添加发送其他待发送的文件
            auto free_file = UserMgr::GetInstance()->GetFreeUploadFile();
            if (free_file == nullptr) {
                return;
            }
            BatchSend(free_file);
            return;
        }
        //更新已经传输的文件大小
        file_info->_rsp_size = (file_info->_last_confirmed_seq) * MAX_FILE_LEN;
        //通知界面显示
        emit sig_update_upload_progress(file_info);
        //如果传输状态不为上传，则直接返回。
        if (!UserMgr::GetInstance()->TransFileIsUploading(name)) {
            return;
        }
        BatchSend(file_info);
    });

```

在`ChatPage`中链接进度上传信号和槽函数

``` cpp
	//接收tcp返回的上传进度信息
	connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_update_upload_progress, 
		this, &ChatDialog::slot_update_upload_progress);
```

接收进度上传信号

``` cpp
void ChatDialog::slot_update_upload_progress(std::shared_ptr<MsgInfo> msg_info) {
	auto chat_data = UserMgr::GetInstance()->GetChatThreadByThreadId(msg_info->_thread_id);
	if (chat_data == nullptr) {
		return;
	}
	
	//更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

	chat_data->UpdateProgress(msg_info);

	if (_cur_chat_thread_id != msg_info->_thread_id) {
		return;
	}
	

	//更新聊天界面信息
	ui->chat_page->UpdateFileProgress(msg_info);
}
```

调用`ChatPage`的`UpdateFileProgress`函数

``` cpp
void ChatPage::UpdateFileProgress(std::shared_ptr<MsgInfo> msg_info) {
    auto iter = _base_item_map.find(msg_info->_msg_id);
    if (iter == _base_item_map.end()) {
        return;
    }

    if (msg_info->_msg_type == MsgType::IMG_MSG) {
        auto bubble = iter.value()->getBubble();
        PictureBubble*  pic_bubble = dynamic_cast<PictureBubble*>(bubble);
        pic_bubble->setProgress(msg_info->_rsp_size);
    }  
}
```

根据消息类型为图片，则调用`PictureBubble`更新图片进度



## 服务器

`ResoureServer`服务器要响应断点续传请求

`LogicWorker`注册消息处理

``` cpp
_fun_callbacks[ID_IMG_CHAT_CONTINUE_UPLOAD_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
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
			auto uid = root["uid"].asInt();
			//转化为字符串
			auto uid_str = std::to_string(uid);
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
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_IMG_CHAT_CONTINUE_UPLOAD_RSP);
			};

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
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_IMG_CHAT_CONTINUE_UPLOAD_RSP);
					return;
				}
			}
			else {
				auto file_info = RedisMgr::GetInstance()->GetFileInfo(name);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_IMG_CHAT_CONTINUE_UPLOAD_RSP);
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_IMG_CHAT_CONTINUE_UPLOAD_RSP);
					return;
				}
			}
			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, ID_IMG_CHAT_CONTINUE_UPLOAD_REQ, uid, file_path_str, name, seq, total_size,
					trans_size, last, file_data, callback),
				index
			);
	};
```

在`FileWorker`响应图片上传逻辑

``` cpp
//处理续传图片请求
	_handlers[ID_IMG_CHAT_CONTINUE_UPLOAD_REQ] = [this](std::shared_ptr<FileTask> task) {
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
		result["error"] = ErrorCodes::Success;

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
			return;
		}

		outfile.write(decoded.data(), decoded.size());
		if (!outfile) {
			std::cerr << "写入文件失败。" << std::endl;
			result["error"] = ErrorCodes::FileWritePermissionFailed;
			task->_callback(result);
			return;
		}

		outfile.close();
		if (last) {
			std::cout << "文件已成功保存为: " << task->_name << std::endl;
			//todo...更新数据库聊天图像上传状态
			//todo...通过grpc通知ChatServer
		}

		if (task->_callback) {
			task->_callback(result);
		}
	};
```

## 效果展示

![image-20260102101925154](https://cdn.llfc.club/img/image-20260102101925154.png)



![image-20260102102645373](https://cdn.llfc.club/img/image-20260102102645373.png)



