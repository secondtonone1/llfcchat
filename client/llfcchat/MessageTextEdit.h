#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QPainter>
#include <QVector>
#include "global.h"


class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageTextEdit(QWidget *parent = nullptr);

    ~MessageTextEdit();

    QVector<std::shared_ptr<MsgInfo>> getMsgList();

    void insertFileFromUrl(const QStringList &urls);
signals:
    void send();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *e);

private:
    void insertImages(const QString &url);
    void insertFiles(const QString &url);
    bool canInsertFromMimeData(const QMimeData *source) const;
    void insertFromMimeData(const QMimeData *source);

private:
    bool isImage(QString url);//判断文件是否为图片
    void insertMsgList(QVector<std::shared_ptr<MsgInfo>>& list, MsgType msgtype,
        QString text_or_url, QPixmap preview_pix,
        QString unique_name, uint64_t total_size, QString md5);
  
    QStringList getUrl(QString text);
    QPixmap getFileIconPixmap(const QString &url);//获取文件图标及大小信息，并转化成图片
    QString getFileSize(qint64 size);//获取文件大小

private slots:
    void textEditChanged();

private:
    QVector<std::shared_ptr<MsgInfo>> _img_or_file_list;
    QVector<std::shared_ptr<MsgInfo>> _total_msg_list;
};

#endif // MESSAGETEXTEDIT_H
